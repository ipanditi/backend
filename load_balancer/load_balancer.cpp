#define _XOPEN_SOURCE_EXTENDED 1 //A special dependency in C++ for connect() function
#include <utility>  // for std::pair
#include <string>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <thread>
#include <vector>
#include <mutex>
std::pair<bool, int> is_server_healthy(const std::string& host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return {false, -1};
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        close(sock);
        return {false, -1};
    }

    int result = connect(sock, (sockaddr*)&server_addr, sizeof(server_addr));
    return {result == 0, result};  // Return whether healthy and the result of connect
}

std::vector<std::string> backend_servers = {"127.0.0.1:8081", "127.0.0.1:8082", "127.0.0.1:8083"};
int current_server = 0;
std::mutex server_mutex;

void handle_client(int client_socket) {
    std::string backend;
    std::string host;
    int port;
    int attempt = 0;
    int backend_socket = -1;
    bool server_healthy = false;

    while (attempt < backend_servers.size()) {
        // Round-robin selection of backend server
        server_mutex.lock();
        backend = backend_servers[current_server];
        current_server = (current_server + 1) % backend_servers.size();
        server_mutex.unlock();

        host = backend.substr(0, backend.find(':'));
        port = std::stoi(backend.substr(backend.find(':') + 1));

        auto [healthy, connect_result] = is_server_healthy(host, port);
        std::cout << (healthy ? "Healthy: " : "Unhealthy: ") << host << ":" << port << std::endl;

        if (healthy) {
            server_healthy = true;

            backend_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (backend_socket == -1) {
                std::cerr << "Failed to create backend socket: " << strerror(errno) << std::endl;
                close(client_socket);
                return;
            }

            sockaddr_in backend_addr;
            backend_addr.sin_family = AF_INET;
            backend_addr.sin_port = htons(port);
            if (inet_pton(AF_INET, host.c_str(), &backend_addr.sin_addr) <= 0) {
                std::cerr << "Invalid address/Address not supported: " << host << std::endl;
                close(client_socket);
                close(backend_socket);
                return;
            }

            if (connect(backend_socket, (sockaddr*)&backend_addr, sizeof(backend_addr)) == -1) {
                std::cerr << "Failed to connect to backend server: " << host << ":" << port << " - " << strerror(errno) << std::endl;
                close(client_socket);
                close(backend_socket);
                return;
            }

            std::cout << "Connected to backend server: " << host << ":" << port << std::endl;
            break;
        }

        attempt++;
    }

    if (!server_healthy) {
        std::cerr << "No healthy backend servers available\n";
        close(client_socket);
        return;
    }

    // Forward client request to backend server
    char buffer[4096];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        send(backend_socket, buffer, bytes_received, 0);
	std::cout<<"Sent to server"<<std::endl;
    }

    // Forward backend response to client
    bytes_received = recv(backend_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        send(client_socket, buffer, bytes_received, 0);
	std::cout<<"Received from server"<<std::endl;
    }

    close(client_socket);
    close(backend_socket);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == -1) {
        std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
        close(server_socket);
        return 1;
    }

    std::cout << "Load balancer is listening on port 8080\n";

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_size = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);
        if (client_socket == -1) {
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            continue;
        }

        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);
    return 0;
}

