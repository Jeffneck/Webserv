#ifndef DATASOCKETHANDLER_HPP
#define DATASOCKETHANDLER_HPP

#include <vector>
#include "DataSocket.hpp"

class DataSocketHandler {
private:
    std::vector<DataSocket*> clientSockets; // Pointers to client data sockets

public:
    DataSocketHandler();
    ~DataSocketHandler();

    void addClientSocket(DataSocket* dataSocket);
    void handleClientSockets();
    void removeClosedSockets();
    const std::vector<DataSocket*>& getClientSockets() const;

    void cleanUp(); // Method to clean up all sockets
};

#endif // DATASOCKETHANDLER_HPP
