#ifndef BASESERVER_H
#define BASESERVER_H

#include <string>

class BaseServer {
public:
    BaseServer(int port);
    virtual ~BaseServer() = default;

    void start();

protected:
    virtual void handleClient(int clientSocket) = 0; // Pure virtual function to be implemented by derived classes

private:
    int port;
    int createServerSocket();
};

#endif

