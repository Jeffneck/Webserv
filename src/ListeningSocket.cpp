#include "ListeningSocket.hpp"
#include "Utils.hpp"
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <iostream>

// ListeningSocket::ListeningSocket(uint32_t host, uint16_t port) {
//     listeningSocket_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (listeningSocket_fd == -1) {
//         std::cerr << "Error during the creation of a Listening Socket (server)" << std::endl;
//     }

//     int opt = 1;
//     if (setsockopt(listeningSocket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
//         std::cerr << "Error function setsockopt" << std::endl;
//     }


//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = host;
//     address.sin_port = port;

//     if (bind(listeningSocket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
//         std::cerr << "Error : bind the socket" << host << ":"<< port << "is impossible"  << std::endl;
//     }

//     if (listen(listeningSocket_fd, SOMAXCONN) < 0)
//     {
//         std::cerr << "Error : listen the socket" << host << ":"<< port << "is impossible"  << std::endl;
//     }
// }

std::string printIp(uint32_t host, uint16_t port) {
    char ipStr[INET_ADDRSTRLEN];
    struct in_addr ip_addr;
    ip_addr.s_addr = host;

    // Convertir l'adresse IP en notation pointée
    if (inet_ntop(AF_INET, &ip_addr, ipStr, sizeof(ipStr)) == NULL) {
        return "Invalid IP";
    }

    // Convertir le port en ordre hôte
    uint16_t hostPort = ntohs(port);

    // Construire la chaîne finale
    std::stringstream ss;
    ss << ipStr << ":" << hostPort;
    return ss.str();
}

ListeningSocket::ListeningSocket(uint32_t host, uint16_t port) {
    listeningSocket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket_fd == -1) {
        throw std::runtime_error("Error creating listening socket");
    }

    int opt = 1;
    if (setsockopt(listeningSocket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(listeningSocket_fd);
        throw std::runtime_error("Error setting socket options");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = host;
    address.sin_port = port;

    if (bind(listeningSocket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(listeningSocket_fd);
        throw std::runtime_error("Error binding socket to " + printIp(host, port));
    }

    if (listen(listeningSocket_fd, SOMAXCONN) < 0) {
        close(listeningSocket_fd);
        throw std::runtime_error("Error listening on socket " + printIp(host, port));
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

