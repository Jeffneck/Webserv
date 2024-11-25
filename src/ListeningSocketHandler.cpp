#include "../includes/ListeningSocketHandler.hpp"
// #include <arpa/inet.h> // Pour htons, htonl
#include <iostream>

ListeningSocketHandler::ListeningSocketHandler() {
}

ListeningSocketHandler::~ListeningSocketHandler() {
    cleanUp();
}

void ListeningSocketHandler::addListeningSocket(ListeningSocket* listeningSocket) {
    listeningSockets_.push_back(listeningSocket);
}

const std::vector<ListeningSocket*>& ListeningSocketHandler::getListeningSockets() const {
    return listeningSockets_;
}

void ListeningSocketHandler::cleanUp() {
    for (size_t i = 0; i < listeningSockets_.size(); ++i) {
        delete listeningSockets_[i];
    }
    listeningSockets_.clear();
    listeningSocketsMap_.clear();
}

void ListeningSocketHandler::initialize(const std::vector<Server*>& servers) {
    for (size_t i = 0; i < servers.size(); ++i) {
        Server* server = servers[i];
        uint32_t host = server->getHost();
        uint16_t port = server->getPort();
        std::pair<uint32_t, uint16_t> key(host, port);

        try {
            // research if a socket already exists at this ip:port
            if (listeningSocketsMap_.find(key) == listeningSocketsMap_.end()) {
                ListeningSocket* newSocket = new ListeningSocket(host, port);
                listeningSocketsMap_[key] = newSocket;
                addListeningSocket(newSocket);
            }

            // link Listening socket to the server config associated
            listeningSocketsMap_[key]->addServer(server);
        } catch (const std::runtime_error& e) {
            // Webserv will not run, critical error
            throw e;
        }
    }
}

