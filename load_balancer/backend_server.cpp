#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<thread>

void run_backend_server(int port){
    int server_socket = socket(AF_INET, SOCK_STREAM,0);
    if(server_socket==-1){
	std::cerr<<"Failed to create socket\n"<<std::endl;
	return;
    }
    sockaddr_in server_addr;
    server_addr.sin_family= AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to bind socket\n";
        return;
    }

    if (listen(server_socket, SOMAXCONN) == -1) {
        std::cerr << "Failed to listen on socket\n";
        return;
    }
    std::cout<<"Bind:\t"<<bind<<"\tlisten:\t"<<listen<<std::endl;
    std::cout << "Backend server is listening on port " << port << "\n";

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_size = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);
        if (client_socket == -1) {
            std::cerr << "Failed to accept connection\n";
            continue;
        }

        char buffer[4096];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
            send(client_socket, response.c_str(), response.size(), 0);
        }

        close(client_socket);
    }

    close(server_socket);
}

int main() {
    std::thread server1(run_backend_server, 8081);
    std::thread server2(run_backend_server, 8082);
    std::thread server3(run_backend_server, 8083);

    server1.join();
    server2.join();
    server3.join();

    return 0;
}
