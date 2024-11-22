#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP

#include <netinet/in.h>
#include <vector>
#include "Server.hpp"


/**
 * @class ListeningSocket
 * 
 * The `ListeningSocket` class represents a listening socket used by a server (Webserver can manage more than 1 server) 
 * to accept incoming connections. It manages the network socket, binds it to a specified host and port, and allows the server 
 * to accept client connections.
 * 
 * - **Socket Management**: This class handles the creation and binding of a listening socket, which is used 
 *   to listen for incoming client connections.
 * 
 * - **Server Association**: It maintains a list of associated servers, allowing multiple servers to share 
 *   the same listening socket if needed.
 * 
 * - **Connection Handling**: The class provides methods to accept new client connections and retrieve 
 *   the socket for further communication.
 * 
 * This class is an essential component of the server infrastructure, enabling it to listen for and handle 
 * incoming client requests over the network.
 */

class ListeningSocket {
private:
    int listeningSocket_fd;
    struct sockaddr_in address;
    std::vector<Server*> associatedServers;

public:
    ListeningSocket(uint32_t host, uint16_t port);
    ~ListeningSocket();

    void addServer(Server* server);
    int acceptConnection();
    int getSocket() const;
    const std::vector<Server*>& getAssociatedServers() const;
};

#endif // LISTENINGSOCKET_HPP
