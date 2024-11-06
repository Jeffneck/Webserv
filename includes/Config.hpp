#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "Server.hpp"

class Server; // Forward declaration

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
