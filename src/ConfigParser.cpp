// ConfigParser.cpp
#include "../includes/ConfigParser.hpp"
#include "../includes/Server.hpp"
#include "../includes/Location.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <arpa/inet.h>
#include <limits>


ConfigParser::ConfigParser(const std::string &filePath)
    : currentTokenIndex_(0), filePath_(filePath), config_(NULL)
{
}

ConfigParser::~ConfigParser(){}

Config* ConfigParser::parse()
{
    std::ifstream file(filePath_.c_str());
    if (!file.is_open())
    {
        throw ParsingException("Error : Config File can't be opened" + filePath_);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    config_ = new Config();

    tokenize(buffer.str());
    parseTokens();
    try 
    {
        checkConfigValidity();
    }
    catch (ParsingException &e)
    {
        delete config_;
        config_ = NULL;
        throw (e);
    }

    return config_;
}

void ConfigParser::tokenize(const std::string &content)
{
    std::string token;
    bool inQuotes = false;
    for (size_t i = 0; i < content.length(); ++i)
    {
        char c = content[i];

        if (c == '#')
        {
            while (i < content.length() && content[i] != '\n')
                ++i;
        }
        else if (std::isspace(c) && !inQuotes)
        {
            if (!token.empty())
            {
                tokens_.push_back(token);
                token.clear();
            }
        }
        else if (c == '"' || c == '\'')
        {
            inQuotes = !inQuotes;
            token += c;
        }
        else if ((c == '{' || c == '}' || c == ';') && !inQuotes)
        {
            if (!token.empty())
            {
                tokens_.push_back(token);
                token.clear();
            }
            tokens_.push_back(std::string(1, c));
        }
        else
        {
            token += c;
        }
    }
    if (!token.empty())
    {
        tokens_.push_back(token);
    }
}

void ConfigParser::parseTokens()
{
    while (currentTokenIndex_ < tokens_.size())
    {
        std::string token = tokens_[currentTokenIndex_];
        if (token == "server")
        {
            parseServer();
        }
        else
        {
            
            if (token == "client_max_body_size")
            {
                size_t size;
                parseClientMaxBodySize(size);
                config_->setClientMaxBodySize(size);
            }
            else if (token == "root")
            {
                std::string rootValue;
                parseSimpleDirective("root", rootValue);
                config_->setRoot(rootValue);
            }
            else if (token == "error_page")
            {
                parseErrorPage(*config_);
            }
            else
            {
                throw ParsingException("Unknown Directive in the context 'global': " + token);
            }
        }
    }
}



// Méthode pour vérifier si une chaîne est un nombre
bool ConfigParser::isNumber(const std::string &s)
{
    for (size_t i = 0; i < s.length(); ++i)
    {
        if (!isdigit(s[i]))
            return false;
    }
    return true;
}

// Méthode pour parser une directive simple avec un point-virgule
void ConfigParser::parseSimpleDirective(const std::string &directiveName, std::string &value)
{
    ++currentTokenIndex_;
    if (currentTokenIndex_ >= tokens_.size())
        throw ParsingException("Value needed after '" + directiveName + "'");
    value = tokens_[currentTokenIndex_];
    ++currentTokenIndex_;
    if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != ";")
        throw ParsingException("';' needed after '" + directiveName + "'");
    ++currentTokenIndex_;
}

