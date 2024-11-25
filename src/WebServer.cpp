// WebServer.cpp
#include "../includes/WebServer.hpp"
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <signal.h>

// Extern, defined in main.cpp, monitored by signals (Ctrl+C SIGINT is a way to stop Webserver properly)
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
        // config_->displayConfig();// debug
    } catch (const ParsingException &e) {
        throw (e);
    }
    std::cout << "Info : Configuration loaded successfully" << std::endl;

}

void WebServer::start() {
    if (config_ == NULL) {
        throw std::runtime_error("Configuration file not loaded.");
    }

    const std::vector<Server*>& servers = config_->getServers();
    
    try {
        listeningHandler_.initialize(servers);
    } catch (const std::runtime_error& e) {
        std::cerr << "Server initialization failed " << std::endl;
        // Webserver will not run
        throw (e);
    }

    // Ignore SigPipe (broken pipe signal) 
    //=> a broken pipe (CGI error) will not make Webserver stop but need to send HTTP 500 code and close client connection
    signal(SIGPIPE, SIG_IGN);

    std::cout << "Info : WebServer is ready and is currently managing " << servers.size() << " servers." << std::endl; // debug
}


/**
 * @brief Runs the event loop for the web server, handling incoming events and socket communication.
 * 
 * This function implements an event-driven model using multiplexing with `poll()`, allowing the server to monitor 
 * multiple file descriptors (sockets and pipes) for input (`POLLIN`) and output (`POLLOUT`) events in a non-blocking manner. 
 * By using multiplexing, the server can efficiently handle multiple connections and processes concurrently without blocking on 
 * any single socket or operation. 
 * 
 * Specifically: POLLIN and POLLOUT are watched at THE same time for every DataSocket 
 * - **POLLIN** is monitored for sockets that have incoming data to read, indicating when a new request or data is available.
 * - **POLLOUT** is monitored for sockets that are ready to send data, allowing the server to send responses back to clients when they are ready.
 * 
 * The `poll()` system call monitors these events across multiple file descriptors, allowing the server to handle multiple 
 * requests and responses concurrently without wasting resources on blocked operations. 
 * 
 * The event loop also handles timeouts, processes CGI output, and closes idle or erroneous sockets as needed.
 */
void WebServer::runEventLoop() {
    std::cout << "Info : Webserver is now running" << std::endl;
    
    while (g_running) {
        //Pollfds stores all fds we want to keep an eye on : it is used to monitor events in multiplexing IO (non-blocking state)
        std::vector<struct pollfd> pollfds;
        
        //Utils allowing the access to sockets when an event is detected
        std::vector<ListeningSocket*> pollListeningSockets;
        std::vector<DataSocket*> pollDataSockets;

        //Used to identify the type of the fd watched (events are treated differently in function of the fd)
        std::vector<int> pollFdTypes; // 0: ListeningSocket, 1: ClientSocket, 2: CgiPipe

        //Setup structures
        setupPollfds(pollfds, pollListeningSockets, pollDataSockets, pollFdTypes);

        // Monitor Multiplexing I/O phase
        //      poll detect events and add flags to pollfds[i].revents
        //      if a flag is detected for a fd / or poll timeout :  Multiplexing I/O phase ends
        //      ret < 0 : Fatal Error or SIGINT
        int timeout = 5000;
        int ret = poll(&pollfds[0], pollfds.size(), timeout);
        if (ret < 0) {
            //poll failed, retry ..
            continue;
        }

        // Events are treated after Multiplexing I/O phase
        size_t i;
        for (i = 0; i < pollfds.size(); ++i) {
            if (pollfds[i].revents == 0)
                continue;
            // Listening Sockets
            if (pollFdTypes[i] == 0) {
                if (pollfds[i].revents & POLLIN) {
                    // std::cout << GREEN <<"LISTENINGSOCKET POLLIN" << RESET << std::endl;
                    ListeningSocket* listeningSocket = pollListeningSockets[i];
                    int new_fd = listeningSocket->acceptConnection();
                    if (new_fd >= 0) {
                        DataSocket* newDataSocket = new DataSocket(new_fd, listeningSocket->getAssociatedServers(), config_);
                        dataHandler_.addClientSocket(newDataSocket);
                    }
                }
            } 

            // Data Sockets
            else if (pollFdTypes[i] == 1) {
                DataSocket* dataSocket = pollDataSockets[i];
                if (pollfds[i].revents & POLLIN) {
                    // std::cout << GREEN <<"DATASOCKET POLLIN fd :" << pollfds[i].fd << RESET << std::endl;
                    if (!dataSocket->receiveData()) {
                        dataSocket->closeSocket();
                    } else if (dataSocket->isRequestComplete()) {
                        dataSocket->processRequest();
                        if (dataSocket->hasCgiProcess()) {
                            activeCgiSockets_.push_back(dataSocket);
                        }
                    }
                }if (pollfds[i].revents & POLLOUT) {
                    // std::cout << GREEN <<"DATASOCKET POLLOUT" << RESET << std::endl;
                    if (dataSocket->hasDataToSend()) {
                        if (!dataSocket->sendData()) {
                            dataSocket->closeSocket();
                        }
                    }
                }if (pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                    // std::cout << GREEN <<"DATASOCKET POLLHUP POLLERR POLLNVAL" << RESET << std::endl;
                    dataSocket->closeSocket();
                }

            } 

            // Pipes CGI 
            //      fd stored in poll = pipe / Datasocket that contains this fd = pollDataSockets[i]
            else if (pollFdTypes[i] == 2) {
                DataSocket* dataSocket = pollDataSockets[i];
                //Data sent by CGI
                if (pollfds[i].revents & POLLIN) {
                    // std::cout << GREEN<< "CGI POLLIN EVENT" << RESET <<std::endl;
                    dataSocket->readFromCgiPipe();
                }
                //EOF sent by CGI
                else if (pollfds[i].revents & (POLLHUP)) {
                    // std::cout<< GREEN << "CGI POLLHUP EVENT"<< RESET <<std::endl;
                    dataSocket->readFromCgiPipe();
                    dataSocket->closeCgiPipe();
                }
                else if (pollfds[i].revents & (POLLERR | POLLNVAL)) {
                    // std::cout<< GREEN << "CGI POLLERR EVENT"<< RESET <<std::endl;//test
                    dataSocket->closeCgiPipe();
                }
            }
        }

        //Events triggered after each multiplexing session
        checkCgiTimeouts();
        checkDataSocketTimeouts();
        dataHandler_.removeClosedSockets();
    }
    //Events triggered afet a SIGINT (not recquired by the subject but useful)
    std::cout << "Info : Webserver had been shut down" << std::endl;
    cleanUp();
}

