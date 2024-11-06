// HttpRequest.cpp
#include "../includes/HttpRequest.hpp"
#include "../includes/Color_Macros.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

HttpRequest::HttpRequest()
    : rawData_(""), 
    method_(""), 
    rawPath_(""), 
    path_(""), 
    queryString_(""), 
    httpVersion_(""), 
    body_(""), 
    contentLength_(0), 
    headersParsed_(false), 
    state_(REQUEST_LINE) {
}

HttpRequest::~HttpRequest() {
}

void HttpRequest::appendData(const std::string& data) {
    rawData_ += data;
}

bool HttpRequest::isComplete() const {
    if (!headersParsed_) {
        // std::cout << "HttpRequest::isComplete()  : headers are not parsed " << std::endl;//test
        return false;
    }
    // std::cout << "HttpRequest::isComplete()  : body_ size: " << body_.size() << " / " << contentLength_ << std::endl;//test

    return body_.size() >= contentLength_;
}

bool HttpRequest::parseRequest() {
    std::istringstream stream(rawData_);
    std::string line;

    while (state_ != COMPLETE && std::getline(stream, line)) {
        if (state_ == REQUEST_LINE) {
            if (line.empty()) {
                return false;
            }
            parseRequestLine(line);
            state_ = HEADERS;
        } else if (state_ == HEADERS) {
            if (line == "\r" || line.empty()) {
                headersParsed_ = true;
                std::map<std::string, std::string>::iterator it = headers_.find("Content-Length");
                if (it != headers_.end()) {
                    std::istringstream lengthStream(it->second);
                    int length;
                    if (!(lengthStream >> length) || length < 0) {
                        return false;  // Rejeter les requêtes avec un `Content-Length` invalide (négatif ou non numérique) Attention il faut throw une erreur html apres
                    }
                    contentLength_ = static_cast<size_t>(length);
                } else {
                    contentLength_ = 0;
                }

                state_ = (contentLength_ > 0) ? BODY : COMPLETE;
            } else {
                parseHeaderLine(line);
            }
        } else if (state_ == BODY) {
            size_t bodyStartPos = rawData_.find("\r\n\r\n");
            if (bodyStartPos != std::string::npos) {
                bodyStartPos += 4; // Passer les "\r\n\r\n"
                body_ = rawData_.substr(bodyStartPos, contentLength_);
                if (body_.size() >= contentLength_) {
                    state_ = COMPLETE;
                } else {
                    return false; // Attendre plus de données
                }
            } else {
                return false; // Attendre plus de données
            }
        }
    }
    return state_ == COMPLETE;
}

void HttpRequest::parseRequestLine(const std::string& line) {
    std::istringstream lineStream(line);
    lineStream >> method_ >> rawPath_ >> httpVersion_;

    // Supprimer le retour chariot de httpVersion_ s'il est présent
    if (!httpVersion_.empty() && httpVersion_[httpVersion_.size() - 1] == '\r') {
        httpVersion_.erase(httpVersion_.size() - 1);
    }

    // Retirer la query string du rawPath_
    size_t queryPos = rawPath_.find('?');
    if (queryPos != std::string::npos) {
        queryString_ = rawPath_.substr(queryPos + 1); // Stocker la query string
        rawPath_ = rawPath_.substr(0, queryPos); // Garder uniquement la partie avant le '?'
    } else {
        queryString_.clear(); // Aucune query string, on vide la variable
    }

    // Normaliser le chemin
    path_ = normalizePath(rawPath_);

    // std::cout << "Parsed request line: " << method_ << " " << path_ << " " << httpVersion_ << std::endl; // test
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

// void HttpRequest::parseRequestLine(const std::string& line) {
//     std::istringstream lineStream(line);
//     lineStream >> method_ >> path_ >> httpVersion_;

//     // Supprimer le retour chariot de httpVersion_ s'il est présent
//     if (!httpVersion_.empty() && httpVersion_[httpVersion_.size() - 1] == '\r') {
//         httpVersion_.erase(httpVersion_.size() - 1);
//     }

//     // Retirer la query string du path_
//     size_t queryPos = path_.find('?');
//     if (queryPos != std::string::npos) {
//         queryString_ = path_.substr(queryPos + 1); // Stocker la query string
//         path_ = path_.substr(0, queryPos); // Garder uniquement la partie avant le '?'
//     } else {
//         queryString_.clear(); // Aucune query string, on vide la variable
//     }

//     // std::cout << "HttpRequest::parseRequestLine  : Parsed request line: " << method_ << " " << path_ << " " << httpVersion_ << std::endl; // test
// }

void HttpRequest::parseHeaderLine(const std::string& line) {
    std::string::size_type pos = line.find(':');
    if (pos != std::string::npos) {
        std::string headerName = line.substr(0, pos);
        std::string headerValue = line.substr(pos + 1);

        // Supprimer les espaces
        headerName.erase(headerName.find_last_not_of(" \t\r\n") + 1);
        headerValue.erase(0, headerValue.find_first_not_of(" \t\r\n"));
        headerValue.erase(headerValue.find_last_not_of(" \t\r\n") + 1);

        headers_[headerName] = headerValue;

        // std::cout << "HttpRequest::parseRequestLine  : Parsed header: " << headerName << " = '" << headerValue << "'"  << std::endl;//test
    }
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
    method_.clear();
    path_.clear();
    httpVersion_.clear();
    body_.clear();
    rawData_.clear();
    contentLength_ = 0;
    headersParsed_ = false;
    state_ = REQUEST_LINE;
    headers_.clear();
}