// Méthode pour parser 'client_max_body_size'
void ConfigParser::parseClientMaxBodySize(size_t &size)
{
    ++currentTokenIndex_;
    if (currentTokenIndex_ >= tokens_.size())
        throw ParsingException("Value needed after 'client_max_body_size'");

    // Récupérer le token représentant la taille
    std::string sizeToken = tokens_[currentTokenIndex_];

    // Vérifier que le token n'est pas vide
    if (sizeToken.empty())
        throw ParsingException("empty value for 'client_max_body_size'");

    // Variables pour stocker la partie numérique et l'unité
    std::string numericPart;
    std::string unitPart;

    // Parcourir le token pour séparer la partie numérique de l'unité
    size_t pos = 0;
    while (pos < sizeToken.length() && isdigit(sizeToken[pos]))
    {
        numericPart += sizeToken[pos];
        ++pos;
    }

    // Vérifier qu'il y a bien une partie numérique
    if (numericPart.empty())
        throw ParsingException("Numeric value needed for 'client_max_body_size'");

    // Récupérer la partie unité
    unitPart = sizeToken.substr(pos);

    // Convertir la partie numérique en nombre
    char *endptr;
    errno = 0; // Réinitialiser errno avant l'appel
    unsigned long numericValue = strtoul(numericPart.c_str(), &endptr, 10);
    if (*endptr != '\0' || errno == ERANGE)
        throw ParsingException("Invalid numeric value for'client_max_body_size'");

    // Vérifier que numericValue peut être converti en size_t sans débordement
    if (numericValue > static_cast<unsigned long>(std::numeric_limits<size_t>::max()))
        throw ParsingException("Too big value for 'client_max_body_size'");

    size_t sizeInBytes = static_cast<size_t>(numericValue);

    // Gérer l'unité si présente
    if (!unitPart.empty())
    {
        if (unitPart == "k" || unitPart == "K")
        {
            if (sizeInBytes > std::numeric_limits<size_t>::max() / 1024)
                throw ParsingException("Too big value for 'client_max_body_size'");
            sizeInBytes *= 1024;
        }
        else if (unitPart == "m" || unitPart == "M")
        {
            if (sizeInBytes > std::numeric_limits<size_t>::max() / (1024 * 1024))
                throw ParsingException("Too big value for 'client_max_body_size'");
            sizeInBytes *= 1024 * 1024;
        }
        else if (unitPart == "g" || unitPart == "G")
        {
            if (sizeInBytes > std::numeric_limits<size_t>::max() / (static_cast<size_t>(1024) * 1024 * 1024))
                throw ParsingException("Too big value for 'client_max_body_size'");
            sizeInBytes *= static_cast<size_t>(1024) * 1024 * 1024;
        }
        else
        {
            throw ParsingException("Invalid unit after'client_max_body_size' (need 'k', 'm' or'g')");
        }
    }

    // Affecter la taille
    size = sizeInBytes;

    ++currentTokenIndex_;
    if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != ";")
        throw ParsingException("';' needed after 'client_max_body_size'");
    ++currentTokenIndex_;
}
// Méthode pour parser 'error_page' pour Config
void ConfigParser::parseErrorPage(Config &config)
{
    ++currentTokenIndex_;
    if (currentTokenIndex_ >= tokens_.size())
        throw ParsingException("Value needed after 'error_page'");

    // Collecter les codes d'état
    std::vector<int> statusCodes;
    while (currentTokenIndex_ < tokens_.size() && isNumber(tokens_[currentTokenIndex_]))
    {
        int code = std::atoi(tokens_[currentTokenIndex_].c_str());
        statusCodes.push_back(code);
        ++currentTokenIndex_;
    }

    if (statusCodes.empty())
        throw ParsingException("Error code needed after 'error_page'");

    // Vérifier qu'il y a une URI après les codes d'état
    if (currentTokenIndex_ >= tokens_.size())
        throw ParsingException("URI needed after Error code 'error_page'");

    std::string uri = tokens_[currentTokenIndex_];
    ++currentTokenIndex_;

    if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != ";")
        throw ParsingException("';' needed after URI 'error_page'");
    ++currentTokenIndex_;

    // Ajouter les associations code d'état -> URI dans la configuration
    for (size_t i = 0; i < statusCodes.size(); ++i)
    {
        config.addErrorPage(statusCodes[i], uri);
    }
}

// Méthode pour parser 'error_page' pour Server
void ConfigParser::parseErrorPage(Server &server)
{
    ++currentTokenIndex_;
    if (currentTokenIndex_ >= tokens_.size())
        throw ParsingException("Value needed after 'error_page'");

    // Collecter les codes d'état
    std::vector<int> statusCodes;
    while (currentTokenIndex_ < tokens_.size() && isNumber(tokens_[currentTokenIndex_]))
    {
        int code = std::atoi(tokens_[currentTokenIndex_].c_str());
        statusCodes.push_back(code);
        ++currentTokenIndex_;
    }

    if (statusCodes.empty())
        throw ParsingException("Error code needed after 'error_page'");

    // Vérifier qu'il y a une URI après les codes d'état
    if (currentTokenIndex_ >= tokens_.size())
        throw ParsingException("URI needed after 'error_page'");

    std::string uri = tokens_[currentTokenIndex_];
    ++currentTokenIndex_;

    if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != ";")
        throw ParsingException("';' needed after 'error_page'");
    ++currentTokenIndex_;

    // Ajouter les associations code d'état -> URI dans le serveur
    for (size_t i = 0; i < statusCodes.size(); ++i)
    {
        server.addErrorPage(statusCodes[i], uri);
    }
}


