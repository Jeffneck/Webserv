// WebServer.cpp
#include "WebServer.hpp"
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <signal.h>

// Externe, défini dans main.cpp
extern volatile bool g_running;

WebServer::WebServer() : config_(NULL) {}

WebServer::~WebServer() {
    cleanUp();
    if (config_ != NULL) {
        delete config_;
        config_ = NULL;
    }
}

void WebServer::loadConfiguration(const std::string& configFile) {
    try {
        ConfigParser parser(configFile);
        config_ = parser.parse();
        config_->displayConfig();//debug
    } catch (const ParsingException &e) {
        throw (e);
    }
}

void WebServer::start() {
    if (config_ == NULL) {
        throw std::runtime_error("Configuration not loaded.");
    }

    const std::vector<Server*>& servers = config_->getServers();
    listeningHandler_.initialize(servers);
    std::cout << "Server started with " << servers.size() << " servers." << std::endl;
}

void WebServer::runEventLoop() {
    std::cout << "WebServer::runEventLoop(): Démarrage de la boucle d'événements." << std::endl;
    // Ignorer SIGPIPE pour éviter que le programme ne se termine lors d'un Broken Pipe
    signal(SIGPIPE, SIG_IGN);
    while (g_running) {
        std::vector<struct pollfd> pollfds;
        std::vector<ListeningSocket*> pollListeningSockets;
        std::vector<DataSocket*> pollDataSockets;
        std::vector<int> pollFdTypes; // 0: ListeningSocket, 1: ClientSocket, 2: CgiPipe

        // Ajouter les sockets d'écoute
        const std::vector<ListeningSocket*>& listeningSockets = listeningHandler_.getListeningSockets();
        size_t i;
        for (i = 0; i < listeningSockets.size(); ++i) {
            struct pollfd pfd;
            pfd.fd = listeningSockets[i]->getSocket();
            pfd.events = POLLIN;
            pfd.revents = 0;
            pollfds.push_back(pfd);
            pollListeningSockets.push_back(listeningSockets[i]);
            pollDataSockets.push_back(NULL); // Pas de DataSocket pour les ListeningSocket
            pollFdTypes.push_back(0); // ListeningSocket
        }

        // Ajouter les sockets de données
        const std::vector<DataSocket*>& dataSockets = dataHandler_.getClientSockets();
        for (i = 0; i < dataSockets.size(); ++i) {
            DataSocket* dataSocket = dataSockets[i];

            // Socket client
            struct pollfd pfd;
            pfd.fd = dataSocket->getSocket();
            pfd.events = POLLIN | POLLOUT;
            pfd.revents = 0;
            pollfds.push_back(pfd);
            pollListeningSockets.push_back(NULL); // Pas de ListeningSocket pour les DataSocket
            pollDataSockets.push_back(dataSocket);
            pollFdTypes.push_back(1); // ClientSocket

            // Pipe CGI (si présent)
            if (dataSocket->hasCgiProcess() && !dataSocket->isCgiComplete()) {
                struct pollfd cgiPfd;
                cgiPfd.fd = dataSocket->getCgiPipeFd();
                // std::cout << "add pipe to Poll : "<< cgiPfd.fd << std::endl;//test
                cgiPfd.events = POLLIN;
                cgiPfd.revents = 0;
                pollfds.push_back(cgiPfd);
                pollListeningSockets.push_back(NULL);
                pollDataSockets.push_back(dataSocket);
                pollFdTypes.push_back(2); // CgiPipe
            }
        }

        
        // Appel à poll()
        int timeout = 12000; // Temps ms avant de sortir de l' etat de poll
        int ret = poll(&pollfds[0], pollfds.size(), timeout);
        if (ret < 0) {
            // perror("poll");
            break;
        }


        // Traitement des événements
        for (i = 0; i < pollfds.size(); ++i) {
            if (pollfds[i].revents == 0)
                continue;

            if (pollFdTypes[i] == 0) {
                // Socket d'écoute
                if (pollfds[i].revents & POLLIN) {
                    ListeningSocket* listeningSocket = pollListeningSockets[i];
                    int new_fd = listeningSocket->acceptConnection();
                    if (new_fd >= 0) {
                        DataSocket* newDataSocket = new DataSocket(new_fd, listeningSocket->getAssociatedServers(), config_);
                        dataHandler_.addClientSocket(newDataSocket);
                    }
                }
            } else if (pollFdTypes[i] == 1) {
                // Socket client
                DataSocket* dataSocket = pollDataSockets[i];
                if (pollfds[i].revents & POLLIN) {
                    std::cout << "POLLIN" << std::endl;
                    if (!dataSocket->receiveData()) {
                        std::cout << "POLLIN Datasocket close" << std::endl;
                        dataSocket->closeSocket();
                    } else if (dataSocket->isRequestComplete()) {
                        dataSocket->processRequest();
                        if (dataSocket->hasCgiProcess()) {
                            activeCgiSockets_.push_back(dataSocket);
                        }
                    }
                }
                if (pollfds[i].revents & POLLOUT) {
                    if (dataSocket->hasDataToSend()) {
                        if (!dataSocket->sendData()) {
                            dataSocket->closeSocket();
                        }
                    }
                }

                // if (pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                //     std::cout << "POLLHUP.. Datasocket close" << std::endl;
                //     dataSocket->closeSocket();
                // }
            } else if (pollFdTypes[i] == 2) {
                // Pipe CGI
                DataSocket* dataSocket = pollDataSockets[i];
                if (pollfds[i].revents & POLLIN) {
                    std::cout << GREEN<< "CGI POLLIN EVENT" << RESET <<std::endl;//test
                    dataSocket->readFromCgiPipe();
                }
                else if (pollfds[i].revents & (POLLHUP)) {
                    std::cout<< GREEN << "CGI POLLHUP EVENT"<< RESET <<std::endl;//test
                    dataSocket->readFromCgiPipe();
                    dataSocket->closeCgiPipe();
                }
                else if (pollfds[i].revents & (POLLERR | POLLNVAL)) {
                    dataSocket->closeCgiPipe();
                }
            }
        }

        checkCgiTimeouts();//verif si les timeouts ce sont declenches avant d' entrer dans poll
        checkDataSocketTimeouts();
        // Nettoyage des sockets fermées
        dataHandler_.removeClosedSockets();
    }
    std::cout << "Boucle d'événements terminée. Fermeture du serveur." << std::endl;

    // Appeler la fonction de nettoyage
    cleanUp();
}

