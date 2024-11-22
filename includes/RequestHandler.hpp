// RequestHandler.hpp
#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include "Config.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Server.hpp"
#include "CgiProcess.hpp"

struct RequestResult {
    bool responseReady;
    HttpResponse response;
    CgiProcess* cgiProcess;

    RequestResult() : responseReady(false), cgiProcess(NULL) {}
};

class HttpException : public std::runtime_error {
public:
    int statusCode;  // Code de statut HTTP

    // Constructeur de l'exception personnalisée
    HttpException(int code, const std::string& message)
        : std::runtime_error(message), statusCode(code) {}
};

class RequestHandler {
public:
    RequestHandler(const Config& config, const std::vector<Server*>& associatedServers);
    ~RequestHandler();

    RequestResult handleRequest(const HttpRequest& request);

private:
    const Server* selectServer(const HttpRequest& request) const;
    const Location* selectLocation(const Server* server, const HttpRequest& request) const;

    void process(const Server* server, const Location* location, const HttpRequest& request, RequestResult& result) const;

    HttpResponse serveStaticFile(const Server* server, const Location* location, const HttpRequest& request) const;
    HttpResponse handleFileUpload(const HttpRequest& request, const Location* location, const Server* server) const;
    HttpResponse handleDeletion(const HttpRequest& request, const Location* location, const Server* server) const;
    
    std::string getFileFullPath(const Server* server, const Location* location, const HttpRequest& request) const;
    void verifyFile(const std::string& fullPath, const bool tryOpen) const;
    bool isPathSecure(const std::string& root, const std::string& fullPath) const;


    HttpResponse generateAutoIndex(const std::string& fullPath, const std::string& requestPath) const;
    std::string getMimeType(const std::string& extension) const;
    // HttpResponse handleError(int statusCode, const Server* server) const;

    CgiProcess* startCgiProcess(const Server* server, const Location* location, const HttpRequest& request) const;
    void setupScriptEnvp(const HttpRequest& request, const std::string& relativeFilePath,  std::vector<std::string>& envVars) const;
    std::map<std::string, std::string> createScriptParamsGET(const std::string& queryString) const;
    std::map<std::string, std::string> createScriptParamsPOST(const std::string& postData) const;

    //err management
    std::string getErrorPageFullPath(int statusCode, const Location* location, const Server* server) const;
    std::string join(const std::vector<std::string>& elements, const std::string& delimiter) const;



    const Config& config_;
    const std::vector<Server*>& associatedServers_;
};

#endif // REQUESTHANDLER_HPP
