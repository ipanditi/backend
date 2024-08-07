#include "BaseServer.h"
#include <iostream>
#include <unistd.h>
class Server2 : public BaseServer {
public:
    Server2() : BaseServer(8082) {}

protected:
    void handleClient(int clientSocket) override {
        const std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello from Server 2";

        write(clientSocket, response.c_str(), response.size());
    }
};

int main() {
    Server2 server;
    server.start();
    return 0;
}

