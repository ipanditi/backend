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
#include <map>
#include <algorithm>
#include <numeric>
#include <string>

// LoadBalancingAlgorithm Base Class
class LoadBalancingAlgorithm {
public:
    virtual std::string selectServer(const std::vector<std::string>& servers) = 0;
    virtual ~LoadBalancingAlgorithm() = default;
};

// RoundRobin Algorithm
class RoundRobin : public LoadBalancingAlgorithm {
public:
    RoundRobin() : current_index(0) {}
    std::string selectServer(const std::vector<std::string>& servers) override {
        std::string server = servers[current_index];
        current_index = (current_index + 1) % servers.size();
        return server;
    }
private:
    size_t current_index;
};

// LeastConnections Algorithm
class LeastConnections : public LoadBalancingAlgorithm {
public:
    LeastConnections(const std::map<std::string, int>& metrics) : server_metrics(metrics) {}
    std::string selectServer(const std::vector<std::string>& servers) override {
        std::string least_connected_server = servers[0];
        int least_connections = server_metrics.at(servers[0]);
        for (const auto& server : servers) {
            int connections = server_metrics.at(server);
            if (connections < least_connections) {
                least_connections = connections;
                least_connected_server = server;
            }
        }
        return least_connected_server;
    }
private:
    std::map<std::string, int> server_metrics;
};

// WeightedRoundRobin Algorithm
class WeightedRoundRobin : public LoadBalancingAlgorithm {
public:
    WeightedRoundRobin(const std::map<std::string, int>& weights) : server_weights(weights), current_index(0) {}
    std::string selectServer(const std::vector<std::string>& servers) override {
        int total_weight = 0;
        for (const auto& server : servers) {
            total_weight += server_weights.at(server);
        }
        int weight_index = current_index % total_weight;
        int cumulative_weight = 0;
        for (const auto& server : servers) {
            cumulative_weight += server_weights.at(server);
            if (weight_index < cumulative_weight) {
                current_index = (current_index + 1) % total_weight;
                return server;
            }
        }
        return servers[0];  // Default fallback
    }
private:
    std::map<std::string, int> server_weights;
    size_t current_index;
};
//Health Check
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

/*std::vector<std::string> backend_servers = {"127.0.0.1:8081", "127.0.0.1:8082", "127.0.0.1:8083"};
int current_server = 0;
std::mutex server_mutex;
*/

std::vector<std::string> backend_servers = {"127.0.0.1:8081", "127.0.0.1:8082", "127.0.0.1:8083"};
std::map<std::string, int> server_metrics;  // To track the load on each server
//std::map<std::string, int> server_weights;
std::map<std::string, int> active_connections;
LoadBalancingAlgorithm* current_algorithm = nullptr;
std::mutex server_mutex;

void monitor_and_adapt() {
    while(true){
	std::this_thread::sleep_for(std::chrono::seconds(10));
        // Simulate updating server health status
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

        // Example adaptation logic
        bool traffic_spike = std::accumulate(active_connections.begin(), active_connections.end(), 0,
                                             [](int sum, const std::pair<std::string, int>& kv) {
                                                 return sum+kv.second;
                                             }) > 50;

        delete current_algorithm;

        if (traffic_spike) {
            current_algorithm = new LeastConnections(active_connections);
	    std::cout<<"Switched to Least Connections";
        } else if (std::all_of(active_connections.begin(), active_connections.end(),
                               [](const std::pair<std::string, int>& kv) {
                                   return kv.second == 0;
                               })) {
            current_algorithm = new RoundRobin();
	    std::cout<<"Switched to Round Robin";
        } else {
            current_algorithm = new WeightedRoundRobin(server_metrics);
            std::cout<<"Switched to Weighted Round Robin";
	}
    }
    /*Example: Adapt based on traffic metrics (dummy implementation)
    server_metrics["127.0.0.1:8081"] = rand() % 100;
    server_metrics["127.0.0.1:8082"] = rand() % 100;
    server_metrics["127.0.0.1:8083"] = rand() % 100;

    bool traffic_spike = std::accumulate(server_metrics.begin(), server_metrics.end(), 0,
        [](int sum, const std::pair<std::string, int>& kv) { return sum + kv.second; }) > 150;

    delete current_algorithm;
    if (traffic_spike) {
        current_algorithm = new LeastConnections(server_metrics);
	std::cout<<"Switched to Least Connections Load Balancing Algorithm due to traffic spike";
    } else if (std::all_of(server_metrics.begin(), server_metrics.end(),
               [](const std::pair<std::string, int>& kv) { return kv.second < 50; })) {
        current_algorithm = new RoundRobin();
	std::cout<<"Switched to Round Robin Load Balancing Algorithm with normal traffic";
    } else {
        current_algorithm = new WeightedRoundRobin(server_weights);
	std::cout<<"Switched to Weighted Round Robin Load Balancing Algorithm";
    }
    */
}

	
void handle_client(int client_socket) {
    std::string selected_server;
    bool server_healthy = false;

    while (!server_healthy) {
        std::lock_guard<std::mutex> lock(server_mutex);
        selected_server = current_algorithm->selectServer(backend_servers);

        std::string host = selected_server.substr(0, selected_server.find(':'));
        int port = std::stoi(selected_server.substr(selected_server.find(':') + 1));

        auto [healthy, _] = is_server_healthy(host, port);
        if (healthy) {
            server_healthy = true;
        } else {
            std::cout << "Unhealthy: " << host << ":" << port << std::endl;
        }
    }

    // Rest of the code to forward the request and response
    int backend_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (backend_socket == -1) {
        std::cerr << "Failed to create backend socket: " << strerror(errno) << std::endl;
        close(client_socket);
        return;
    }

    std::string host = selected_server.substr(0, selected_server.find(':'));
    int port = std::stoi(selected_server.substr(selected_server.find(':') + 1));

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

    // Forward client request to backend server
    char buffer[4096];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        send(backend_socket, buffer, bytes_received, 0);
        std::cout << "Sent to server" << std::endl;
    }

    // Forward backend response to client
    bytes_received = recv(backend_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        send(client_socket, buffer, bytes_received, 0);
        std::cout << "Received from server" << std::endl;
    }

    close(client_socket);
    close(backend_socket);
}

int main() {
    current_algorithm = new RoundRobin();  // Start with Round Robin
    std::cout<<"Starting with default Round Robin strategy";
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

    // Start monitoring and adapting the load balancing algorithm
    std::thread monitoring_thread(monitor_and_adapt);

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

    monitoring_thread.join();
    close(server_socket);
    delete current_algorithm;
    return 0;
}

/*
void handle_client(int client_socket){
    OLD CODE
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
*/
