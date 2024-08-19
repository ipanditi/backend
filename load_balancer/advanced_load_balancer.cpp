#define _XOPEN_SOURCE_EXTENDED 1 // A special dependency in C++ for connect() function
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
#include <map>
#include <algorithm>
#include <numeric>
#include <climits>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <chrono>

class LoadBalancingAlgorithm {
public:
    virtual ~LoadBalancingAlgorithm() {}
    virtual std::string selectServer(const std::vector<std::string>& servers) = 0;
};

class RoundRobin : public LoadBalancingAlgorithm {
public:
    RoundRobin() : current_index(0) {}
    std::string selectServer(const std::vector<std::string>& servers) override {
        std::string server = servers[current_index];
        std::cout << "RoundRobin selected server: " << server << std::endl;
        current_index = (current_index + 1) % servers.size();
        return server;
    }
private:
    int current_index;
};

class LeastConnections : public LoadBalancingAlgorithm {
public:
    LeastConnections(const std::map<std::string, int>& connections) : active_connections(connections) {}
    std::string selectServer(const std::vector<std::string>& servers) override {
        std::string best_server;
        int min_connections = INT_MAX;
        for (const auto& server : servers) {
	    auto it  = active_connections.find(server);
            if(it!= active_connections.end()){
		int connections = it->second;
	   	if (connections < min_connections) {
                   min_connections = connections;
                   best_server = server;
		}
		else{
		   std::cerr << "Warning: Server " << server << " not found in active connections.\n";
   		}
            }
        }
        std::cout << "LeastConnections selected server: " << best_server << std::endl;
        return best_server;
    }
private:
    std::map<std::string, int> active_connections;
};

class WeightedRoundRobin : public LoadBalancingAlgorithm {
public:
    WeightedRoundRobin(const std::map<std::string, int>& weights) : server_weights(weights), current_index(0) {}
    std::string selectServer(const std::vector<std::string>& servers) override {
        int total_weight = std::accumulate(server_weights.begin(), server_weights.end(), 0,
                                           [](int sum, const std::pair<std::string, int>& kv) {
                                               return sum + kv.second;
                                           });
        int index = current_index;
        int weight_sum = 0;
        for (const auto& server : servers) {
            weight_sum += server_weights[server];
            if (weight_sum >= (total_weight * index / servers.size())) {
                current_index = (index + 1) % servers.size();
                std::cout << "WeightedRoundRobin selected server: " << server << std::endl;
                return server;
            }
        }
        std::cout << "WeightedRoundRobin fallback to server: " << servers[0] << std::endl;
        return servers[0]; // Default case
    }
private:
    std::map<std::string, int> server_weights;
    int current_index;
};

// Global variables
std::vector<std::string> backend_servers = {"127.0.0.1:8084", "127.0.0.1:8082", "127.0.0.1:8083"};
std::map<std::string, int> server_metrics;
std::map<std::string, int> active_connections;
std::mutex server_mutex;
LoadBalancingAlgorithm* current_algorithm = nullptr;


std::ofstream log_file("traffic_log.csv", std::ios::app);

void log_to_csv(const std::string& timestamp, const std::string& server, const std::string& algorithm) {
    if (log_file.is_open()) {
        log_file << timestamp << "," << server << "," << algorithm << "\n";
    } else {
        std::cerr << "Failed to open log file\n";
    }
}

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
    bool healthy = (result == 0);
    close(sock);
    std::cout << "Health check for server " << host << ":" << port << " - " << (healthy ? "Healthy" : "Unhealthy") << std::endl;
    return {healthy, result};  // Return whether healthy and the result of connect
}

