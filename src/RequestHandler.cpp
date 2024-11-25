// RequestHandler.cpp
#include "RequestHandler.hpp"
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits.h>
#include "../includes/Utils.hpp"
#include "../includes/Error.hpp"
#include "../includes/Color_Macros.hpp"
#include "../includes/Utils.hpp"
#include <cerrno>
#include <string.h>


RequestHandler::RequestHandler(const Config& config, const std::vector<Server*>& associatedServers)
    : config_(config), associatedServers_(associatedServers)
{
}

RequestHandler::~RequestHandler() {}

RequestResult RequestHandler::handleRequest(const HttpRequest& request) {
    RequestResult result;
    const Server* server = selectServer(request);

    // Vérifier si le serveur est NULL
    if (!server) {
        result.response = handleError(400, config_.getErrorPageFullPath(400));
        result.responseReady = true;
        return result;
    }

    const Location* location = selectLocation(server, request);

    process(server, location, request, result);
    return result;
}

const Server* RequestHandler::selectServer(const HttpRequest& request) const {
    std::string hostHeader = request.getHeader("host");
    if (hostHeader.empty()) {
        std::cerr << "No Host header found in the request." << std::endl;
        return NULL; // Error managed after
    }

    // find the good server in associatedServers_
    for (size_t i = 0; i < associatedServers_.size(); ++i) {
        const std::vector<std::string>& serverNames = associatedServers_[i]->getServerNames();
        if (std::find(serverNames.begin(), serverNames.end(), hostHeader) != serverNames.end()) {
            return associatedServers_[i];
        }
    }

    // first server is default
    if (!associatedServers_.empty()) {
        return associatedServers_[0];
    }

    // No server in .conf
    return NULL;// Error managed after
}

const Location* RequestHandler::selectLocation(const Server* server, const HttpRequest& request) const {
    if (!server) {
        return NULL;
    }
    std::string requestPath = request.getPath();
    const std::vector<Location>& locations = server->getLocations();

    const Location* matchedLocation = NULL;
    size_t longestMatch = 0;

    for (size_t i = 0; i < locations.size(); ++i) {
        std::string locPath = locations[i].getPath();
        if (requestPath.find(locPath) == 0 && locPath.length() > longestMatch) {
            matchedLocation = &locations[i];
            longestMatch = locPath.length();
        }
    }

    return matchedLocation; // NULL is not an error here, because it is possible that there is no specific rules for this location
}

/**
 * @brief Processes the HTTP request and generates an appropriate HTTP response.
 * 
 * This function is responsible for handling various types of HTTP requests. It processes the request by first 
 * selecting the appropriate server and location, validating the HTTP method, checking request headers (such as 
 * Content-Length), and then deciding whether the request should be handled by serving static files, processing 
 * CGI scripts, handling file uploads, or file deletions. It also handles redirections and validates the security 
 * of file paths.
 * 
 * - **Server and Location Selection**: The function first verifies that the correct server and location are 
 *   selected based on the request. If a server or location is not found, an error is returned.
 * 
 * - **Method Validation**: It checks if the HTTP method (GET, POST, DELETE, etc.) is allowed for the current 
 *   location. If the method is not allowed, it responds with a `405 Method Not Allowed` error.
 * 
 * - **Content-Length Validation**: The function checks the `Content-Length` header to ensure it does not exceed 
 *   the maximum allowed size for the server or location. If the length is invalid or exceeds the limit, it responds 
 *   with a `400 Bad Request` or `413 Payload Too Large` error.
 * 
 * - **Redirection Handling**: If a redirection is configured for the location, the request is redirected to the 
 *   specified URL with a `302 Found` status code.
 * 
 * - **CGI Process Handling**: If the request should be handled by a CGI script, the function verifies the file's 
 *   existence and permissions, starts the CGI process, and handles the interaction with the process.
 * 
 * - **Static File Handling**: If the request is for a static file (typically a GET request), the file is served 
 *   to the client.
 * 
 * - **File Upload and Deletion**: If the request is a POST for file upload or DELETE for file deletion, the 
 *   corresponding handler is called to manage the file operation.
 * 
 * - **Error Handling**: If any error occurs during processing (e.g., invalid method, missing file, or invalid request), 
 *   the function generates an appropriate error response with an error page.
 * 
 * This function ensures that the web server can handle a variety of HTTP request types, returning the correct response 
 * based on the configuration and request specifics.
 */