void WebServer::checkCgiTimeouts() {
    std::vector<DataSocket*>::iterator it = activeCgiSockets_.begin();
    // int i = 0; //debug
    while (it != activeCgiSockets_.end()) {
        DataSocket* dataSocket = *it;
        // std::cout << "WebServer::checkCgiTimeouts"<< i++ << std::endl; // le programme n' arrive jamais ici
        if (dataSocket->hasCgiProcess()) {
            if (!dataSocket->cgiProcessIsRunning()) {
                // Le processus CGI n'est plus en cours d'exécution
                it = activeCgiSockets_.erase(it);
            } else if (dataSocket->cgiProcessHasTimedOut()) {
                // Le processus CGI a dépassé le délai maximal
                dataSocket->terminateCgiProcess();
                it = activeCgiSockets_.erase(it);
            } else {
                ++it;
            }
        } else {
            // Le DataSocket n'a plus de processus CGI
            it = activeCgiSockets_.erase(it);
        }
    }
}

void WebServer::checkDataSocketTimeouts() {
    std::cout << "WebServer::checkDataSocketTimeouts" << std::endl;
    const std::vector<DataSocket*>& dataSockets = dataHandler_.getClientSockets();
    time_t currentTime = time(NULL);

    for (size_t i = 0; i < dataSockets.size(); ++i) {
        DataSocket* dataSocket = dataSockets[i];
        if (difftime(currentTime, dataSocket->getLastActivityTime()) > INACTIVITY_TIMEOUT) {
            std::cout << RED << "DataSocket timeout. Closing inactive socket fd: " << dataSocket->getSocket() << RESET << std::endl;
            dataSocket->closeSocket();
        }
    }
}

void WebServer::cleanUp() {
    listeningHandler_.cleanUp();
    dataHandler_.cleanUp();
}
