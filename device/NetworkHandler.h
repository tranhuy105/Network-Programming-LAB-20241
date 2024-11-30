#ifndef NETWORK_HANDLER_H
#define NETWORK_HANDLER_H

#include <string>
#include <sys/select.h>
#include "CommandHandler.h"
#include "lib/json.hpp"

class NetworkHandler {
private:
    CommandHandler& commandHandler;
    int tcpPort;
    std::string multicastIP = "239.255.255.250";
    int multicastPort = 1900;

    bool stopFlag = false;

public:
    NetworkHandler(CommandHandler& commandHandler, int tcpPort);
    ~NetworkHandler();

    void start();
    void stop();

private:
    void udpDiscovery();
    void tcpServer();

    int createServerSocket();
    void handleNewConnection(int serverSock, fd_set &masterSet, int &maxFd);
    bool handleClientRequest(int clientSock);
    void processJsonRequest(int clientSock, const nlohmann::json &requestJson);
    void closeClientConnection(int clientSock, fd_set &masterSet);
};

#endif // NETWORK_HANDLER_H