void RequestHandler::process(const Server* server, const Location* location, const HttpRequest& request, RequestResult& result) const {
    // Error if Server has not been found
    if (!server) {
        result.response = handleError(400, config_.getErrorPageFullPath(400));
        result.responseReady = true;
        return;
    }

    // Extract allowed method in the current context (location > Server)
    std::vector<std::string> allowedMethods;
    if (location && !location->getAllowedMethods().empty()) {
        // std::cout << "Using allowed methods found in location" << std::endl;
        allowedMethods = location->getAllowedMethods();
        //DENY in .conf stands for : No method allowed here
        if(allowedMethods[0] == "DENY"){
            result.response = handleError(405, getErrorPageFullPath(405, location, server));
            result.responseReady = true;
            return;
        }
    } else {
        // std::cout << "Using default allowed methods GET POST DELETE" << std::endl;
        allowedMethods.push_back("GET");
        allowedMethods.push_back("POST");
        allowedMethods.push_back("DELETE");
    }

    // Verify if the extracted Method is allowed
    std::vector<std::string>::iterator it = std::find(allowedMethods.begin(), allowedMethods.end(), request.getMethod());
    if (it == allowedMethods.end()) {
        result.response = handleError(405, getErrorPageFullPath(405, location, server));
        result.response.setHeader("Allow", join(allowedMethods, ", "));
        result.responseReady = true;
        return;
    }

    //  Check that Content-Length is not greater than client_max_body_size
    std::string contentLengthStr = request.getHeader("content-length");
    if (!contentLengthStr.empty()) {
        // Content-Length str to size_t
        char* endptr;
        errno = 0;
        unsigned long contentLength = strtoul(contentLengthStr.c_str(), &endptr, 10);
        if (*endptr != '\0' || errno != 0) {
            // Invalid Content Length
            result.response = handleError(400, getErrorPageFullPath(400, location, server));
            result.responseReady = true;
            return;
        }

        // Extract client max body size in the current context (location > Server)
        size_t clientMaxBodySize = 0;
        if (location && location->getClientMaxBodySize() > 0) {
            clientMaxBodySize = location->getClientMaxBodySize();
        } else if (server->getClientMaxBodySize() > 0) {
            clientMaxBodySize = server->getClientMaxBodySize();
        } else {
            // Default value
            clientMaxBodySize = 0; // 0 stands for 'no limit'
        }

        if (clientMaxBodySize > 0 && contentLength > clientMaxBodySize) {
            // std::cout << YELLOW << "client max body size "<<clientMaxBodySize << " vs content length " << contentLength << RESET << std::endl;//test
            result.response = handleError(413, getErrorPageFullPath(413, location, server));
            result.responseReady = true;
            return;
        }
    }

    // Redirection
    if (location && !location->getRedirection().empty()) {
        std::string redirectionUrl = location->getRedirection();
        result.response.setStatusCode(302);
        result.response.setHeader("Location", redirectionUrl);
        result.response.setBody("Redirecting to " + redirectionUrl);
        result.responseReady = true;
        return;
    }

    // Handle CGI
    if (location && !location->getCgiExtension().empty() && location->getCGIEnable() && endsWith(request.getPath(), location->getCgiExtension())) {
        try {
            //verify if the file is existent and can be given to the cgi
            std::string fileFullPath = getFileFullPath(server, location, request); 
            verifyFile(fileFullPath, true);

            CgiProcess* cgiProcess = startCgiProcess(server, location, request);
            result.cgiProcess = cgiProcess;
            result.responseReady = false;
            return;
        } catch (const HttpException& e) {
            result.response = handleError(e.statusCode, getErrorPageFullPath(e.statusCode, location, server));
            result.responseReady = true;
            return;
        }
    }

    // Handle static files
    if (request.getMethod() == "GET") {
        result.response = serveStaticFile(server, location, request);
        result.responseReady = true;
        return;
    }

    // Handle file upload
    if (request.getMethod() == "POST" && location && location->getUploadEnable()) {
        result.response = handleFileUpload(request, location, server);
        result.responseReady = true;
        return;
    }

    // Handle Deleting files
    if (request.getMethod() == "DELETE" && location) {
        result.response = handleDeletion(request, location, server);
        result.responseReady = true;
        return;
    }

    // Method not allowed if we're here
    result.response = handleError(405, getErrorPageFullPath(405, location, server));
    result.response.setHeader("Allow", join(allowedMethods, ", "));
    result.responseReady = true;
}

