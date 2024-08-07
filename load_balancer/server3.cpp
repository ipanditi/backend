#include "BaseServer.h"
#include <iostream>
#include <unistd.h>
class Server3 : public BaseServer {
public:
    Server3() : BaseServer(8083) {}

protected:
    void handleClient(int clientSocket) override {
        const std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello from Server 3";

        write(clientSocket, response.c_str(), response.size());
    }
};

int main() {
    Server3 server;
    server.start();
    return 0;
}

