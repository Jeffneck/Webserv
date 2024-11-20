// HttpRequest.cpp
#include "../includes/HttpRequest.hpp"
#include "../includes/Color_Macros.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <set>

HttpRequest::HttpRequest()
    : rawData_(""), 
      method_(""), 
      rawPath_(""), 
      path_(""), 
      queryString_(""), 
      httpVersion_(""), 
      body_(""), 
      contentLength_(0), 
      bodyStartPos_(0), 
      headersParsed_(false), 
      state_(REQUEST_LINE),
      parseError_(false),
      parseErrorCode_(0)
{
}

HttpRequest::~HttpRequest() {
}

void HttpRequest::appendData(const std::string& data) {
    rawData_ += data;
}

bool HttpRequest::isComplete() const {
    if (!headersParsed_) {
        return false;
    }

    return body_.size() >= contentLength_;
}

bool HttpRequest::parseRequest() {
    std::istringstream stream(rawData_);
    std::string line;

    while (state_ != COMPLETE && std::getline(stream, line)) {
        if (state_ == REQUEST_LINE) {
            if (!handleRequestLine(line)) {
                return false;
            }
        } else if (state_ == HEADERS) {
            if (!handleHeaders(line)) {
                return false;
            }
        } else if (state_ == BODY) {
            if (!handleBody()) {
                return false;
            }
        }
    }
    return state_ == COMPLETE;
}

bool HttpRequest::handleRequestLine(const std::string& line) {
    if (line == "\r" || line.empty()) {
        // Incomplete Request
        return false;
    }
    if (!parseRequestLine(line)) {
        // An error occurred while parsing the request line
        return false;
    }
    state_ = HEADERS;
    return true;
}

bool HttpRequest::handleHeaders(const std::string& line) {
    if (line == "\r" || line.empty()) {
        headersParsed_ = true;
        if (!validateHeaders()) {
            return false;
        }
        // Record the position of the beginning of the body
        bodyStartPos_ = rawData_.find("\r\n\r\n");
        if (bodyStartPos_ != std::string::npos) {
            bodyStartPos_ += 4; // Passer les "\r\n\r\n"
        } else {
            // Can't find end of headers
            parseError_ = true;
            parseErrorCode_ = 400; // Bad Request
            return false;
        }
        state_ = (contentLength_ > 0) ? BODY : COMPLETE;
    } else {
        parseHeaderLine(line);
    }
    return true;
}

bool HttpRequest::handleBody() {
    // Calculate the remaining body size to be read
    size_t bodyReceivedLength = rawData_.size() - bodyStartPos_;
    if (bodyReceivedLength >= contentLength_) {
        body_ = rawData_.substr(bodyStartPos_, contentLength_);
        state_ = COMPLETE;
        return true;
    } else {
        //need to read few more times 
        return false;
    }
}


bool HttpRequest::validateHeaders() {
    // Vérifier la présence de l'en-tête Host
    if (headers_.find("host") == headers_.end()) {
        std::cerr << "Missing Host header in HTTP request." << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }
    if (method_ == "POST" && (!validatePOSTContentType() || !validatePOSTContentLength() )) {
        return false;
    }
    return true;
}

bool HttpRequest::validatePOSTContentLength() {
    // Search for 'Transfer-Encoding header: chunked'
    std::map<std::string, std::string>::iterator it = headers_.find("transfer-encoding");
    if (it != headers_.end()) {
        contentLength_ = 0;
        std::cerr << "Chuncked requests are not implemented" << std::endl;
        parseError_ = true;
        parseErrorCode_ = 501;
        return false;
    }
    
    // Recherche de l'en-tête Content-Length
    it = headers_.find("content-length");
    if (it == headers_.end()) {
        contentLength_ = 0;
        std::cerr << "Missing Content-Length header in POST request." << std::endl;
        parseError_ = true;
        parseErrorCode_ = 411; // Length Required
        return false;
    }

    // Conversion de la valeur de Content-Length
    std::istringstream lengthStream(it->second);
    int length;
    if (!(lengthStream >> length) || length < 0) {
        std::cerr << "Invalid Content-Length: " << it->second << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }

    contentLength_ = static_cast<size_t>(length);
    return true;
}


bool HttpRequest::validatePOSTContentType() {
    // Verify the presence of 'Content-Type' header
    std::map<std::string, std::string>::iterator it = headers_.find("content-type");
    if (it == headers_.end() || it->second.empty()) {
        std::cerr << "Missing Content-Type header in POST request." << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }

    // Extract the principal MIME TYPE (localized before semicolon)
    std::string contentType = it->second;
    size_t semicolonPos = contentType.find(';');
    if (semicolonPos != std::string::npos) {
        contentType = contentType.substr(0, semicolonPos);
    }

    // Normalize content type string
    contentType = trim(contentType);
    std::transform(contentType.begin(), contentType.end(), contentType.begin(), ::tolower);

    // Vérify if we authorise this content type in our server
    if (contentType == "application/x-www-form-urlencoded" || contentType == "multipart/form-data") {
        return true;
    } else {
        std::cerr << "Unsupported Content-Type: " << contentType << std::endl;
        parseError_ = true;
        parseErrorCode_ = 415; // Unsupported Media Type
        return false;
    }
}