/**
 * @brief Verifies the existence, accessibility, and validity of a file.
 * 
 * This function checks if the specified file exists and whether it is accessible.
 * It throws an HttpException if the file cannot be accessed or if it is not a regular file.
 * Optionally, it tries to open the file if `tryOpen` is true, throwing an exception if the file cannot be opened.
 */
void RequestHandler::verifyFile(const std::string& fullPath, const bool tryOpen) const {

    // Check file existence and accessibility
    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) != 0) {
        if (errno == EACCES) {
            throw HttpException(403, "Forbidden: File access denied");
        } else if (errno == ENOENT || errno == ENOTDIR) {
            throw HttpException(404, "Not Found: File not found or not a directory");
        } else {
            throw HttpException(500, "Internal Server Error: Unable to access file");
        }
    }

    // Check that it's a regular file
    if (!S_ISREG(fileStat.st_mode)) {
        throw HttpException(403, "Forbidden: Not a regular file");
    }

    // check that it is possible to open the file
    if(tryOpen == true)
    {
        std::ifstream file(fullPath.c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            if (errno == EACCES) {
                throw HttpException(403, "Forbidden: Unable to open file");
            } else {
                throw HttpException(404, "Not Found: File could not be opened");
            }
        }
        file.close();
    }
}

/**
 * @brief Starts a CGI process to handle a request.
 * 
 * This function initializes the necessary environment for running a CGI script, including setting up
 * the working directory, constructing the argument and environment variable lists, and starting the CGI process.
 * It throws an HttpException if any step in the process fails.
 */
CgiProcess* RequestHandler::startCgiProcess(const Server* server, const Location* location, const HttpRequest& request) const {
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        throw HttpException(500, "Internal Server Error: Unable to get current working directory");
    }

    // build scriptWorkingDir path (directory where the script is gonna be exec) = often /cgi-bin/
    std::string scriptWorkingDir = cwd;
    scriptWorkingDir += "/" ;
    scriptWorkingDir += server->getRoot();
    if (location) {
        scriptWorkingDir += "/";
        scriptWorkingDir += location->getPath();
        scriptWorkingDir += "/";
    }

    // build relativeFilePath path (file that is gonna be exec)
    std::string relativeFilePath = request.getPath();
    if (location && relativeFilePath.compare(0, location->getPath().length(), location->getPath()) == 0) {
        relativeFilePath.erase(0, location->getPath().length());
    }
    relativeFilePath = "./" + relativeFilePath;

    // Extract parameters to give to the script (different methods for GET and POST)
    std::map<std::string, std::string> params;
    if (request.getMethod() == "GET") {
        // Params are in the query string for GET
        params = createScriptParamsGET(request.getQueryString());
    } else if (request.getMethod() == "POST") {
        std::string contentType = request.getHeader("content-type");
        if (contentType == "application/x-www-form-urlencoded") {
            // Params are in the body for POST
            params = createScriptParamsPOST(request.getBody());
        } 
        else if (contentType == "plain/text")
        {
            //do nothing, body will be transmitted via envVars
        }
        else {
            // Content is not supported
            throw HttpException(415, "Unsupported Media Type: " + contentType);
        }
    } else {
        // Method is not supported
        throw HttpException(405, "Method Not Allowed: " + request.getMethod());
    }

    std::vector<std::string> envVars;
    setupScriptEnvp(request, relativeFilePath, envVars);

    CgiProcess* cgiProcess = new CgiProcess(scriptWorkingDir, relativeFilePath, params, envVars);
    if (!cgiProcess->start()) {
        delete cgiProcess;
        throw HttpException(500, "Internal Server Error: Failed to start CGI process");
    }
    return cgiProcess;
}

/**
 * @brief Sets up environment variables for the CGI script.
 * 
 * This function populates the environment variable list (`envVars`) required to run a CGI script.
 * It adds variables such as `REQUEST_METHOD`, `CONTENT_TYPE`, `CONTENT_LENGTH`, and the script's filename.
 */
