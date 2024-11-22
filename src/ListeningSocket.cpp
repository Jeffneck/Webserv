#include "ListeningSocket.hpp"
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <iostream>

ListeningSocket::ListeningSocket(uint32_t host, uint16_t port) {
    listeningSocket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket_fd == -1) {
        std::cerr << "Error during the creation of a Listening Socket (server)" << std::endl;
    }

    int opt = 1;
    if (setsockopt(listeningSocket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error function setsockopt" << std::endl;
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = host;
    address.sin_port = port;

    if (bind(listeningSocket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Error : bind the socket is impossible" << std::endl;
    }

    if (listen(listeningSocket_fd, SOMAXCONN) < 0)
    {
        std::cerr << "Error : listen the socket is impossible" << std::endl;
    }
}

ListeningSocket::~ListeningSocket() {
    close(listeningSocket_fd);
}

void ListeningSocket::addServer(Server* server) {
    associatedServers.push_back(server);
}

int ListeningSocket::acceptConnection() {
    int addrlen = sizeof(address);
    int new_socket = accept(listeningSocket_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        std::cerr << "Error creating a new Datasocket (communication with a new client can't be accepted)" << std::endl;
    }
    return new_socket;
}

int ListeningSocket::getSocket() const {
    return listeningSocket_fd;
}

const std::vector<Server*>& ListeningSocket::getAssociatedServers() const {
    return associatedServers;
}
