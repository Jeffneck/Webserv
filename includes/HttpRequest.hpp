// HttpRequest.hpp
#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
    public:
        HttpRequest();
        ~HttpRequest();

        void appendData(const std::string& data);
        bool isComplete() const;
        bool parseRequest(); // Retourne true si la requête est complète
        void reset();

        // Accesseurs
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

        void parseRequestLine(const std::string& line);
    std::string normalizePath(const std::string& path) const ;

        void parseHeaderLine(const std::string& line);

        std::string rawData_;
        std::string method_;
        std::string rawPath_;
        std::string path_;
        std::string queryString_;
        std::string httpVersion_;
        std::string body_;
        size_t contentLength_;
        bool headersParsed_;
        std::map<std::string, std::string> headers_;

        // État du parsing
        enum ParseState { REQUEST_LINE, HEADERS, BODY, COMPLETE };
        mutable ParseState state_;
};

#endif // HTTPREQUEST_HPP