void RequestHandler::setupScriptEnvp(const HttpRequest& request, const std::string& relativeFilePath,  std::vector<std::string>& envVars) const{
    envVars.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
    envVars.push_back("REQUEST_METHOD=" + request.getMethod());
    envVars.push_back("SCRIPT_FILENAME=" + relativeFilePath);
    envVars.push_back("CONTENT_TYPE=" + request.getHeader("content-Type"));
    envVars.push_back("CONTENT_LENGTH=" + request.getHeader("content-Length"));
    envVars.push_back("REQUEST_BODY=" + request.getBody());
    envVars.push_back("QUERY_STRING=" + request.getQueryString());
}

/**
 * @brief Creates a map of parameters from the query string of a GET request.
 * 
 * This function parses the query string from a GET request, extracting key-value pairs and storing them in a map.
 * It handles both the case where a parameter has a value and where it does not.
 */
std::map<std::string, std::string> RequestHandler::createScriptParamsGET(const std::string& queryString) const {
    std::map<std::string, std::string> params;
    std::string::size_type last_pos = 0, amp_pos;

    while ((amp_pos = queryString.find('&', last_pos)) != std::string::npos) {
        std::string key_value_pair = queryString.substr(last_pos, amp_pos - last_pos);
        std::string::size_type eq_pos = key_value_pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = key_value_pair.substr(0, eq_pos);
            std::string value = key_value_pair.substr(eq_pos + 1);
            params[key] = value;
        } else if (!key_value_pair.empty()) {
            // If there's no '=', treat the entire string as a key with an empty value
            params[key_value_pair] = "";
        }
        last_pos = amp_pos + 1;
    }

    // Handle the last parameter (or only parameter if no '&' was found)
    std::string key_value_pair = queryString.substr(last_pos);
    if (!key_value_pair.empty()) {
        std::string::size_type eq_pos = key_value_pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = key_value_pair.substr(0, eq_pos);
            std::string value = key_value_pair.substr(eq_pos + 1);
            params[key] = value;
        } else {
            // If there's no '=', treat the entire string as a key with an empty value
            params[key_value_pair] = "";
        }
    }

    return params;
}

/**
 * @brief Creates a map of parameters from the body of a POST request.
 * 
 * This function parses the body of a POST request, extracting key-value pairs and storing them in a map.
 * It assumes the body is formatted as `key=value&key=value`.
 */
std::map<std::string, std::string> RequestHandler::createScriptParamsPOST(const std::string& postBody) const {
    std::map<std::string, std::string> params;
    std::string::size_type last_pos = 0, amp_pos;

    while ((amp_pos = postBody.find('&', last_pos)) != std::string::npos) {
        std::string key_value_pair = postBody.substr(last_pos, amp_pos - last_pos);
        std::string::size_type eq_pos = key_value_pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = key_value_pair.substr(0, eq_pos);
            std::string value = key_value_pair.substr(eq_pos + 1);
            params[key] = value;
        } else if (!key_value_pair.empty()) {
            params[key_value_pair] = "";
        }
        last_pos = amp_pos + 1;
    }

    // Treat last parameter
    std::string key_value_pair = postBody.substr(last_pos);
    if (!key_value_pair.empty()) {
        std::string::size_type eq_pos = key_value_pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = key_value_pair.substr(0, eq_pos);
            std::string value = key_value_pair.substr(eq_pos + 1);
            params[key] = value;
        } else {
            params[key_value_pair] = "";
        }
    }

    return params;
}


/**
 * @brief Returns the full path to the requested file.
 * 
 * This function constructs the absolute file path based on the server's root and the location's root, 
 * adjusting for any specified path and query parameters. It returns the resulting path.
 */
std::string RequestHandler::getFileFullPath(const Server* server, const Location* location, const HttpRequest& request) const {

    std::string root = server->getRoot();

    if (location) {
        root = location->getRoot();
    }

    std::string requestPath = request.getPath();

    // if empty, make it root by adding /
    if (requestPath.empty()) {
        requestPath = "/";
    }

    // Delete location path if root is defined in location
    if (location && location->getRootIsSet()) {
        std::string to_remove = location->getPath();
        size_t pos = requestPath.find(to_remove);
        if (pos != std::string::npos) {
            requestPath.erase(pos, to_remove.length());
        }
    }
    return (root + requestPath);
}


/**
 * @brief Serves a static file to the client.
 * 
 * This function attempts to serve a static file to the client by reading the file and sending its content in the response.
 * It handles the cases where the path is a directory, and it can generate an auto-index if enabled. If any error occurs,
 * it responds with an appropriate error code.
 */
