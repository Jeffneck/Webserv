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
    void readFromCgiPipe();
    void closeCgiPipe();

    bool cgiProcessIsRunning() const;
    bool cgiProcessHasTimedOut() const;
    void terminateCgiProcess();


private:
    int client_fd_;
    std::vector<Server*> associatedServers_;
    HttpRequest httpRequest_;
    bool requestComplete_;
    const Config *config_;
    std::string sendBuffer_;
    size_t sendBufferOffset_;
    time_t lastActivityTime_; 

    // CGI
    CgiProcess* cgiProcess_;
    // pid_t cgiPid_;
    int cgiPipeFd_;
    bool cgiComplete_;
    std::string cgiOutputBuffer_;
    bool shouldCloseAfterSend_;
};

#endif // DATASOCKET_HPP
