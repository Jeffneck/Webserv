#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
public:
    HttpRequest();
    ~HttpRequest();

    // Méthodes publiques
    void appendData(const std::string& data);
    bool isComplete() const;
    bool parseRequest();
    
    bool hasParseError() const;
    int getParseErrorCode() const;

    void displayContent() const;
    void reset();

    // Accesseurs
    const std::string& getMethod() const;
    const std::string& getPath() const;
    const std::string& getRawPath() const;
    const std::string& getHttpVersion() const;
    std::string getHeader(const std::string& headerName) const;
    const std::string& getBody() const;
    std::string getQueryString() const;

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

    // Membres pour gérer les erreurs
    bool parseError_;
    int parseErrorCode_;

    // Fonctions privées pour le parsing et la validation
    bool handleRequestLine(const std::string& line);
    bool handleHeaders(const std::string& line);
    bool handleBody();
    bool validateHeaders();
    // bool parseContentLength();
    bool validatePOSTContentType();
    bool validatePOSTContentLength(); 


    bool parseRequestLine(const std::string& line);
    void parseHeaderLine(const std::string& line);
    std::string normalizePath(const std::string& path) const;
    std::string trim(const std::string& str);
};

#endif // HTTPREQUEST_HPP
