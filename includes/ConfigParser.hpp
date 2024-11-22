#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <map>
#include "Config.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Exceptions.hpp"

/**
 * @class ConfigParser
 * 
 * The `ConfigParser` class is responsible for parsing the configuration file for the web server. 
 * It reads the configuration file, tokenizes the content, and processes each section (e.g., server, location) 
 * to generate a `Config` object that holds all the server settings. This class validates and parses directives 
 * in the configuration, such as server names, error pages, body size limits, and more.
 * 
 * - **Configuration Parsing**: The class tokenizes the configuration file and processes the tokens to 
 *   build a structured representation of the server's configuration.
 * 
 * - **Server and Location Parsing**: It extracts global (= Config) server and location-specific configurations from 
 *   the parsed tokens, setting up the correct properties such as root directory, allowed methods, and 
 *   error pages.
 * 
 * - **Directive Handling**: The class handles various configuration directives (e.g., `client_max_body_size`, 
 *   `error_page`, `listen`) and ensures they are correctly parsed and stored.
 * 
 * - **Configuration Validation**: The class performs checks to ensure that the configuration file is valid, 
 *   ensuring that all required directives are set and that their values are correct.
 * 
 * This class is a key component of the web server, enabling it to correctly interpret and load the server's 
 * configuration file and apply the settings to the server and its locations.
 */
class ConfigParser {
public:
    ConfigParser(const std::string &filePath);
    ~ConfigParser();

    
    Config* parse();

    // DEBUG : display the content of Config, Servers, and Locations
    void displayParsingResult(); 

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
    void parseListen(Server &server);

    //check Methods
    void checkConfigValidity() const ; 

    size_t currentTokenIndex_;
    std::vector<std::string> tokens_;
    std::string filePath_;
    Config* config_;
};

#endif // CONFIGPARSER_HPP
