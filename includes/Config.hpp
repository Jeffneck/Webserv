#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "Server.hpp"

class Server; // Forward declaration


/**
 * @class Config
 * 
 * The `Config` class is responsible for storing and managing the configuration of a web server.
 * It holds important global settings such as client request limits, error page mappings, 
 * and the root directory for serving files. It also manages a collection of `Server` objects
 * that represent individual web servers that may be configured to handle different IPs and ports.
 * 
 * This class allows the configuration data to be accessed and modified through various getter and setter methods.
 * It also provides utility methods for managing error pages, the root directory, and other server configurations.
 */
class Config
{
public:
    Config();
    ~Config();

    // Methods to access and modify global directives
    void setClientMaxBodySize(size_t size);
    size_t getClientMaxBodySize() const;

    void addErrorPage(int statusCode, const std::string &uri);
    const std::map<int, std::string> &getErrorPages() const;
    const std::string getErrorPage(int errorCode) const;
    const std::string getErrorPageFullPath(int errorCode) const;

    void setRoot(const std::string &root);
    const std::string &getRoot() const;

    void setIndex(const std::string &index);
    const std::string &getIndex() const;

    void addServer(Server* server);
    const std::vector<Server*>& getServers() const;

    // DEBUG: Display the content of the config
    void displayConfig() const;

private:
    size_t clientMaxBodySize_;
    std::map<int, std::string> errorPages_;
    std::string root_;
    std::string index_;
    std::vector<Server*> servers_;

};

#endif // CONFIG_HPP
