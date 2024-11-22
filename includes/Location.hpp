#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <vector>
#include <map>

class Server; // Forward declaration


/**
 * @class Location
 * 
 * The `Location` class represents a specific location or directory block within a server's configuration. 
 * It is used to define how requests for a specific path should be handled. This class allows you to set various 
 * parameters related to the handling of requests for that path, such as allowed methods, redirections, 
 * CGI configuration, file upload settings, and error pages.
 * 
 * - **Path Management**: The class allows setting and getting the path that the location block corresponds to.
 * 
 * - **Method Handling**: It enables the configuration of allowed HTTP methods (e.g., GET, POST) for requests 
 *   to that location.
 * 
 * - **Redirection and Indexing**: The class supports setting up redirections and defining the index file 
 *   for the location. It also allows enabling or disabling automatic directory indexing.
 * 
 * - **CGI and Uploads**: The location can be configured to handle CGI requests and file uploads with custom 
 *   settings for the upload directory and maximum body size.
 * 
 * - **Error Pages**: The class allows defining custom error pages for specific HTTP status codes, allowing 
 *   different error messages or pages to be displayed for different types of errors.
 * 
 * This class is an important component in the server configuration, enabling fine-grained control over how 
 * different paths or locations are handled within the server, including the types of requests that can be 
 * processed, how errors are managed, and how files are served or uploaded.
 */

class Location
{
public:
    Location(const Server &server, const std::string &path);
    ~Location();

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
    // Reference to the parent server
    const Server &server_; 

    // Indicators of whether values are defined locally
    bool clientMaxBodySizeIsSet_;
    bool rootIsSet_;
    bool indexIsSet_;

    // Inheritable directives
    size_t clientMaxBodySize_;
    std::string root_;
    std::string index_;
    std::map<int, std::string> errorPages_;

    // Specific directives (=that can be only found in location context)
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
