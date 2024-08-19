#include "BaseServer.h"
#define _XOPEN_SOURCE_EXTENDED 1 
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <thread>
#include <chrono>
BaseServer::BaseServer(int port) : port(port) {}

void BaseServer::start() {
    int serverSocket = createServerSocket();
    if (serverSocket == -1) {
        std::cerr << "Failed to create server socket" << std::endl;
        return;
    }

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            std::cerr << "Failed to accept client connection" << std::endl;
            continue;
        }

        handleClient(clientSocket);
        close(clientSocket);
    }

    close(serverSocket);
}
int BaseServer::createServerSocket() {
    int sock = -1;
    int retries = 5;
    int opt = 1;

    for (int i = 0; i < retries; ++i) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            perror("socket");
            return -1;
        }
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            perror("setsockopt");
            close(sock);
            return -1;
        }
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("bind");
            close(sock);
            sock = -1;
            if (i < retries - 1) {
                std::cerr << "Retrying bind operation...\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } else {
                return -1;
            }
        } else {
            break;
        }
    }

    if (sock != -1 && listen(sock, SOMAXCONN) == -1) {
        std::cerr << "Failed to listen on socket\n";
        close(sock);
        return -1;
    }

    return sock;
}

/*
int BaseServer::createServerSocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(sock);
        return -1;
    }
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind");
        close(sock);
        return -1;
    }
    if (listen(sock, SOMAXCONN) == -1) {
        std::cerr << "Failed to listen on socket\n";
        return -1;
    }
    /
    if (listen(sock, 10) == -1) {
        perror("listen");
        close(sock);
        return -1;
    }
    /

    return sock;
}
*/