void ConfigParser::parseServer()
{
    ++currentTokenIndex_;
    if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != "{")
        throw ParsingException("'{' needed after 'server'");
    ++currentTokenIndex_;

    Server* server = new Server(*config_);

    while (currentTokenIndex_ < tokens_.size())
    {
        std::string token = tokens_[currentTokenIndex_];
        if (token == "}")
        {
            ++currentTokenIndex_;
            break;
        }
        else if (token == "listen")
        {
            parseListen(*server);
        }
        else if (token == "server_name")
        {
            ++currentTokenIndex_;
            while (currentTokenIndex_ < tokens_.size() && tokens_[currentTokenIndex_] != ";")
            {
                server->addServerName(tokens_[currentTokenIndex_]);
                ++currentTokenIndex_;
            }
            if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != ";")
                throw ParsingException("';' needed after 'server_name'");
            ++currentTokenIndex_;
        }
        else if (token == "root")
        {
            std::string rootValue;
            parseSimpleDirective("root", rootValue);
            server->setRoot(rootValue);
        }
        else if (token == "index")
        {
            std::string indexValue;
            parseSimpleDirective("index", indexValue);
            server->setIndex(indexValue);
        }
        else if (token == "error_page")
        {
            parseErrorPage(*server);
        }
        else if (token == "client_max_body_size")
        {
            size_t size;
            parseClientMaxBodySize(size);
            server->setClientMaxBodySize(size);
        }
        else if (token == "location")
        {
            parseLocation(*server);
        }
        else
        {
            throw ParsingException("Unknown Directive in the context  'server': " + token);
        }
    }
    config_->addServer(server);
}

void ConfigParser::parseListen(Server &server)
{
    ++currentTokenIndex_;
    if (currentTokenIndex_ >= tokens_.size())
        throw ParsingException("Value needed after 'listen'");

    std::string listenValue = tokens_[currentTokenIndex_];
    ++currentTokenIndex_;
    if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != ";")
        throw ParsingException("';' needed after 'listen'");
    ++currentTokenIndex_;

    std::string ipPart = "0.0.0.0";
    std::string portPart;

    size_t colonPos = listenValue.find(':');
    if (colonPos != std::string::npos)
    {
        ipPart = listenValue.substr(0, colonPos);
        portPart = listenValue.substr(colonPos + 1);
    }
    else
    {
        portPart = listenValue;
    }

    // IP char** to uint32_t(network)
    struct in_addr addr;
    if (inet_pton(AF_INET, ipPart.c_str(), &addr) != 1)
    {
        throw ParsingException("Invalid IP Adress in 'listen': " + ipPart);
    }

    // Port network to int
    int port = std::atoi(portPart.c_str());
    if (port <= 0 || port > 65535)
    {
        throw ParsingException("Invalid Port number in 'listen': " + portPart);
    }

    server.setHost(addr.s_addr);
    // Port : int to uint16_t (network format)
    server.setPort(htons(static_cast<uint16_t>(port)));    
    // std::cout << "CONFIGPARSER.cpp parseListen : "<< ipPart << ":"<<portPart << " hexa :" << server.getHost()<< ":" << server.getPort() << std::endl;//test
}

