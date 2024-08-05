#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class LoadBalancer {
public:
    LoadBalancer(const std::vector<std::string>& backend_addresses, unsigned short port)
        : backend_addresses_(backend_addresses), acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)), current_backend_(0) {}

    void run() {
        accept_connections();
        io_context_.run();
    }

private:
    void accept_connections() {
        auto socket = std::make_shared<tcp::socket>(io_context_);
        acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& ec) {
            if (!ec) {
                handle_request(socket);
            }
            accept_connections();
        });
    }

    void handle_request(std::shared_ptr<tcp::socket> socket) {
        auto backend_address = get_next_backend();
        auto backend_socket = std::make_shared<tcp::socket>(io_context_);
        auto resolver = std::make_shared<tcp::resolver>(io_context_);

        auto endpoint_iterator = resolver->resolve(tcp::resolver::query(backend_address, "http"));
        boost::asio::async_connect(*backend_socket, endpoint_iterator, [this, socket, backend_socket](const boost::system::error_code& ec, const tcp::resolver::iterator&) {
            if (!ec) {
                forward_request(socket, backend_socket);
                forward_response(backend_socket, socket);
            }
        });
    }

    void forward_request(std::shared_ptr<tcp::socket> client_socket, std::shared_ptr<tcp::socket> backend_socket) {
        auto buffer = std::make_shared<boost::asio::streambuf>();
        boost::asio::async_read_until(*client_socket, *buffer, "\r\n\r\n", [this, client_socket, backend_socket, buffer](const boost::system::error_code& ec, std::size_t) {
            if (!ec) {
                boost::asio::async_write(*backend_socket, *buffer, [this, client_socket, backend_socket](const boost::system::error_code& ec, std::size_t) {
                    if (!ec) {
                        forward_request(client_socket, backend_socket);
                    }
                });
            }
        });
    }

    void forward_response(std::shared_ptr<tcp::socket> backend_socket, std::shared_ptr<tcp::socket> client_socket) {
        auto buffer = std::make_shared<boost::asio::streambuf>();
        boost::asio::async_read(*backend_socket, *buffer, boost::asio::transfer_at_least(1), [this, client_socket, backend_socket, buffer](const boost::system::error_code& ec, std::size_t) {
            if (!ec) {
                boost::asio::async_write(*client_socket, *buffer, [this, client_socket, backend_socket](const boost::system::error_code& ec, std::size_t) {
                    if (!ec) {
                        forward_response(backend_socket, client_socket);
                    }
                });
            }
        });
    }

    std::string get_next_backend() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string backend = backend_addresses_[current_backend_];
        current_backend_ = (current_backend_ + 1) % backend_addresses_.size();
        return backend;
    }

    std::vector<std::string> backend_addresses_;
    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
    std::atomic<size_t> current_backend_;
    std::mutex mutex_;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: load_balancer <port> <backend1> [<backend2> ...]" << std::endl;
        return 1;
    }

    unsigned short port = static_cast<unsigned short>(std::stoi(argv[1]));
    std::vector<std::string> backend_addresses;
    for (int i = 2; i < argc; ++i) {
        backend_addresses.push_back(argv[i]);
    }

    LoadBalancer lb(backend_addresses, port);
    lb.run();

    return 0;
}

