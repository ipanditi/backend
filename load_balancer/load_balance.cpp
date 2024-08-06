#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <mutex>
#include <thread>

std::vector<std::string> backend_servers = {"127.0.0.1:8081", "127.0.0.1:8082", "127.0.0.1:8083"};
int current_server = 0;

void handle_client(int client_socket) {
    // Round-robin selection of backend server
    std::string backend = backend_servers[current_server];
    current_server = (current_server + 1) % backend_servers.size();

    std::string host = backend.substr(0, backend.find(':'));
    int port = std::stoi(backend.substr(backend.find(':') + 1));

    int backend_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (backend_socket == -1) {
        std::cerr << "Failed to create backend socket\n";
        close(client_socket);
        return;
    }

    sockaddr_in backend_addr;
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &backend_addr.sin_addr);

    if (connect(backend_socket, (sockaddr*)&backend_addr, sizeof(backend_addr)) == -1) {
        std::cerr << "Failed to connect to backend server\n";
        close(client_socket);
        close(backend_socket);
        return;
    }

    // Forward client request to backend server
    char buffer[4096];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    send(backend_socket, buffer, bytes_received, 0);

    // Forward backend response to client
    bytes_received = recv(backend_socket, buffer, sizeof(buffer), 0);
    send(client_socket, buffer, bytes_received, 0);

    close(client_socket);
    close(backend_socket);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to bind socket\n";
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == -1) {
        std::cerr << "Failed to listen on socket\n";
        return 1;
    }

    std::cout << "Load balancer is listening on port 8080\n";

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_size = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);
        if (client_socket == -1) {
            std::cerr << "Failed to accept connection\n";
            continue;
        }

        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}