void ConfigParser::parseLocation(Server &server)
{
    ++currentTokenIndex_;
    if (currentTokenIndex_ >= tokens_.size())
        throw ParsingException("Expecting path before 'location' block");

    std::string path = tokens_[currentTokenIndex_];
    ++currentTokenIndex_;

    if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != "{")
        throw ParsingException("'{' Expected after 'location' path ");
    ++currentTokenIndex_;

    Location location(server, path);

    while (currentTokenIndex_ < tokens_.size())
    {
        std::string token = tokens_[currentTokenIndex_];
        if (token == "}")
        {
            ++currentTokenIndex_;
            break;
        }
        else if (token == "root")
        {
            std::string rootValue;
            parseSimpleDirective("root", rootValue);
            location.setRoot(rootValue);
        }
        else if (token == "client_max_body_size")
        {
            size_t size;
            parseClientMaxBodySize(size);
            location.setClientMaxBodySize(size);
        }
        else if (token == "index")
        {
            std::string indexValue;
            parseSimpleDirective("index", indexValue);
            location.setIndex(indexValue);
        }
        else if (token == "autoindex")
        {
            ++currentTokenIndex_;
            if (currentTokenIndex_ >= tokens_.size())
                throw ParsingException("'on' or'off' needed after 'autoindex'");
            if (tokens_[currentTokenIndex_] == "on")
                location.setAutoIndex(true);
            else if (tokens_[currentTokenIndex_] == "off")
                location.setAutoIndex(false);
            else
                throw ParsingException("Invalid value for 'autoindex': " + tokens_[currentTokenIndex_]);
            ++currentTokenIndex_;
            if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != ";")
                throw ParsingException("';' needed after 'autoindex' value");
            ++currentTokenIndex_;
        }

        else if (token == "cgi")
        {
            ++currentTokenIndex_;
            if (currentTokenIndex_ >= tokens_.size())
                throw ParsingException("'on' or'off' needed after'autoindex'");
            if (tokens_[currentTokenIndex_] == "on")
                location.setCGIEnable(true);
            else if (tokens_[currentTokenIndex_] == "off")
                location.setCGIEnable(false);
            else
                throw ParsingException("Invalid value for 'cgi': " + tokens_[currentTokenIndex_]);
            ++currentTokenIndex_;
            if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != ";")
                throw ParsingException("';' needed after 'cgi'");
            ++currentTokenIndex_;
        }

        else if (token == "limit_except")
        {
            ++currentTokenIndex_;
            std::vector<std::string> methods;
            while (currentTokenIndex_ < tokens_.size() && tokens_[currentTokenIndex_] != ";")
            {
                methods.push_back(tokens_[currentTokenIndex_]);
                ++currentTokenIndex_;
            }
            location.setAllowedMethods(methods);
            if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != ";")
                throw ParsingException("';' needed after 'cgi'");
            ++currentTokenIndex_;
        }
        else if (token == "return")
        {
            std::string returnValue;
            parseSimpleDirective("return", returnValue);
            location.setRedirection(returnValue);
        }
        else if (token == "cgi_pass")
        {
            std::string cgiValue;
            parseSimpleDirective("cgi_pass", cgiValue);
            location.setCgiExtension(cgiValue);
        }
        else if (token == "upload_enable")
        {
            ++currentTokenIndex_;
            if (currentTokenIndex_ >= tokens_.size())
                throw ParsingException("'on' or'off' needed after 'upload_enable'");
            if (tokens_[currentTokenIndex_] == "on")
                location.setUploadEnable(true);
            else if (tokens_[currentTokenIndex_] == "off")
                location.setUploadEnable(false);
            else
                throw ParsingException("Invalid value for 'upload_enable': " + tokens_[currentTokenIndex_]);
            ++currentTokenIndex_;
            if (currentTokenIndex_ >= tokens_.size() || tokens_[currentTokenIndex_] != ";")
                throw ParsingException("';' needed after 'upload_enable'");
            ++currentTokenIndex_;
        }
        else if (token == "upload_store")
        {
            std::string uploadStoreValue;
            parseSimpleDirective("upload_store", uploadStoreValue);
            location.setUploadStore(uploadStoreValue);
        }
        else
        {
            throw ParsingException("Unknown directive in the context 'location': " + token);
        }
    }

    server.addLocation(location);
}

void ConfigParser::checkConfigValidity() const
{
    if(!config_)
        throw (ParsingException("An error occured while charging configuration file"));
    const std::vector<Server*> servers = config_->getServers();
    if(servers.empty())
        throw (ParsingException("An error occured while charging configuration file : no server has been set"));
    for(size_t i = 0; i < servers.size(); i++)
    {
        if(servers[i]->getRoot() == "")
            throw (ParsingException("An error occured while charging configuration file :\nat least one server have no root directory"));
        if(servers[i]->getPort() == 0 )
            throw (ParsingException("An error occured while charging configuration file :\nat least one server don't have a valid IP:PORT to listen"));
    }
}

//DEBUG METHOD
void ConfigParser::displayParsingResult()
{
    if (config_ == NULL)
    {
        std::cerr << "Error in the parsing of the .conf file" << std::endl;
        return;
    }

    config_->displayConfig();
}
