#include "../includes/DataSocketHandler.hpp"
#include <assert.h>

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
        // DataSocket* temp = *it;
        delete *it;
        it = clientSockets.erase(it);
        // Vérifiez si l'objet est utilisé après suppression (hypothétique, dépend du contexte de votre programme)
        // assert(temp->getSocket() == -1 && "DataSocket accessed after deletion!");
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
