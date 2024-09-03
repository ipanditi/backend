#include "BaseServer.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>

class Server1 : public BaseServer {
public:
    Server1(int port) : BaseServer(port) {}

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

int main(int argc, char* argv[]) {
    // Set default port or get from environment variable
    int port = 8084; // default port

    if (const char* envPort = std::getenv("SERVER_PORT")) {
        port = std::atoi(envPort);
    }

    // Or accept port as a command-line argument
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    Server1 server(port);
    server.start();
    return 0;
}

