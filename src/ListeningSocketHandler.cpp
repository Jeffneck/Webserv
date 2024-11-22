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
            // Vérifier si un ListeningSocket existe déjà pour cette IP:Port
            if (listeningSocketsMap_.find(key) == listeningSocketsMap_.end()) {
                // Créer un nouveau ListeningSocket
                ListeningSocket* newSocket = new ListeningSocket(host, port);
                listeningSocketsMap_[key] = newSocket;
                addListeningSocket(newSocket);
            }

            // Lier le serveur au ListeningSocket existant
            listeningSocketsMap_[key]->addServer(server);
        } catch (const std::runtime_error& e) {
            std::cerr << "Exception while initializing listening socket: " << e.what() << std::endl;
            // Ici, nous arrêtons le serveur si une erreur critique se produit lors de la création du socket
            throw std::runtime_error("Critical error: Unable to initialize listening socket, server will stop.");
        }
    }
}

