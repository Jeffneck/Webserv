// Server.cpp
#include "../includes/Server.hpp"
#include "../includes/Location.hpp"
#include <iostream>
#include <arpa/inet.h> // Pour inet_ntop

Server::Server(const Config &config)
    : config_(config), clientMaxBodySizeIsSet_(false), rootIsSet_(false), indexIsSet_(false),
      host_(INADDR_ANY), port_(htons(0))
{
}

Server::~Server()
{
}

void Server::addServerName(const std::string &serverName)
{
    serverNames_.push_back(serverName);
}

void Server::setRoot(const std::string &root)
{
    root_ = root;
    rootIsSet_ = true;
}

void Server::setIndex(const std::string &index)
{
    index_ = index;
    indexIsSet_ = true;
}

void Server::addErrorPage(int statusCode, const std::string &uri)
{
    errorPages_[statusCode] = uri;
}

void Server::setClientMaxBodySize(size_t size)
{
    clientMaxBodySize_ = size;
    clientMaxBodySizeIsSet_ = true;
}

uint32_t Server::getHost() const
{
    return host_;
}

uint16_t Server::getPort() const
{
    return port_;
}

void Server::setHost(uint32_t host)
{
    host_ = host;
}

void Server::setPort(uint16_t port)
{
    port_ = port;
}

const std::vector<std::string> &Server::getServerNames() const
{
    return serverNames_;
}

const std::string &Server::getRoot() const
{
    if (rootIsSet_)
        return root_;
    else
        return config_.getRoot();
}

const std::string &Server::getIndex() const
{
    if (indexIsSet_)
        return index_;
    else
        return config_.getIndex();
}

const std::map<int, std::string> &Server::getErrorPages() const
{
    if (!errorPages_.empty())
        return errorPages_;
    else
        return config_.getErrorPages();
}

const std::string Server::getErrorPage(int errorCode) const
{
    std::map<int, std::string>::const_iterator it = errorPages_.find(errorCode);
    if (it != errorPages_.end())
        return it->second;
    else
        return config_.getErrorPage(errorCode);
}

const std::string Server::getErrorPageFullPath(int errorCode) const
{
    return(getRoot() + getErrorPage(errorCode));
}


size_t Server::getClientMaxBodySize() const
{
    if (clientMaxBodySizeIsSet_)
        return clientMaxBodySize_;
    else
        return config_.getClientMaxBodySize();
}

void Server::addLocation(const Location &location)
{
    locations_.push_back(location);
}

const std::vector<Location> &Server::getLocations() const
{
    return locations_;
}

// DEBUG
void Server::displayServer() const
{
    // Afficher l'adresse IP et le port
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &host_, ipStr, INET_ADDRSTRLEN);
    std::cout << "  listen: " << ipStr << ":" << ntohs(port_) << std::endl;

    // Affichage des server_name du serveur
    const std::vector<std::string> &serverNames = this->getServerNames();
    for (size_t j = 0; j < serverNames.size(); ++j)
    {
        std::cout << "  server_name: " << serverNames[j] << std::endl;
    }

    // Affichage des pages d'erreur du serveur
    const std::map<int, std::string> &serverErrorPages = this->getErrorPages();
    for (std::map<int, std::string>::const_iterator it = serverErrorPages.begin(); it != serverErrorPages.end(); ++it)
    {
        std::cout << "  error_page " << it->first << " : " << it->second << std::endl;
    }

    // Affichage du contenu des Locations du serveur
    const std::vector<Location> &locations = this->getLocations();
    for (size_t k = 0; k < locations.size(); ++k)
    {
        const Location &location = locations[k];
        location.displayLocation();
    }
}
