#include <fstream>
#include <sstream>
#include <iostream>
#include "Utils.hpp"
#include "Server.hpp"
#include "HttpResponse.hpp"
#include "Color_Macros.hpp"
#include <sys/stat.h> 

/**
 * Handles the creation of an error HTTP response with a specified status code and error page.
 * 
 * This function generates an HTTP response based on the given status code and the path to an error page.
 * If the error page path is provided and valid, the content of the error page is read and used in the response body.
 * If the error page path is invalid (file does not exist, insufficient permissions, or is a directory), a default error message is used.
 * If no error page path is provided, a default error message corresponding to the status code is used.
 * 
 * The function also sets the necessary headers for the error response, including the "Content-Type" as "text/html" and "Connection" as "close".
 * 
 * @param statusCode    The HTTP status code for the error (e.g., 404 for Not Found, 500 for Internal Server Error).
 * @param errorPagePath The path to a custom error page. If empty or invalid, a default error message is generated.
 * @return              The HTTP response object with the appropriate error status, message, and headers.
 */

HttpResponse handleError(int statusCode, const std::string &errorPagePath) 
{
    std::cout << RED <<"error page path : "<< errorPagePath << RESET << std::endl; // Debug
    HttpResponse response;
    response.setStatusCode(statusCode);

    if (!errorPagePath.empty()) {
        struct stat pathStat;
        if (stat(errorPagePath.c_str(), &pathStat) != 0) {
            // Path access error (file does not exist or insufficient permissions)
            response.setBody("Error " + toString(statusCode));
        } else if (S_ISDIR(pathStat.st_mode)) {
            // Path is a directory, treat as if error file not found
            response.setBody("Error " + toString(statusCode));
        } else {
            // The path is a regular file, try to open it
            std::ifstream errorFile(errorPagePath.c_str(), std::ios::in | std::ios::binary);
            if (errorFile.is_open()) {
                std::stringstream buffer;
                buffer << errorFile.rdbuf();
                std::string errorContent = buffer.str();
                errorFile.close();
                response.setBody(errorContent);
            } else {
                response.setBody("Error " + toString(statusCode));
            }
        }
    } else {
        // Default Error message
        std::cout << "Message d'erreur par défaut." << std::endl; // Debug
        response.setBody(response.getDefaultReasonPhrase(statusCode));
    }

    // Prepare error HTTP headers
    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    response.setHeader("Connection", "close");

    return response;
}
