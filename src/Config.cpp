#include "../includes/Config.hpp"
#include "../includes/Color_Macros.hpp"
#include <iostream>

Config::Config() : 
    clientMaxBodySize_(0),
    errorPages_(),
    root_(""),
    index_(""),
    servers_()
{
}

Config::~Config()
{
    for (size_t i = 0; i < servers_.size(); ++i)
    {
        delete servers_[i];
    }
    servers_.clear();
}

void Config::setClientMaxBodySize(size_t size)
{
    clientMaxBodySize_ = size;
}

size_t Config::getClientMaxBodySize() const
{
    return clientMaxBodySize_;
}

void Config::addErrorPage(int statusCode, const std::string &uri)
{
    errorPages_[statusCode] = uri;
}

const std::map<int, std::string> &Config::getErrorPages() const
{
    return errorPages_;
}

const std::string Config::getErrorPage(int errorCode) const
{
    std::map<int, std::string>::const_iterator it = errorPages_.find(errorCode);
    if (it != errorPages_.end())
        return it->second;
    else
        return ""; 
}

const std::string Config::getErrorPageFullPath(int errorCode) const
{
    return(getRoot() + getErrorPage(errorCode));
}

void Config::setRoot(const std::string &root)
{
    root_ = root;
}

const std::string &Config::getRoot() const
{
    return root_;
}

void Config::setIndex(const std::string &index)
{
    index_ = index;
}

const std::string &Config::getIndex() const
{
    return index_;
}

void Config::addServer(Server* server)
{
    servers_.push_back(server);
}

const std::vector<Server*>& Config::getServers() const
{
    return servers_;
}

// Debug function
void Config::displayConfig() const
{
    std::cout << GREEN;
    std::cout << "Root global: " << this->getRoot() << std::endl;
    std::cout << "client_max_body_size global: " << this->getClientMaxBodySize() << std::endl;

    const std::map<int, std::string> &globalErrorPages = this->getErrorPages();
    for (std::map<int, std::string>::const_iterator it = globalErrorPages.begin(); it != globalErrorPages.end(); ++it)
    {
        std::cout << "error_page " << it->first << " : " << it->second << std::endl;
    }

    const std::vector<Server*>& servers = this->getServers();
    for (size_t i = 0; i < servers.size(); ++i)
    {
        std::cout << "  Server [" << i << "] :" << std::endl;
        const Server* server = servers[i];
        server->displayServer();
    }
    std::cout << RESET;
}
