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
        configFile = "./configs/example.conf";
    }
    else if (argc == 2) {
        configFile = argv[1];
    }
    else {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    try {
        WebServer webServer;
        webServer.loadConfiguration(configFile);
        webServer.start();
        webServer.runEventLoop();

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::cout << "Info : Webserver is now inactive" << std::endl;
    return 0;
}
