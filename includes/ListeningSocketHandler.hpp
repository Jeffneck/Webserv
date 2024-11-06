#ifndef LISTENINGSOCKETHANDLER_HPP
#define LISTENINGSOCKETHANDLER_HPP

#include <vector>
#include <map>
#include <utility> // Pour std::pair
#include "ListeningSocket.hpp"
#include "Server.hpp"
#include "Exceptions.hpp"

class ListeningSocketHandler {
private:
    std::vector<ListeningSocket*> listeningSockets_; // Pointeurs vers les sockets d'écoute
    std::map<std::pair<uint32_t, uint16_t>, ListeningSocket*> listeningSocketsMap_; // Map IP:Port -> ListeningSocket*

public:
    ListeningSocketHandler();
    ~ListeningSocketHandler();

    void addListeningSocket(ListeningSocket* listeningSocket);
    const std::vector<ListeningSocket*>& getListeningSockets() const;

    void cleanUp(); // Méthode pour nettoyer tous les sockets

    // Nouvelle méthode pour initialiser les sockets d'écoute en fonction des serveurs
    void initialize(const std::vector<Server*>& servers);
};

#endif // LISTENINGSOCKETHANDLER_HPP