void WebServer::setupPollfds(std::vector<struct pollfd> &pollfds,
        std::vector<ListeningSocket*> &pollListeningSockets,
        std::vector<DataSocket*> &pollDataSockets,
        std::vector<int> &pollFdTypes)
{
    // Add Listening Sockets to pollfds 
        //      a ListeningSocket is setup at IP:PORT of every server
        //      Listening sockets are used to detect new connections and setup Datasockets for every client
        const std::vector<ListeningSocket*>& listeningSockets = listeningHandler_.getListeningSockets();
        size_t i;
        for (i = 0; i < listeningSockets.size(); ++i) {
            struct pollfd pfd;
            pfd.fd = listeningSockets[i]->getSocket();
            pfd.events = POLLIN;
            pfd.revents = 0;//reset revent
            pollfds.push_back(pfd);
            pollListeningSockets.push_back(listeningSockets[i]);
            pollDataSockets.push_back(NULL); // Pas de DataSocket pour les ListeningSocket
            pollFdTypes.push_back(0); // ListeningSocket
        }

        // Add dataSockets to pollfds for every client :
        //      a Datasocket is created when a new client connects to a ListeningSocket 
        //      multiples clients can be handled by 1 server
        //      Datasockets are used to exchange with clients in HTTP
        const std::vector<DataSocket*>& dataSockets = dataHandler_.getClientSockets();
        for (i = 0; i < dataSockets.size(); ++i) {
            DataSocket* dataSocket = dataSockets[i];
            struct pollfd pfd;
            pfd.fd = dataSocket->getSocket();
            pfd.events = POLLIN;
            if(dataSocket->hasDataToSend())
                pfd.events |= POLLOUT;
            pfd.revents = 0; //reset revent
            pollfds.push_back(pfd);
            pollListeningSockets.push_back(NULL); // No ListeningSocket in Datasockets
            pollDataSockets.push_back(dataSocket);
            pollFdTypes.push_back(1); // ClientSocket

            // Add Pipe CGI to pollfds when a client request a file that needs to be exec by a CGI (Python here)
            if (dataSocket->hasCgiProcess() && !dataSocket->isCgiComplete()) {
                struct pollfd cgiPfd;
                cgiPfd.fd = dataSocket->getCgiPipeFd();
                cgiPfd.events = POLLIN;
                cgiPfd.revents = 0; //reset revent
                pollfds.push_back(cgiPfd);
                pollListeningSockets.push_back(NULL);
                pollDataSockets.push_back(dataSocket);
                pollFdTypes.push_back(2); // CgiPipe
            }
        }
}

void WebServer::checkCgiTimeouts() {
    std::vector<DataSocket*>::iterator it = activeCgiSockets_.begin();
    usleep(500);
    while (it != activeCgiSockets_.end()) {
        DataSocket* dataSocket = *it;
        if (dataSocket->hasCgiProcess()) {
            if (!dataSocket->cgiProcessIsRunning()) {
                // CGI process is not running anymore in Datasocket
                it = activeCgiSockets_.erase(it);
            } else if (dataSocket->cgiProcessHasTimedOut()) {
                // CGI process timed out
                dataSocket->terminateCgiProcess(504);
                it = activeCgiSockets_.erase(it);
            } else {
                // CGI process still running properly
                ++it;
            }
        } else {
            // Datasocket have no more CGI process
            it = activeCgiSockets_.erase(it);
        }
    }
}

void WebServer::checkDataSocketTimeouts() {
    const std::vector<DataSocket*>& dataSockets = dataHandler_.getClientSockets();
    time_t currentTime = time(NULL);

    for (size_t i = 0; i < dataSockets.size(); ++i) {
        DataSocket* dataSocket = dataSockets[i];
        if (difftime(currentTime, dataSocket->getLastActivityTime()) > SOCKET_INACTIVITY_TIMEOUT) {
            dataSocket->closeSocket();
        }
    }
}

void WebServer::cleanUp() {
    listeningHandler_.cleanUp();
    dataHandler_.cleanUp();
}
