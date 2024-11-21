#include <iostream>
#include "../includes/WebServer.hpp"
#include "../includes/Exceptions.hpp"
#include <csignal>  // Pour signal()
// #include <cstring> 


volatile bool g_running = true;
void signalHandler(int signum) {
    std::cout << "\nInfo : Signal (" << signum << ") Webserver Gonna close..." << std::endl;
    g_running = false;
}

int main(int argc, char *argv[])
{
    // Signals used to stop Webserver properly (functionnality not recquired by the subject)
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::string configFile;
    if (argc == 1) {
        configFile = "./configs/webserv.conf";
    }
    else if (argc == 2) {
        configFile = argv[1];
    }
    else {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    try {
        // Créer une instance de WebServer
        WebServer webServer;

        // Charger la configuration à partir du fichier
        try {
            webServer.loadConfiguration(configFile);
            std::cout << "Info : Configuration loaded successfully" << std::endl;
        }
        catch (const ParsingException &e) {
            std::cout << e.what() << std::endl;
            return 1;
        }
        webServer.start();
        std::cout << "Info : Webserver is now running" << std::endl;
        webServer.runEventLoop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Info : Webserver is now inactive" << std::endl;
    return 0;
}
