#include "../includes/DataSocketHandler.hpp"

DataSocketHandler::DataSocketHandler() {
}

DataSocketHandler::~DataSocketHandler() {
    cleanUp();
}

void DataSocketHandler::addClientSocket(DataSocket* dataSocket) {
    clientSockets.push_back(dataSocket);
}

void DataSocketHandler::removeClosedSockets() {
    for (std::vector<DataSocket*>::iterator it = clientSockets.begin(); it != clientSockets.end(); ) {
        if ((*it)->getSocket() == -1) {
            delete *it;
            it = clientSockets.erase(it);
        } else {
            ++it;
        }
    }
}

const std::vector<DataSocket*>& DataSocketHandler::getClientSockets() const {
    return clientSockets;
}

void DataSocketHandler::cleanUp() {
    for (size_t i = 0; i < clientSockets.size(); ++i) {
        delete clientSockets[i];
    }
    clientSockets.clear();
}
