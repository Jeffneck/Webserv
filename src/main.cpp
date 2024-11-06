#include <iostream>
#include "../includes/WebServer.hpp"
#include "../includes/Exceptions.hpp"

int main(int argc, char *argv[])
{
    // Vérification des arguments
    std::string configFile;
    if (argc == 1) {
        configFile = "./nginxApp/webserv.conf";
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
        try
        {
            webServer.loadConfiguration(configFile);
            std::cout << "Configuration chargée avec succès." << std::endl;
        }
        catch (const ParsingException &e)
        {
            std::cout << e.what() << std::endl;
            return 1;
        }

        // Démarrer le serveur
        webServer.start();
        std::cout << "Serveur démarré." << std::endl;

        // Exécuter la boucle d'événements
        webServer.runEventLoop();
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
