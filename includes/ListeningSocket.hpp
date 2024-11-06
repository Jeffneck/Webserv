#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP

#include <netinet/in.h>
#include <vector>
#include "Server.hpp"

class ListeningSocket {
private:
    int server_fd;                          // Descripteur du socket
    struct sockaddr_in address;             // Adresse associée au socket
    std::vector<Server*> associatedServers; // Pointeurs vers les serveurs associés

public:
    ListeningSocket(uint32_t host, uint16_t port);
    ~ListeningSocket();

    void addServer(Server* server);
    int acceptConnection();
    int getSocket() const;
    const std::vector<Server*>& getAssociatedServers() const;
};

#endif // LISTENINGSOCKET_HPP
