#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <vector>
#include <string>
#include <map>
#include <poll.h> 
#include "ListeningSocketHandler.hpp"
#include "DataSocketHandler.hpp"
#include "Config.hpp"
#include "ConfigParser.hpp"
#include "Color_Macros.hpp"


/**
 * @class WebServer
 * 
 * The `WebServer` class represents the main server that handles client requests, manages incoming connections, 
 * and processes CGI (Common Gateway Interface) requests. It is responsible for configuring the server, 
 * running the event loop, and managing both the listening sockets and data sockets. It also handles 
 * timeouts for both the client connections and CGI processes.
 * 
 * This class integrates the server's functionality, from reading the configuration file to running the 
 * server event loop, and ensures smooth multiplexing of client requests and timeouts.
 */

// Time to close inactive DataSockets in seconds
const time_t SOCKET_INACTIVITY_TIMEOUT = 45; 
const time_t MULTIPLEXING_LOOP_TIME = 45; 

class WebServer {
private:
    ListeningSocketHandler listeningHandler_;
    DataSocketHandler dataHandler_;           
    std::vector<DataSocket*> activeCgiSockets_;
    Config* config_;                         

public:
    WebServer();
    ~WebServer();

    // Prepare Webserver
    void loadConfiguration(const std::string& configFile);
    void start();
    
    // Running WebServer Loop
    void runEventLoop(); 
    void setupPollfds(std::vector<struct pollfd> &pollfds, std::vector<ListeningSocket*> &pollListeningSockets, std::vector<DataSocket*> &pollDataSockets, std::vector<int> &pollFdTypes);
    void checkCgiTimeouts(); 
    void checkDataSocketTimeouts(); 

    // Close exit Webserver
    void cleanUp(); 
};

#endif // WEBSERVER_HPP
