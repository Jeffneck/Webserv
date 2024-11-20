#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <vector>
#include <string>
#include <map>
#include <utility> // Pour std::pair
#include <poll.h> 
#include "ListeningSocketHandler.hpp"
#include "DataSocketHandler.hpp"
#include "Config.hpp"
#include "ConfigParser.hpp"
#include "Color_Macros.hpp"

// Time to close inactive DataSockets in seconds
const time_t SOCKET_INACTIVITY_TIMEOUT = 45; 
const time_t MULTIPLEXING_LOOP_TIME = 45; 

class WebServer {
private:
    ListeningSocketHandler listeningHandler_; // Gère les sockets d'écoute
    DataSocketHandler dataHandler_;           // Gère les sockets de communication avec les clients
    std::vector<DataSocket*> activeCgiSockets_;//Gere les pipes de cgi ? 
    Config* config_;                          // Pointeur vers la configuration

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
