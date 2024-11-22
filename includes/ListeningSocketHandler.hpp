#ifndef LISTENINGSOCKETHANDLER_HPP
#define LISTENINGSOCKETHANDLER_HPP

#include <vector>
#include <map>
#include <utility>
#include "ListeningSocket.hpp"
#include "Server.hpp"
#include "Exceptions.hpp"


/**
 * @class ListeningSocketHandler
 * 
 * The `ListeningSocketHandler` class manages the listening sockets used by the web server. It is responsible 
 * for adding, storing, and cleaning up listening sockets, which are used to accept incoming client connections.
 * It also maintains a mapping of listening sockets based on their host and port combination.
 * 
 * - **Socket Management**: The class handles the creation and management of multiple listening sockets, 
 *   allowing the server to listen for incoming connections on various host-port combinations.
 * 
 * - **Socket Collection**: It stores a collection of listening sockets in a vector and maps them to a specific 
 *   host and port in a map, providing efficient lookup and management.
 * 
 * - **Server Initialization**: The handler is responsible for initializing listening sockets based on the provided 
 *   server configuration, associating them with the appropriate servers.
 * 
 * - **Cleanup**: The class provides a method to clean up all the listening sockets when the server shuts down, 
 *   ensuring resources are properly released.
 * 
 * This class is a central component in the server’s networking infrastructure, enabling it to listen for and 
 * manage incoming client connections on multiple sockets.
 */

class ListeningSocketHandler {
private:
    std::vector<ListeningSocket*> listeningSockets_; 
    std::map<std::pair<uint32_t, uint16_t>, ListeningSocket*> listeningSocketsMap_; 

public:
    ListeningSocketHandler();
    ~ListeningSocketHandler();

    void addListeningSocket(ListeningSocket* listeningSocket);
    const std::vector<ListeningSocket*>& getListeningSockets() const;

    void cleanUp();

    void initialize(const std::vector<Server*>& servers);
};

#endif // LISTENINGSOCKETHANDLER_HPP