HttpResponse RequestHandler::serveStaticFile(const Server* server, const Location* location, const HttpRequest& request) const {
    HttpResponse response;
    std::string fileFullPath = getFileFullPath(server, location, request);

    // Handle the case where the path ends with a '/'
    if (fileFullPath[fileFullPath.size() - 1] == '/') {
        // if index is not defined and auto-index is enabled = Generate auto-index 
        if (location && !location->getIndexIsSet() && location->getAutoIndex()) {
            return generateAutoIndex(fileFullPath, request.getPath());
        } 
        // if index is defined = Serve Index file 
        else if(location && location->getIndexIsSet()){
            fileFullPath += location->getIndex();
        } else{
            return handleError(403, getErrorPageFullPath(403, location, server)); // Forbidden
        }
    }

    //verify the file
    try{ 
        verifyFile(fileFullPath, false);
    } catch (const HttpException& e) {
        return handleError(e.statusCode, getErrorPageFullPath(e.statusCode, location, server)); // Forbidde
    }

    // Open the file 
    std::ifstream file(fileFullPath.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        if (errno == EACCES) {
            return handleError(403, getErrorPageFullPath(403, location, server)); // Forbidden
        } else {
            return handleError(404, getErrorPageFullPath(404, location, server)); // Not Found
        }
    }

    // Read fle content
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContent = buffer.str();
    file.close();

    // Define response headers and body
    response.setStatusCode(200);
    response.setBody(fileContent);

    // Define Content-Type according to file extension
    size_t dotPos = fileFullPath.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string extension = fileFullPath.substr(dotPos + 1);
        std::string contentType = getMimeType(extension);
        if (!contentType.empty()) {
            response.setHeader("Content-Type", contentType);
            response.setHeader("Connection", "close");
        }
    }

    return response;
}



/*
 * This function handles file uploads from HTTP requests with the "multipart/form-data" content type.
 * 
 * Steps:
 * 1. It first checks if the Content-Type of the request is "multipart/form-data".
 * 2. It extracts the boundary parameter from the Content-Type header, which is used to separate different parts of the request body.
 * 3. It reads the body of the request and checks if the upload directory exists.
 * 4. The body of the request is then parsed to separate each part. Each part contains headers and the actual file data.
 * 5. For each part, it looks for the "filename" field in the headers to identify if it's a file part.
 * 6. If a file is found, it saves the file to the specified upload directory.
 * 7. If any errors occur (such as missing Content-Type, boundary, or upload directory issues), an error response is returned.
 * 8. On successful upload, a 201 status code is returned along with a success message.
 */
HttpResponse RequestHandler::handleFileUpload(const HttpRequest& request, const Location* location, const Server* server) const {
    // std::cout << RED << "RequestHandler::handleFileUpload" << RESET << std::endl; //Debug 
    HttpResponse response;

    // Check that the Content-Type is multipart/form-data
    std::string contentType = request.getHeader("content-type");
    if (contentType.find("multipart/form-data") != 0 ) {
        response = handleError(400, getErrorPageFullPath(400, location, server));
        return response;
    }

    // Extract boundary from Content-Type header
    std::string boundaryPrefix = "boundary=";
    std::string::size_type boundaryPos = contentType.find(boundaryPrefix);
    if (boundaryPos == std::string::npos) {
        // No boundary found
        response = handleError(400, getErrorPageFullPath(400, location, server));
        return response;
    }
    boundaryPos += boundaryPrefix.length();
    std::string boundary = "--" + contentType.substr(boundaryPos);

    // Read the body of the request
    std::string body = request.getBody();

    // Check that the upload directory exists
    std::string uploadDirectory = location->getUploadStore();
    struct stat dirStat;
    if (stat(uploadDirectory.c_str(), &dirStat) != 0 || !S_ISDIR(dirStat.st_mode)) {
        std::cerr << "Upload directory does not exist: " << uploadDirectory << std::endl;
        response = handleError(500, getErrorPageFullPath(500, location, server));
        return response;
    }

    // Separate the different parts
    std::string::size_type start = 0;
    while ((start = body.find(boundary, start)) != std::string::npos) {
        start += boundary.length();
        //end = next boundary found (if not found, there is no more content to read)
        std::string::size_type end = body.find(boundary, start);
        if (end == std::string::npos) break;

        std::string part = body.substr(start, end - start);

        // Extract part headers (each part separated by a boudary have headers)
        std::string::size_type headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos) continue;

        std::string headers = part.substr(0, headerEnd);
        std::string fileData = part.substr(headerEnd + 4);

        // Check if this part is a file
        std::string filenamePrefix = "filename=\"";
        std::string::size_type filenamePos = headers.find(filenamePrefix);
        if (filenamePos != std::string::npos) {
            std::string::size_type filenameEndPos = headers.find("\"", filenamePos + filenamePrefix.length());
            if (filenameEndPos != std::string::npos) {
                std::string filename = headers.substr(filenamePos + filenamePrefix.length(), filenameEndPos - (filenamePos + filenamePrefix.length()));

                // Build the complete path to save the file
                std::string fullPath = uploadDirectory + "/" + filename;

                // Save the file
                std::ofstream file(fullPath.c_str(), std::ios::binary);
                if (file.is_open()) {
                    file.write(fileData.c_str(), fileData.size());
                    file.close();
                    // std::cout << "Info : File saved: " << fullPath << std::endl;//debug
                } else {
                    std::cerr << "Error : Failed to save file: " << fullPath << std::endl;
                    response = handleError(500, getErrorPageFullPath(500, location, server));
                    return response;
                }
            }
        }
    }

    response.setStatusCode(201);
    response.setHeader("Connection", "close");
    response.setBody("File upload successful.\n");
    return response;
}


