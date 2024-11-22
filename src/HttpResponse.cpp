#include "../includes/HttpResponse.hpp"
#include "../includes/Color_Macros.hpp"
#include <sstream>
#include <iostream>

/*
    classe qui contient les attributs necessaires a la construction d' une reponse http
    cette classe est utilisee par requestHandler qui va set tous ses attributs puis HttpResponse::generateResponse() genere la reponse sous forme de string

*/

HttpResponse::HttpResponse()
    : statusCode(200), reasonPhrase("OK"), body("") {
    headers["Content-Type"] = "text/html";
}

void HttpResponse::setStatusCode(int code) {
    statusCode = code;
    reasonPhrase = getDefaultReasonPhrase(code);
}

void HttpResponse::setReasonPhrase(const std::string& phrase) {
    reasonPhrase = phrase;
}

void HttpResponse::setBody(const std::string& bodyContent) {
    body = bodyContent;
    // Update Content-Length header
    std::ostringstream oss;
    oss << body.size();
    headers["Content-Length"] = oss.str();
}

void HttpResponse::setHeader(const std::string& headerName, const std::string& headerValue) {
    headers[headerName] = headerValue;
}

std::string HttpResponse::generateResponse() const {
    // std::cout << YELLOW <<"\n\n\n\nREPONSE :HttpResponse::generateResponse()\n"<< std::endl;//test
    std::ostringstream response;

    // Status line
    response << "HTTP/1.1 " << statusCode << " " << reasonPhrase << "\r\n";

    // Headers
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }

    response << "\r\n"; // Empty line to separate headers from body

    // std::cout << "HEADERS\n" << response.str() << RESET << std::endl;//test
    // std::cout << "\nBODY\n" << body << RESET << std::endl;//test

    // Body
    response << body;

    return response.str();
}

std::string HttpResponse::getDefaultReasonPhrase(int code) const {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method not Allowed";
        case 408: return "Request Timeout";
        case 411: return "Length Required";
        case 414: return "URI Too Long";
        case 415: return "Unsupported Media Type";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";
        default:  return "OK";
    }
}