bool HttpRequest::parseRequestLine(const std::string& line) {
    std::istringstream lineStream(line);
    std::string method, rawPath, httpVersion;

    // Limit the max size allowed for the request line
    if (line.length() > MAX_REQUEST_LINE_LENGTH) {
        std::cerr << "Request line too long: " << line << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }

    // Error if less than 3 strings in the line
    if (!(lineStream >> method >> rawPath >> httpVersion)) {
        std::cerr << "Invalid request line: " << line << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }

    // Limit the max size allowed for the URI
    if (rawPath.length() > MAX_URI_LENGTH) {
        std::cerr << "URI too long: " << rawPath << std::endl;
        parseError_ = true;
        parseErrorCode_ = 414; // URI Too Long
        return false;
    }

    std::string extra;
    if (lineStream >> extra) {
        std::cerr << "Invalid request line (too many arguments): " << line << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }

    method_ = method;
    rawPath_ = rawPath;
    httpVersion_ = httpVersion;

    // Del \r from the HTTPver string
    if (!httpVersion_.empty() && httpVersion_[httpVersion_.size() - 1] == '\r') {
        httpVersion_.erase(httpVersion_.size() - 1);
    }

    // Vérify HTTP version
    if (httpVersion_ != "HTTP/1.1" && httpVersion_ != "HTTP/1.0") {
        std::cerr << "Unsupported HTTP version: " << httpVersion_ << std::endl;
        parseError_ = true;
        parseErrorCode_ = 505; // HTTP Version Not Supported
        return false;
    }

    // List of allowed methods in HTTP/1.1
    std::set<std::string> knownMethods;
    knownMethods.insert("GET");
    knownMethods.insert("POST");
    knownMethods.insert("DELETE");
    knownMethods.insert("PUT");
    knownMethods.insert("HEAD");
    knownMethods.insert("OPTIONS");
    knownMethods.insert("PATCH");

    // Detect impossible Methods
    if (knownMethods.find(method_) == knownMethods.end()) {
        std::cerr << "Unknown HTTP method: " << method_ << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }

    // Detect unimplemented Methods
    if (method_ != "GET" && method_ != "POST" && method_ != "DELETE") {
        std::cerr << "Not implemented HTTP method: " << method_ << std::endl;
        parseError_ = true;
        parseErrorCode_ = 501;
        return false;
    }

    // Extract query string from rawPath
    size_t queryPos = rawPath_.find('?');
    if (queryPos != std::string::npos) {
        queryString_ = rawPath_.substr(queryPos + 1); // Stocker la query string
        rawPath_ = rawPath_.substr(0, queryPos);      // Garder uniquement la partie avant le '?'
    } else {
        queryString_.clear(); // Aucune query string, on vide la variable
    }

    // Delete multiples /
    path_ = normalizePath(rawPath_);

    // path have to begin with '/'
    if (path_.empty() || path_[0] != '/') {
        std::cerr << "Invalid request target: " << rawPath_ << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }

    // Successful parsing
    return true;
}



std::string HttpRequest::normalizePath(const std::string& path) const {
    std::string normalizedPath;
    bool prevWasSlash = false;
    for (size_t i = 0; i < path.length(); ++i) {
        char c = path[i];
        if (c == '/') {
            if (!prevWasSlash) {
                normalizedPath += c;
                prevWasSlash = true;
            }
        } else {
            normalizedPath += c;
            prevWasSlash = false;
        }
    }
    return normalizedPath;
}

void HttpRequest::parseHeaderLine(const std::string& line) {
    size_t colonPos = line.find(':');
    //colon is found
    if (colonPos != std::string::npos) {
        // Extract Name and value of the header
        std::string headerName = line.substr(0, colonPos);
        std::string headerValue = line.substr(colonPos + 1);

        // Del whitespaces before Name and after Value
        headerName = trim(headerName);
        headerValue = trim(headerValue);

        // Lowering headerName
        std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);

        headers_[headerName] = headerValue;
    } else {
        // colon not found in the header line
        std::cerr << "Invalid header line: " << line << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400;
    }
}

std::string HttpRequest::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

const std::string& HttpRequest::getMethod() const {
    return method_;
}

const std::string& HttpRequest::getPath() const {
    return path_;
}

const std::string& HttpRequest::getRawPath() const {
    return rawPath_;
}

const std::string& HttpRequest::getHttpVersion() const {
    return httpVersion_;
}

std::string HttpRequest::getHeader(const std::string& headerName) const {
    std::map<std::string, std::string>::const_iterator it = headers_.find(headerName);
    if (it != headers_.end()) {
        return it->second;
    }
    return "";
}

const std::string& HttpRequest::getBody() const {
    return body_;
}

std::string HttpRequest::getQueryString() const {
        return queryString_;
}

bool HttpRequest::hasParseError() const {
    return parseError_;
}

int HttpRequest::getParseErrorCode() const {
    return parseErrorCode_;
}

void HttpRequest::displayContent() const
{
    std::cout << RED <<"HttpRequest::displayContent" << RESET << std::endl;
    std::cout << RED <<"Method : "<< method_ << RESET << std::endl;
    std::cout << RED <<"Path : " << path_ <<RESET << std::endl;
    std::cout << RED <<"HttpVersion : " << httpVersion_ << RESET << std::endl;
    // std::cout << RED <<"Headers" << RESET << std::endl;
    std::cout << RED <<"Body : " << body_ << RESET << std::endl;
}

void HttpRequest::reset() {
    rawData_.clear();
    method_.clear();
    rawPath_.clear();
    path_.clear();
    queryString_.clear();
    httpVersion_.clear();
    body_.clear();
    contentLength_ = 0;
    bodyStartPos_ = 0;
    headersParsed_ = false;
    state_ = REQUEST_LINE;
    headers_.clear();
}
