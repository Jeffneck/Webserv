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
        // Requête incomplète
        return false;
    }
    if (!parseRequestLine(line)) {
        // Une erreur s'est produite lors du parsing de la ligne de requête
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
        // Enregistrer la position du début du corps
        bodyStartPos_ = rawData_.find("\r\n\r\n");
        if (bodyStartPos_ != std::string::npos) {
            bodyStartPos_ += 4; // Passer les "\r\n\r\n"
        } else {
            // Impossible de trouver la fin des en-têtes (ne devrait pas arriver)
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
    // Calculer la taille restante du corps à lire
    size_t bodyReceivedLength = rawData_.size() - bodyStartPos_;
    if (bodyReceivedLength >= contentLength_) {
        body_ = rawData_.substr(bodyStartPos_, contentLength_);
        // std::cout << GREEN << " HttpRequest::handleBody() contentlen: "<< contentLength_<< body_ << RESET << std::endl;//test
        state_ = COMPLETE;
        return true;
    } else {
        // Attendre plus de données
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
    // Recherche de l'en-tête Transfer-Encoding: chunked
    std::map<std::string, std::string>::iterator it = headers_.find("transfer-encoding");
    if (it != headers_.end()) {
        contentLength_ = 0;
        std::cerr << "Chuncked requests are not implemented" << std::endl;
        parseError_ = true;
        parseErrorCode_ = 501; // Length Required
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

    // Vérification si Content-Length dépasse 10 Mo (10 * 1024 * 1024 octets)
    // if (static_cast<size_t>(length) > 10 * 1024 * 1024) {
    //     std::cerr << "Content-Length exceeds 10 Mo (max size supported by the server), length: " << length << std::endl;
    //     parseError_ = true;
    //     parseErrorCode_ = 413; // Payload Too Large
    //     return false;
    // }

    contentLength_ = static_cast<size_t>(length);
    return true;
}


bool HttpRequest::validatePOSTContentType() {
    // Récupérer l'en-tête Content-Type
    std::map<std::string, std::string>::iterator it = headers_.find("content-type");
    if (it == headers_.end() || it->second.empty()) {
        std::cerr << "Missing Content-Type header in POST request." << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }

    // Extraire le type MIME principal (avant le point-virgule)
    std::string contentType = it->second;
    size_t semicolonPos = contentType.find(';');
    if (semicolonPos != std::string::npos) {
        contentType = contentType.substr(0, semicolonPos);
    }

    // Supprimer les espaces inutiles et convertir en minuscules pour une comparaison insensible à la casse
    contentType = trim(contentType);
    std::transform(contentType.begin(), contentType.end(), contentType.begin(), ::tolower);

    // Vérifier si le Content-Type est accepté
    if (contentType == "application/x-www-form-urlencoded" || contentType == "multipart/form-data") {
        return true; // Content-Type accepté
    } else {
        std::cerr << "Unsupported Content-Type: " << contentType << std::endl;
        parseError_ = true;
        parseErrorCode_ = 415; // Unsupported Media Type
        return false;
    }
}

// bool HttpRequest::ValidatePOSTContentLength() {

//     if (contentLength_) {
//         std::cerr << "Missing Content-Length header in POST request." << std::endl;
//         parseError_ = true;
//         parseErrorCode_ = 411; // Length Required
//         return false;
//     }
//     return true;
// }

bool HttpRequest::parseRequestLine(const std::string& line) {
    std::istringstream lineStream(line);
    std::string method, rawPath, httpVersion;

    // Limiter la taille max d' une requete
    const size_t MAX_REQUEST_LINE_LENGTH = 100; // Limite typique (à ajuster selon les besoins)
    if (line.length() > MAX_REQUEST_LINE_LENGTH) {
        std::cerr << "Request line too long: " << line << std::endl;
        parseError_ = true;
        parseErrorCode_ = 414; // URI Too Long
        return false;
    }

    if (!(lineStream >> method >> rawPath >> httpVersion)) {
        std::cerr << "Invalid request line: " << line << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }

    // Vérifier si l'URI est trop longue
    const size_t MAX_URI_LENGTH = 2048; // Définir la limite appropriée
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

    // Supprimer le retour chariot de httpVersion_ s'il est présent
    if (!httpVersion_.empty() && httpVersion_[httpVersion_.size() - 1] == '\r') {
        httpVersion_.erase(httpVersion_.size() - 1);
    }

    // Vérifier la version HTTP
    if (httpVersion_ != "HTTP/1.1" && httpVersion_ != "HTTP/1.0") {
        std::cerr << "Unsupported HTTP version: " << httpVersion_ << std::endl;
        parseError_ = true;
        parseErrorCode_ = 505; // HTTP Version Not Supported
        return false;
    }

    // Liste des méthodes reconnues
    std::set<std::string> knownMethods;
    knownMethods.insert("GET");
    knownMethods.insert("POST");
    knownMethods.insert("DELETE");
    knownMethods.insert("PUT");
    knownMethods.insert("HEAD");
    knownMethods.insert("OPTIONS");
    knownMethods.insert("PATCH");

    // Vérifier si la méthode est reconnue
    if (knownMethods.find(method_) == knownMethods.end()) {
        std::cerr << "Unknown HTTP method: " << method_ << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }

    // Vérifier si la méthode est implémentée
    if (method_ == "GET" || method_ == "POST" || method_ == "DELETE") {
        // Méthode implémentée
    } else {
        std::cerr << "Not implemented HTTP method: " << method_ << std::endl;
        parseError_ = true;
        parseErrorCode_ = 501; // Not Implemented
        return false;
    }

    // Retirer la query string du rawPath_
    size_t queryPos = rawPath_.find('?');
    if (queryPos != std::string::npos) {
        queryString_ = rawPath_.substr(queryPos + 1); // Stocker la query string
        rawPath_ = rawPath_.substr(0, queryPos);      // Garder uniquement la partie avant le '?'
    } else {
        queryString_.clear(); // Aucune query string, on vide la variable
    }

    // Normaliser le chemin
    path_ = normalizePath(rawPath_);

    // Vérifier que le chemin commence par '/'
    if (path_.empty() || path_[0] != '/') {
        std::cerr << "Invalid request target: " << rawPath_ << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
        return false;
    }

    // Parsing réussi
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
    if (colonPos != std::string::npos) {
        std::string headerName = line.substr(0, colonPos);
        std::string headerValue = line.substr(colonPos + 1);

        // Supprimer les espaces inutiles
        headerName = trim(headerName);
        headerValue = trim(headerValue);

        // Convertir le nom de l'en-tête en minuscule pour une comparaison insensible à la casse
        std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);

        headers_[headerName] = headerValue;
    } else {
        std::cerr << "Invalid header line: " << line << std::endl;
        parseError_ = true;
        parseErrorCode_ = 400; // Bad Request
    }
}

std::string HttpRequest::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// void HttpRequest::parseHeaderLine(const std::string& line) {
//     std::string::size_type pos = line.find(':');
//     if (pos != std::string::npos) {
//         std::string headerName = line.substr(0, pos);
//         std::string headerValue = line.substr(pos + 1);

//         // Supprimer les espaces
//         headerName.erase(headerName.find_last_not_of(" \t\r\n") + 1);
//         headerValue.erase(0, headerValue.find_first_not_of(" \t\r\n"));
//         headerValue.erase(headerValue.find_last_not_of(" \t\r\n") + 1);

//         headers_[headerName] = headerValue;

//         // std::cout << "HttpRequest::parseRequestLine  : Parsed header: " << headerName << " = '" << headerValue << "'"  << std::endl;//test
//     }
// }

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
    // std::cout << GREEN <<"HttpRequest::getBody()  " << body_ << RESET << std::endl;//test
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
