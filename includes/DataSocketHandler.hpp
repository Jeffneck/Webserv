#ifndef DATASOCKETHANDLER_HPP
#define DATASOCKETHANDLER_HPP

#include <vector>
#include "DataSocket.hpp"

/**
 * @class DataSocketHandler
 * 
 * The `DataSocketHandler` class is responsible for managing multiple client sockets in the web server. 
 * It keeps track of active client connections and ensures proper socket cleanup when a 
 * connection is closed.
 * 
 * - **Socket Management**: The class handles adding and removing client sockets, ensuring that only active 
 *   sockets are kept in the list of client sockets.
 * 
 * - **Cleanup**: The class ensures that closed or inactive sockets are removed, freeing up resources and preventing 
 *   resource leaks.
 * 
 * This class is an essential component of the web server's infrastructure, ensuring proper management of client 
 * connections, resource cleanup, and overall handling of client-server communication.
 */

class DataSocketHandler {
private:
    std::vector<DataSocket*> clientSockets;

public:
    DataSocketHandler();
    ~DataSocketHandler();

    void addClientSocket(DataSocket* dataSocket);
    void handleClientSockets();
    void removeClosedSockets();
    const std::vector<DataSocket*>& getClientSockets() const;

    void cleanUp();
};

#endif // DATASOCKETHANDLER_HPP