void monitor_and_adapt() {
    while (true) {
	backend_servers = {"127.0.0.1:8084","127.0.0.1:8082","127.0.0.1:8083"};
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Update server health status and active connections
        std::map<std::string, bool> health_status;
        for (const auto& server : backend_servers) {
            std::string host = server.substr(0, server.find(':'));
            int port = std::stoi(server.substr(server.find(':') + 1));
            auto [healthy, _] = is_server_healthy(host, port);
            health_status[server] = healthy;
        }

        // Remove unhealthy servers
        std::vector<std::string> healthy_servers;
        for (const auto& server : backend_servers) {
            if (health_status[server]) {
                healthy_servers.push_back(server);
            } else {
                std::cout << "Server " << server << " is unhealthy and will be removed." << std::endl;
            }
        }

        if (healthy_servers.empty()) {
            std::cerr << "All servers are unhealthy. Waiting for health status to improve...\n";
            continue;
        }

        // Update the list of backend servers
        server_mutex.lock();
        backend_servers = healthy_servers;
        server_mutex.unlock();

        // Adapt load balancing algorithm based on active connections
        bool traffic_spike = std::accumulate(active_connections.begin(), active_connections.end(), 0,
                                             [](int sum, const std::pair<std::string, int>& kv) {
                                                 return sum + kv.second;
                                             }) > 5;

        delete current_algorithm;

        if (traffic_spike) {
            std::cout << "Switching to LeastConnections algorithm due to traffic spike.\n";
            current_algorithm = new LeastConnections(active_connections);
        } else if (std::all_of(active_connections.begin(), active_connections.end(),
                               [](const std::pair<std::string, int>& kv) {
                                   return kv.second == 0;
                               })) {
            std::cout << "Switching to RoundRobin algorithm.\n";
            current_algorithm = new RoundRobin();
        } else {
            std::cout << "Switching to WeightedRoundRobin algorithm.\n";
            current_algorithm = new WeightedRoundRobin(server_metrics);
        }
    }
}

void handle_client(int client_socket) {
    std::string backend;
    std::string host;
    int port;
    int backend_socket = -1;
    bool server_healthy = false;
    std::string selected_server;
    std::string algorithm_name = typeid(*current_algorithm).name();

    while (true) {
        server_mutex.lock();
        backend = current_algorithm->selectServer(backend_servers);
        //active_connections[backend]++;
	server_mutex.unlock();

        host = backend.substr(0, backend.find(':'));
        port = std::stoi(backend.substr(backend.find(':') + 1));

        auto [healthy, connect_result] = is_server_healthy(host, port);
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

            // Increment active connection count
            server_mutex.lock();
            active_connections[backend]++;
            server_mutex.unlock();

            std::cout << "Connected to backend server: " << host << ":" << port << std::endl;
            break; // Successfully connected to a healthy server
        } else {
            std::cerr << "Backend server " << host << ":" << port << " is unhealthy, trying another server.\n";
        }
    }

    if (!server_healthy) {
        std::cerr << "No healthy servers available, closing client connection." << std::endl;
        close(client_socket);
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream timestamp;
    timestamp << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    log_to_csv(timestamp.str(), selected_server, algorithm_name);

    // Relay traffic between client and backend
    fd_set readfds;
    char buffer[4096];
    ssize_t bytes_read, bytes_sent;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);
        FD_SET(backend_socket, &readfds);

        int max_fd = std::max(client_socket, backend_socket) + 1;
        int activity = select(max_fd, &readfds, nullptr, nullptr, nullptr);

        if (activity < 0 && errno != EINTR) {
            std::cerr << "Select error: " << strerror(errno) << std::endl;
            break;
        }

        if (FD_ISSET(client_socket, &readfds)) {
            bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_read <= 0) break;

            bytes_sent = send(backend_socket, buffer, bytes_read, 0);
            if (bytes_sent <= 0) break;
        }

        if (FD_ISSET(backend_socket, &readfds)) {
            bytes_read = recv(backend_socket, buffer, sizeof(buffer), 0);
            if (bytes_read <= 0) break;

            bytes_sent = send(client_socket, buffer, bytes_read, 0);
            if (bytes_sent <= 0) break;
        }
    }

    // Decrement active connection count
    server_mutex.lock();
    active_connections[backend]--;
    server_mutex.unlock();
    close(client_socket);
    close(backend_socket);
}

int main() {
    // Initialize active connections for each server
    for (const auto& server : backend_servers) {
        active_connections[server] = 0;
    }

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

    // Initialize with default algorithm
    current_algorithm = new RoundRobin();

    std::thread(monitor_and_adapt).detach();

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

    // Cleanup
    delete current_algorithm;
    close(server_socket);
    log_file.close();
    return 0;
}