/**
 * @brief Handles file deletion requests.
 * 
 * This function processes a DELETE request for a file, checking the validity and permissions of the requested file.
 * It ensures the file exists, is accessible, and is a regular file. If any checks fail, an appropriate HTTP error response
 * is returned. If the file can be deleted, it attempts the deletion and returns a success response with status 204.
 */
HttpResponse RequestHandler::handleDeletion(const HttpRequest& request, const Location* location, const Server* server) const {
    // std::cout << RED << "RequestHandler::handleDeletion" << RESET << std::endl; // Debug

    HttpResponse response;

    // Get root directory
    std::string root = server->getRoot();
    if (location) {
        root = location->getRoot();
    }

    // Build complete path to file to delete
    std::string requestPath = request.getPath();

    // Manage case wher '/' is at the end (file to delete is a directory = Error)
    if (!requestPath.empty() && requestPath[requestPath.size() - 1] == '/') {
        response = handleError(400, getErrorPageFullPath(400, location, server));
        return response;
    }

    // Remove location path from requestPath if root is defined in the location
    if (location && location->getRootIsSet()) {
        std::string to_remove = location->getPath();
        if (requestPath.find(to_remove) == 0) {
            requestPath.erase(0, to_remove.length());
        }
    }

    // Build the absolute path to the file
    std::string fullPath = root + requestPath;
    // Replace special chars written in the path like : %20 for space
    decodeURI(fullPath);
    

    // Verify if path is secure
    if (!isPathSecure(root, fullPath)) {
        response = handleError(403, getErrorPageFullPath(403, location, server)); // Forbidden
        return response;
    }

    // Verify if file exist
    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) != 0) {
        if (errno == ENOENT) {
            response = handleError(404, getErrorPageFullPath(404, location, server)); // Not Found
            return response;
        } else {
            std::cerr << "Error Deletion :  unaccessible file: " << strerror(errno) << std::endl;
            response = handleError(500, getErrorPageFullPath(500, location, server)); // Internal Server Error
            return response;
        }
    }

    // Verify if it is a regular file
    if (!S_ISREG(fileStat.st_mode)) {
        std::cerr << "Error Deletion : Target is not a regular file." << std::endl;
        response = handleError(403, getErrorPageFullPath(403, location, server)); // Forbidden
        return response;
    }

    // Verify if we are allowed to delete this file
    if (access(fullPath.c_str(), W_OK) != 0) {
        std::cerr << "Error Deletion : No permission to delete the file: " << strerror(errno) << std::endl;
        response = handleError(403, getErrorPageFullPath(403, location, server)); // Forbidden
        return response;
    }

    // Try to delete the file
    if (unlink(fullPath.c_str()) != 0) {
        std::cerr << "Error Deletion : Failed to delete file: " << strerror(errno) << std::endl;
        response = handleError(500, getErrorPageFullPath(500, location, server)); // Internal Server Error
        return response;
    }

    // std::cout << "Info: file deleted: " << fullPath << std::endl; // Debug
    // build success response
    response.setStatusCode(204);
    response.setHeader("Content-Type", "text/plain; charset=UTF-8");
    response.setHeader("Connection", "close");

    return response;
}



