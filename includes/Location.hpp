#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <vector>
#include <map>

class Server; // Forward declaration

class Location
{
public:
    Location(const Server &server, const std::string &path);
    ~Location();

    // Méthodes pour accéder et modifier les directives de la location
    void setPath(const std::string &path);
    const std::string &getPath() const;

    void setAllowedMethods(const std::vector<std::string> &methods);
    const std::vector<std::string> &getAllowedMethods() const;

    void setRedirection(const std::string &redirection);
    const std::string &getRedirection() const;

    void setRoot(const std::string &root);
    const std::string &getRoot() const;

    void setAutoIndex(bool autoIndex);
    bool getAutoIndex() const;

    void setCGIEnable(bool autoIndex);
    bool getCGIEnable() const;

    void setIndex(const std::string &index);
    const std::string &getIndex() const;

    void setCgiExtension(const std::string &extension);
    const std::string &getCgiExtension() const;

    void setUploadEnable(bool enable);
    bool getUploadEnable() const;

    void setUploadStore(const std::string &uploadStore);
    const std::string &getUploadStore() const;

    void setClientMaxBodySize(size_t size);
    size_t getClientMaxBodySize() const;

    bool getRootIsSet() const;
    bool getIndexIsSet() const;
    bool getClientMaxBodySizeIsSet() const;

    // Méthodes pour gérer les pages d'erreur
    void addErrorPage(int statusCode, const std::string &uri);
    const std::map<int, std::string> &getErrorPages() const;
    const std::string getErrorPage(int errorCode) const;
    const std::string getErrorPageFullPath(int errorCode) const;

    // DEBUG
    void displayLocation() const;

private:
    const Server &server_; // Référence au Server parent

    // Indicateurs pour savoir si les valeurs sont définies localement
    bool clientMaxBodySizeIsSet_;
    bool rootIsSet_;
    bool indexIsSet_;

    // Directives pouvant être héritées
    size_t clientMaxBodySize_;
    std::string root_;
    std::string index_;
    std::map<int, std::string> errorPages_;

    // Directives spécifiques à la location
    std::string path_;
    std::vector<std::string> allowedMethods_;
    std::string redirection_;
    bool autoIndex_;
    bool cgiEnable_;
    std::string cgiExtension_;
    bool uploadEnable_;
    std::string uploadStore_;
};

#endif // LOCATION_HPP
