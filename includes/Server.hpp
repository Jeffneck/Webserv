// Server.hpp
#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <netinet/in.h> // Pour les types réseau
#include "Location.hpp"
#include "Config.hpp"

class Config;   // Forward declaration
class Location; // Forward declaration


/**
 * @class Server
 * 
 * The `Server` class is responsible for encapsulating the configuration settings of a web server. It contains directives related to 
 * server behavior such as server name, root directory, index file, error pages, and client request size limits. Additionally, it 
 * stores specific information about the server's network settings such as IP address and port, and a list of locations which define 
 * paths on the server with specific configurations.
 * 
 * The `Server` class is designed to allow easy access to and modification of server-related configuration settings. It can handle 
 * multiple server names, locations, and error pages, providing a complete configuration for a single server instance.
 */
class Server
{
public:
    Server(const Config &config);
    ~Server();

    // Méthodes pour accéder et modifier les directives du serveur
    void addServerName(const std::string &serverName);
    void setRoot(const std::string &root);
    void setIndex(const std::string &index);
    void addErrorPage(int statusCode, const std::string &uri);
    void setClientMaxBodySize(size_t size);

    uint32_t getHost() const; // Nouvelle méthode pour obtenir l'adresse IP
    uint16_t getPort() const; // Nouvelle méthode pour obtenir le port
    void setHost(uint32_t host); // Setter pour host_
    void setPort(uint16_t port); // Setter pour port_
    const std::vector<std::string> &getServerNames() const;
    const std::string &getRoot() const;
    const std::string &getIndex() const;
    size_t getClientMaxBodySize() const;


    const std::map<int, std::string> &getErrorPages() const;
    const std::string getErrorPage(int errorCode) const;
    const std::string getErrorPageFullPath(int errorCode) const;

    void addLocation(const Location &location);
    const std::vector<Location> &getLocations() const;

    // DEBUG
    void displayServer() const;

private:
    const Config &config_; // Référence au Config parent

    // Indicateurs pour savoir si les valeurs sont définies localement
    bool clientMaxBodySizeIsSet_;
    bool rootIsSet_;
    bool indexIsSet_;

    // Directives pouvant être héritées
    size_t clientMaxBodySize_;
    std::map<int, std::string> errorPages_;
    std::string root_;
    std::string index_;

    // Directives spécifiques au serveur
    uint32_t host_; // Adresse IP en ordre réseau
    uint16_t port_; // Numéro de port en ordre réseau
    std::vector<std::string> serverNames_;
    std::vector<Location> locations_;
};

#endif // SERVER_HPP
