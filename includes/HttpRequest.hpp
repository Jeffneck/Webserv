#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

// these limit can be modified
const size_t MAX_REQUEST_LINE_LENGTH = 500;
const size_t MAX_URI_LENGTH = 250;


/**
 * @class HttpRequest
 * 
 * The `HttpRequest` class is responsible for representing and parsing incoming HTTP requests in a web server. 
 * It is used to store and process data from an incoming HTTP request, including the request line, headers, 
 * body, and any associated query strings. 
 * 
 * - **Request Parsing**: It handles the parsing of the request line, headers, and body, ensuring that the 
 *   incoming request conforms to the HTTP protocol and is valid.
 * 
 * - **Data Handling**: The class can append incoming data, check whether the request is complete, and 
 *   extract specific information such as the HTTP method, path, headers, and body.
 * 
 * - **Error Management**: It provides error handling mechanisms, including the detection of parsing errors 
 *   and the retrieval of error codes when the request is invalid.
 * 
 * - **Content Retrieval**: It allows access to different parts of the request, including the HTTP method 
 *   (`GET`, `POST`, etc.), the requested path, headers, body content, and any query string.
 * 
 * This class acts as a foundational component for request handling, enabling a web server to accurately 
 * process and respond to HTTP requests.
 */

class HttpRequest {
public:
    HttpRequest();
    ~HttpRequest();

    void appendData(const std::string& data);
    bool isComplete() const;
    bool parseRequest();
    
    bool hasParseError() const;
    int getParseErrorCode() const;

    void reset();

    const std::string& getMethod() const;
    const std::string& getPath() const;
    const std::string& getRawPath() const;
    const std::string& getHttpVersion() const;
    std::string getHeader(const std::string& headerName) const;
    const std::string& getBody() const;
    std::string getQueryString() const;

    //debug
    void displayContent() const;

private:
    // Membres de données
    std::string rawData_;
    std::string method_;
    std::string rawPath_;
    std::string path_;
    std::string queryString_;
    std::string httpVersion_;
    std::string body_;
    size_t contentLength_;
    size_t bodyStartPos_;
    bool headersParsed_;
    enum State { REQUEST_LINE, HEADERS, BODY, COMPLETE } state_;
    std::map<std::string, std::string> headers_;

    // Manage errors
    bool parseError_;
    int parseErrorCode_;

    // Parsing 
    bool handleRequestLine(const std::string& line);
    bool handleHeaders(const std::string& line);
    bool handleBody();
    bool validateHeaders();
    bool validatePOSTContentType();
    bool validatePOSTContentLength(); 
    bool parseRequestLine(const std::string& line);
    void parseHeaderLine(const std::string& line);
    std::string normalizePath(const std::string& path) const;
    std::string trim(const std::string& str);
};

#endif // HTTPREQUEST_HPP
