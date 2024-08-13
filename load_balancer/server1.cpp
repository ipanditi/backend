#include "BaseServer.h"
#include <iostream>
#include <unistd.h>
class Server1 : public BaseServer {
public:
    Server1() : BaseServer(8084) {}

protected:
    void handleClient(int clientSocket) override {
        const std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 20\r\n"
            "\r\n"
            "Hello from Server 1\n";

        ssize_t bytesSent = write(clientSocket, response.c_str(), response.size());
    	if (bytesSent < 0) {
        	std::cerr << "Failed to send response" << std::endl;
	}
	//write(clientSocket, response.c_str(), response.size());
    }
};

int main() {
    Server1 server;
    server.start();
    return 0;
}

