#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <map>
#include "Config.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Exceptions.hpp"

// Consider renaming to ConfigManager if it goes beyond parsing
class ConfigParser {
public:
    ConfigParser(const std::string &filePath);
    ~ConfigParser();

    // Step 1: Parse filePath_
    Config* parse(); // Return a pointer to Config
    void displayParsingResult(); // Debug function to display the content of Config, Servers, and Locations

private:
    // Parsing methods
    void tokenize(const std::string &content);
    bool isNumber(const std::string &s);
    void parseTokens();
    void parseServer();
    void parseLocation(Server &server);
    // Auxiliary methods
    void parseSimpleDirective(const std::string &directiveName, std::string &value);
    void parseClientMaxBodySize(size_t &size);
    void parseErrorPage(Config &config);
    void parseErrorPage(Server &server);
    void parseListen(Server &server); // New method to parse 'listen'
    //check Methods
    void checkConfigValidity() const ; 
    // Step 2: Validate the entire configuration (possibly in a checkConfig class)

    // Private members
    size_t currentTokenIndex_;
    std::vector<std::string> tokens_;
    std::string filePath_;
    Config* config_;
};

#endif // CONFIGPARSER_HPP
