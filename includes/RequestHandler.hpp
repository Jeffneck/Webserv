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
    int statusCode;  // HTTP status code
    HttpException(int code, const std::string& message)
        : std::runtime_error(message), statusCode(code) {}
};


/**
 * @class RequestHandler
 * 
 * The `RequestHandler` class is responsible for processing HTTP requests and generating appropriate HTTP 
 * responses. It handles the entire lifecycle of an HTTP request, including selecting the appropriate server 
 * and location, processing the request (whether it involves serving a static file, handling file uploads, 
 * or executing a CGI process), and returning the final response.
 * 
 * - **Request Processing**: The class processes incoming HTTP requests by selecting the correct server 
 *   and location, handling different types of requests, and generating the corresponding response.
 * 
 * - **Static File Handling**: It manages the serving of static files by generating the full file path, 
 *   verifying the file's security, and ensuring the correct MIME type is set for the response.
 * 
 * - **CGI Process Management**: The class is capable of handling dynamic content via CGI by setting up 
 *   the necessary environment variables, creating the appropriate parameters, and starting the CGI process.
 * 
 * - **File Upload and Deletion**: It handles HTTP POST requests for file uploads and DELETE requests for 
 *   file deletions, managing file locations and ensuring appropriate permissions and size limits.
 * 
 * - **Error Handling**: It includes methods for managing errors and returning appropriate HTTP error codes 
 *   along with custom error pages when needed.
 * 
 * This class is central to the web server's ability to interpret and respond to HTTP requests, whether 
 * the request is for static content, dynamic content via CGI, or file operations.
 */
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
