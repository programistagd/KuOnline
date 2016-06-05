#include "seasocks/PrintfLogger.h"
#include "seasocks/Server.h"
#include "seasocks/StringUtil.h"
#include "seasocks/WebSocket.h"
#include "seasocks/util/Json.h"
#include "json.hpp"
#include <set>
#include <thread>
#include <string>

using namespace std;
using namespace seasocks;
using namespace nlohmann;

struct Message{
	Message(){}
	Message(string name, json data) : name(name), data(data) {}
	Message(string text){
		json msg = json::parse(text);
		name = msg["name"];
		data = msg["data"];
	}
	string name;
	json data;
	string toText() const{
		json msg;
		msg["name"] = name;
		msg["data"] = data;
		return msg.dump();
	}
};

struct Card{
	enum Type{
		A='A',K='K',Q='Q',J='J',T='T'
	} type;
	char name(){
		return (char) type;
	}
};

struct Player{
	int hp = 2;
	int money = 2;
	Card living[2];
	Card dead[2];
};

struct GameCore{
	vector<Player> players;
	void prepare(int num){
		players.resize(num);
	}
};

struct User{
	User(WebSocket* ws) : ws(ws) {}
	User(){}
	string nickname = "???";
	WebSocket* ws;
	int playerId;
};

struct Game : WebSocket::Handler {
	GameCore core;
	//helper functions
    void onConnect(WebSocket *socket) override 
    {
		if(state == PLAYING){
			send(socket,Message("error","Game in progress. Spectating not supported").toText());
			socket->close();
			return;
		}
		connections.insert(make_pair(socket,User(socket)));
		updateUsers();
	}
    void onData(WebSocket * conn, const char *data) override 
    {
		Message m(data);
		if(handlers[m.name]){
			handlers[m.name](conn,m.data);
		}
	}
    void onDisconnect(WebSocket *socket) override 
    {
		if(state == WAITING)
			connections.erase(socket);
		else{
			connections[socket].nickname = connections[socket].nickname + " (disconnected)";
			connections[socket].ws = nullptr;
		}
		updateUsers();
	}
	
	void broadcast(const Message& m){
		string data = m.toText();
		for (auto c : connections){
			if(c.second.ws!=nullptr)
				c.second.ws->send(data.c_str());
		}
	}
	void send(WebSocket* c, const Message& m){
		if(c==nullptr) return;
		c->send(m.toText().c_str());
	}
	
	//logic implementation
	map<WebSocket*,User> connections;
	map<string,function<void(WebSocket*,json)>> handlers;
    enum State{
		WAITING, PLAYING
	} state = WAITING;
    Game(){
		handlers["login"] = [&](WebSocket* conn, json data){
			connections[conn].nickname = data["nickname"];
			updateUsers();
		};
		handlers["requestStart"] = [&](WebSocket* conn, json data){
			startPlaying();
		};
	}
	void updateUsers(){
		Message m;
		m.name = "updateUsers";
		for(auto c : connections){
			json u;
			u["id"] = c.second.playerId;
			u["name"] = c.second.nickname;
			m.data.push_back(u);
		}
		broadcast(m);
	}
	
	void startPlaying(){
		cout<<"Starting the game"<<endl;
		state = PLAYING;
		int count = 0;
		for(auto c : connections){
			if(c.second.nickname!="???") count++;
		}
		core.prepare(count);
		int pid = 0;
		for(auto c : connections){
			if(c.second.nickname!="???") c.second.playerId = pid++;
		}
		updateUsers();
		broadcast(Message("start",json()));
	}
};

int main() {
    Server server(make_shared<PrintfLogger>());
    shared_ptr<Game> game = make_shared<Game>();
    server.addWebSocketHandler("/ws", game);
    volatile bool running = true;
    thread consoleThread([&](){
		while(running){
			string command;
			cin>>command;
			if(command=="start"){
				server.execute([&](){
					game->startPlaying();
				});
			}else{
				cout<<"Unrecognized command"<<endl;
			}
		}
	});
    server.serve("../web", 3000);
    running = false;
    consoleThread.join();
}
