#include "seasocks/PrintfLogger.h"
#include "seasocks/Server.h"
#include "seasocks/StringUtil.h"
#include "seasocks/WebSocket.h"
#include "seasocks/util/Json.h"
#include <set>

using namespace std;
using namespace seasocks;

struct ChatHandler : WebSocket::Handler {
    set<WebSocket *> connections;
    void onConnect(WebSocket *socket) override 
    { connections.insert(socket); }
    void onData(WebSocket *, const char *data) override 
    { for (auto c : connections) c->send(data); }
    void onDisconnect(WebSocket *socket) override 
    { connections.erase(socket); }
};

int main() {
    Server server(make_shared<PrintfLogger>());
    server.addWebSocketHandler("/chat", make_shared<ChatHandler>());
    server.serve("../web", 3000);
}
