#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>


/**
 * @class HttpResponse
 * 
 * The `HttpResponse` class is responsible for constructing and representing an HTTP response that will be sent by webserver. 
 * It allows setting and managing the various components of an HTTP response, such as the status code, 
 * reason phrase, headers, and body content. This class is used to generate the response in the proper 
 * format before sending it to the client.
 * 
 * - **Response Construction**: The class provides methods to set the HTTP status code, reason phrase, 
 *   headers, and body, allowing the server to generate responses according to the HTTP protocol.
 * 
 * - **Content Management**: It manages the response body and headers, including the ability to set and 
 *   retrieve headers, which are critical for HTTP communication.
 * 
 * - **Response Formatting**: The class includes a method to convert the response into a valid HTTP format 
 *   for transmission over the network.
 * 
 * This class is a key component in the web server’s ability to send properly structured HTTP responses 
 * to clients, ensuring the server communicates effectively with the requesting client.
 */

class HttpResponse {
private:
    int statusCode;                                   
    std::string reasonPhrase;                         
    std::string body;                                 
    std::map<std::string, std::string> headers;       

public:
    HttpResponse();

    // Methods to set response properties
    void setStatusCode(int code);
    std::string getDefaultReasonPhrase(int code) const;
    void setReasonPhrase(const std::string& phrase);
    void setBody(const std::string& bodyContent);
    void setHeader(const std::string& headerName, const std::string& headerValue);

    // Put the response to HTTP format before sending it
    std::string generateResponse() const;

private:
};

#endif // HTTPRESPONSE_HPP
