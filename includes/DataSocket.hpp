// DataSocket.hpp
#ifndef DATASOCKET_HPP
#define DATASOCKET_HPP

#include <vector>
#include <string>
#include <ctime>
#include "Server.hpp"
#include "Config.hpp"
#include "HttpRequest.hpp"
#include "CgiProcess.hpp"


/**
 * @class DataSocket
 * 
 * The `DataSocket` class is responsible for managing the communication with a client through a socket. 
 * It handles the reception and processing of HTTP requests, as well as sending HTTP responses back to the client. 
 * It also manages the execution of CGI processes for dynamic content generation and handles the communication 
 * between the web server and the CGI process via pipes.
 * 
 * - **Data Reception and Parsing**: The class receives data from the client, parses the HTTP request, and 
 *   checks if the request is complete. If there is a parse error, it manages the error handling.
 * 
 * - **Request Processing**: It processes the HTTP request, including handling static file serving and delegating 
 *   to CGI processes for dynamic content.
 * 
 * - **CGI Process Management**: The class manages the CGI process, including reading from the CGI pipe, 
 *   checking the status of the CGI process, and handling its timeout and exit status.
 * 
 * - **Data Sending and Timeout**: It manages the sending of the HTTP response to the client and checks for 
 *   inactivity timeouts to close the socket if no activity is detected.
 * 
 * - **Socket Management**: The class provides methods for closing the socket, checking if the request is complete, 
 *   and retrieving the last activity time to handle client disconnections or timeouts.
 * 
 * This class plays a crucial role in the web server by managing both client communication and dynamic content 
 * generation through CGI processes, ensuring smooth data transfer and proper error handling.
 */

class DataSocket {
public:
    DataSocket(int fd, const std::vector<Server*>& servers, const Config* config);
    ~DataSocket();

    bool receiveData();
    void handleParseError(int errorCode);
    bool isRequestComplete() const;
    void processRequest();
    bool sendData();
    bool hasDataToSend() const;
    void closeSocket();
    int getSocket() const;
    const Server* getAssociatedServer() const;
    time_t getLastActivityTime() const;

    // CGI handling methods
    bool hasCgiProcess() const;
    int getCgiPipeFd() const;
    bool isCgiComplete() const;
    bool readFromCgiPipe();
    void handleCgiProcessExitStatus();
    void closeCgiPipe();

    bool cgiProcessIsRunning() const;
    bool cgiProcessHasTimedOut() const;
    void terminateCgiProcess(int errorCode);


private:
    int client_fd_;
    std::vector<Server*> associatedServers_;
    HttpRequest httpRequest_;
    bool requestComplete_;
    const Config *config_;
    std::string sendBuffer_;
    size_t sendBufferOffset_;
    
    // Check Inactivity Timeout
    time_t lastActivityTime_; 

    // CGI handling attributes
    CgiProcess* cgiProcess_;
    int cgiPipeFd_;
    bool cgiComplete_;
    std::string cgiOutputBuffer_;
    bool shouldCloseAfterSend_;
};

#endif // DATASOCKET_HPP
