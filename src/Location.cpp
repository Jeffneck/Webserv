#include "../includes/Location.hpp"
#include "../includes/Server.hpp"
#include <iostream>

Location::Location(const Server &server, const std::string &path)
    : server_(server),                 
      clientMaxBodySizeIsSet_(false), 
      rootIsSet_(false),               
      indexIsSet_(false),              
      clientMaxBodySize_(0),          
      root_(""),                      
      index_(""),                     
      errorPages_(),                 
      path_(path),                   
      allowedMethods_(),             
      redirection_(""),               
      autoIndex_(false),             
      cgiEnable_(false),             
      cgiExtension_(""),             
      uploadEnable_(false),            
      uploadStore_("")                 
{
}

Location::~Location()
{
}

void Location::setPath(const std::string &path)
{
    path_ = path;
}

const std::string &Location::getPath() const
{
    return path_;
}

void Location::setAllowedMethods(const std::vector<std::string> &methods)
{
    allowedMethods_ = methods;
}

const std::vector<std::string> &Location::getAllowedMethods() const
{
    return allowedMethods_;
}

void Location::setRedirection(const std::string &redirection)
{
    redirection_ = redirection;
}

const std::string &Location::getRedirection() const
{
    return redirection_;
}

void Location::setRoot(const std::string &root)
{
    root_ = root;
    rootIsSet_ = true;
}

const std::string &Location::getRoot() const
{
    if (rootIsSet_)
        return root_;
    else
        return server_.getRoot();
}

void Location::setAutoIndex(bool autoIndex)
{
    autoIndex_ = autoIndex;
}

bool Location::getAutoIndex() const
{
    return autoIndex_;
}

void Location::setCGIEnable(bool cgiEnable)
{
    cgiEnable_ = cgiEnable;
}

bool Location::getCGIEnable() const
{
    return cgiEnable_;
}
void Location::setIndex(const std::string &index)
{
    index_ = index;
    indexIsSet_ = true;
}

const std::string &Location::getIndex() const
{
    if (indexIsSet_)
        return index_;
    else
        return server_.getIndex();
}

void Location::setCgiExtension(const std::string &extension)
{
    cgiExtension_ = extension;
}

const std::string &Location::getCgiExtension() const
{
    return cgiExtension_;
}

void Location::setUploadEnable(bool enable)
{
    uploadEnable_ = enable;
}

bool Location::getUploadEnable() const
{
    return uploadEnable_;
}

void Location::setUploadStore(const std::string &uploadStore)
{
    uploadStore_ = uploadStore;
}

const std::string &Location::getUploadStore() const
{
    return uploadStore_;
}

void Location::setClientMaxBodySize(size_t size)
{
    clientMaxBodySize_ = size;
    clientMaxBodySizeIsSet_ = true;
}

size_t Location::getClientMaxBodySize() const
{
    if (clientMaxBodySizeIsSet_)
        return clientMaxBodySize_;
    else
        return server_.getClientMaxBodySize();
}

void Location::addErrorPage(int statusCode, const std::string &uri)
{
    errorPages_[statusCode] = uri;
}

const std::map<int, std::string> &Location::getErrorPages() const
{
    if (!errorPages_.empty())
        return errorPages_;
    else
        return server_.getErrorPages();
}

const std::string Location::getErrorPage(int errorCode) const
{
    std::map<int, std::string>::const_iterator it = errorPages_.find(errorCode);
    if (it != errorPages_.end())
        return it->second;
    else
        return server_.getErrorPage(errorCode);
}

const std::string Location::getErrorPageFullPath(int errorCode) const
{
    // std::cout << "Location::getErrorPageFullPath" << std::endl;//debug
    return(getRoot() + getErrorPage(errorCode));
}


bool Location::getRootIsSet() const
{
    return(rootIsSet_);
}

bool Location::getIndexIsSet() const
{
    return(indexIsSet_);
}

bool Location::getClientMaxBodySizeIsSet() const
{
    return(clientMaxBodySizeIsSet_);
}

// DEBUG
void Location::displayLocation() const
{
    std::cout << "  Location " << this->getPath() << ":" << std::endl;
    std::cout << "    root: " << this->getRoot() << std::endl;
    std::cout << "    index: " << this->getIndex() << std::endl;
    std::cout << "    autoindex: " << (this->getAutoIndex() ? "on" : "off") << std::endl;
    std::cout << "    cgi enabled: " << (this->getCGIEnable() ? "on" : "off") << std::endl;

    // Afficher les méthodes autorisées
    const std::vector<std::string> &methods = this->getAllowedMethods();
    if (!methods.empty())
    {
        std::cout << "    allowed_methods:";
        for (size_t m = 0; m < methods.size(); ++m)
        {
            std::cout << " " << methods[m];
        }
        std::cout << std::endl;
    }

    // Afficher d'autres informations si nécessaire
    if (!this->getRedirection().empty())
    {
        std::cout << "    redirection: " << this->getRedirection() << std::endl;
    }

    if (!this->getCgiExtension().empty())
    {
        std::cout << "    cgi_pass: " << this->getCgiExtension() << std::endl;
    }

    std::cout << "    upload_enable: " << (this->getUploadEnable() ? "on" : "off") << std::endl;

    if (!this->getUploadStore().empty())
    {
        std::cout << "    upload_store: " << this->getUploadStore() << std::endl;
    }

    std::cout << "    client_max_body_size: " << this->getClientMaxBodySize() << std::endl;

    // Affichage des pages d'erreur de la location
    const std::map<int, std::string> &locationErrorPages = this->getErrorPages();
    for (std::map<int, std::string>::const_iterator it = locationErrorPages.begin(); it != locationErrorPages.end(); ++it)
    {
        std::cout << "    error_page " << it->first << " : " << it->second << std::endl;
    }
}