/**
 * @brief Generates an auto-index HTML page for a directory.
 * 
 * If a request is made to a directory, this function generates an HTML page listing the contents of the directory,
 * including links to files and subdirectories. It handles cases where the directory cannot be read and returns a 
 * generic error message if necessary.
 */
HttpResponse RequestHandler::generateAutoIndex(const std::string& fullPath, const std::string& requestPath) const {
    std::stringstream ss;

    
    ss << "<html><head><title>Index of " << requestPath << "</title></head><body>";
    ss << "<h1>Index of " << requestPath << "</h1><ul>";

    DIR* dir = opendir(fullPath.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string name = entry->d_name;
            if (name == ".")
                continue;
            ss << "<li><a href=\"" << name << "\">" << name << "</a></li>";
        }
        closedir(dir);
    } else {
        ss << "<li>Error reading directory.</li>";
    }

    ss << "</ul></body></html>";

    
    HttpResponse response;
    response.setStatusCode(200);
    response.setBody(ss.str());
    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    response.setHeader("Connection", "close");

    return response; 
}


/**
 * @brief Returns the MIME type based on file extension.
 * 
 * This function maps a file extension to its corresponding MIME type. It is used to determine the `Content-Type` 
 * header when serving files. If the extension is unknown, it defaults to "application/octet-stream".
 */
std::string RequestHandler::getMimeType(const std::string& extension) const {
    if (extension == "html" || extension == "htm")
        return "text/html";
    if (extension == "css")
        return "text/css";
    if (extension == "js")
        return "application/javascript";
    if (extension == "png")
        return "image/png";
    if (extension == "jpg" || extension == "jpeg")
        return "image/jpeg";
    if (extension == "gif")
        return "image/gif";
    if (extension == "txt")
        return "text/plain";
    return "application/octet-stream"; // Default type = navigators will upload the file served if this mime type is send
}


/**
 * @brief Checks if the file path is secure and does not lead to directory traversal.
 * 
 * This function verifies that the requested file path is within the root directory, preventing potential directory traversal 
 * attacks. It uses `realpath` to get the absolute paths and checks if the file path is contained within the root path.
 */
bool RequestHandler::isPathSecure(const std::string& root, const std::string& fullPath) const {
    char realRoot[PATH_MAX];
    char realFullPath[PATH_MAX];

    if (realpath(root.c_str(), realRoot) == NULL) {
        std::cerr << "Invalid root path: " << root << std::endl;
        return false;
    }
    realpath(fullPath.c_str(), realFullPath);

    std::string realRootStr(realRoot);
    std::string realFullPathStr(realFullPath);
    if (realFullPathStr.find(realRootStr) != 0) {
        std::cerr << "Path traversal attempt detected: " << fullPath << std::endl;
        return false;
    }

    return true;
}


/**
 * @brief Retrieves the full file path for the error page based on the status code.
 * 
 * This function looks up the full path to the error page for a specific HTTP status code. It checks for an error page 
 * defined in the location context first, then the server context, and finally the global config context if no specific 
 * error page is found.
 */
std::string RequestHandler::getErrorPageFullPath(int statusCode, const Location* location, const Server* server) const {
    if (location && !location->getErrorPageFullPath(statusCode).empty()) {
        return location->getErrorPageFullPath(statusCode);
    } else if (server && !server->getErrorPageFullPath(statusCode).empty()) {
        return server->getErrorPageFullPath(statusCode);
    } else {
        return config_.getErrorPageFullPath(statusCode);
    }
}

/**
 * @brief Joins a list of strings with a specified delimiter.
 * 
 * This utility function takes a list of strings and concatenates them into a single string, inserting the specified delimiter
 * between each element. It is typically used for constructing header values like the "Allow" header in error responses.
 */
std::string RequestHandler::join(const std::vector<std::string>& elements, const std::string& delimiter) const {
    std::ostringstream os;
    for (size_t i = 0; i < elements.size(); ++i) {
        os << elements[i];
        if (i < elements.size() - 1) {
            os << delimiter;
        }
    }
    return os.str();
}
