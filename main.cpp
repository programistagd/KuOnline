#include "seasocks/PrintfLogger.h"
#include "seasocks/Server.h"
#include "seasocks/StringUtil.h"
#include "seasocks/WebSocket.h"
#include "seasocks/util/Json.h"
#include "json.hpp"
#include <set>
#include <string>

using namespace std;
using namespace seasocks;
using namespace nlohmann;

struct Message{
	Message(){}
	Message(string data){
		json msg = json::parse(data);
		name = msg["name"];
		data = msg["data"];
	}
	string name;
	json data;
	string toData() const{
		json msg;
		msg["name"] = name;
		msg["data"] = data;
		return msg.dump();
	}
};

struct Player{
	Player(WebSocket* ws) : ws(ws) {}
	Player(){}
	string nickname;
	WebSocket* ws;
};

struct Game : WebSocket::Handler {
	//helper functions
    void onConnect(WebSocket *socket) override 
    {
		connections.insert(make_pair(socket,Player(socket)));
	}
    void onData(WebSocket *, const char *data) override 
    {
		Message m(data);
	}
    void onDisconnect(WebSocket *socket) override 
    {
		if(state == WAITING)
			connections.erase(socket);
		else{
			connections[socket].nickname = connections[socket].nickname + " (disconnected)";
			connections[socket].ws = nullptr;
		}
	}
	
	void broadcast(const Message& m){
		string data = m.toData();
		for (auto c : connections){
			if(c.second.ws!=nullptr)
				c.second.ws->send(data.c_str());
		}
	}
	void send(WebSocket* c, const Message& m){
		if(c==nullptr) return;
		c->send(m.toData().c_str());
	}
	
	//logic implementation
	map<WebSocket*,Player> connections;
    enum State{
		WAITING, PLAYING
	} state = WAITING;
    Game(){
		
	}
	void refreshPlayers(){
		
	}
};

int main() {
    Server server(make_shared<PrintfLogger>());
    server.addWebSocketHandler("/ws", make_shared<Game>());
    server.serve("../web", 3000);
}
