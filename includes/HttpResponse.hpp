#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>

class HttpResponse {
private:
    int statusCode;                                   // HTTP status code
    std::string reasonPhrase;                         // Reason phrase corresponding to the status code
    std::string body;                                 // Response body
    std::map<std::string, std::string> headers;       // Response headers

public:
    HttpResponse();

    // Methods to set response properties
    void setStatusCode(int code);
    std::string getDefaultReasonPhrase(int code) const;
    void setReasonPhrase(const std::string& phrase);
    void setBody(const std::string& bodyContent);
    void setHeader(const std::string& headerName, const std::string& headerValue);

    // Method to generate the HTTP response string
    std::string generateResponse() const;

private:
};

#endif // HTTPRESPONSE_HPP
