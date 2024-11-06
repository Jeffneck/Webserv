#include "ListeningSocket.hpp"
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <iostream>

ListeningSocket::ListeningSocket(uint32_t host, uint16_t port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        // Gérer l'erreur
        std::cerr << "Erreur lors de la création du socket d'écoute" << std::endl;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        // Gérer l'erreur
        std::cerr << "Erreur lors de setsockopt" << std::endl;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = host;
    address.sin_port = port;

    //debug bind 
    // char ipStr[INET_ADDRSTRLEN];//test
    // inet_ntop(AF_INET, &host, ipStr, INET_ADDRSTRLEN);//test 
    // std::cout << "  listen: " << ipStr << ":" << ntohs(port) << std::endl;//test
    // std::cout << "LISTENINGSOCKET.cpp Constructeur : "<< ipStr << ":"<< ntohs(port) << " hexa :" << host << ":" << port << std::endl;//test

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        // Gérer l'erreur
        std::cerr << "Erreur lors du bind" << std::endl;
    }

    if (listen(server_fd, 10) < 0) //SOMAXCONN peut etre utilise a la place de 10 
    {
        // Gérer l'erreur
        std::cerr << "Erreur lors du listen" << std::endl;
    }
}

ListeningSocket::~ListeningSocket() {
    close(server_fd);
}

void ListeningSocket::addServer(Server* server) {
    associatedServers.push_back(server);
}

int ListeningSocket::acceptConnection() {
    int addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        // Gérer l'erreur
        std::cerr << "Erreur lors de l'accept" << std::endl;
    }
    return new_socket;
}

int ListeningSocket::getSocket() const {
    return server_fd;
}

const std::vector<Server*>& ListeningSocket::getAssociatedServers() const {
    return associatedServers;
}
