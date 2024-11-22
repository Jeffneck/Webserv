// DataSocket.cpp
#include "DataSocket.hpp"
#include "RequestHandler.hpp"
#include "Color_Macros.hpp"
#include "Error.hpp"
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <errno.h>//debug
#include <cstring>//debug

DataSocket::DataSocket(int fd, const std::vector<Server*>& servers, const Config* config)
    : client_fd_(fd), associatedServers_(servers), requestComplete_(false), config_(config),
      sendBufferOffset_(0), cgiProcess_(NULL), cgiPipeFd_(-1), cgiComplete_(true),
      shouldCloseAfterSend_(false) {
    // Initialiser le temps de la dernière activité à l'instant de la création
    lastActivityTime_ = time(NULL);
}


DataSocket::~DataSocket() {
    std::cout << "DESTRUCTOR Datasocket" << std::endl;
    // closeSocket();
    if (cgiProcess_) {
        delete cgiProcess_;
        cgiProcess_ = NULL;
    }
}

bool DataSocket::receiveData() {
    char buffer[4096];
    ssize_t bytesRead = recv(client_fd_, buffer, sizeof(buffer), 0);

    if (bytesRead > 0) {
        lastActivityTime_ = time(NULL);
        std::string data(buffer, bytesRead);
        httpRequest_.appendData(data);

        if (httpRequest_.parseRequest()) {
            if (httpRequest_.hasParseError()) {
                handleParseError(httpRequest_.getParseErrorCode());
                // Ne pas fermer immédiatement, permettre l'envoi de la réponse
                return true;
            }
            requestComplete_ = httpRequest_.isComplete();
        } else {
            if (httpRequest_.hasParseError()) {
                handleParseError(httpRequest_.getParseErrorCode());
                // Ne pas fermer immédiatement, permettre l'envoi de la réponse
                return true;
            }
        }
        return true;
    } else if (bytesRead == 0) {
        // Le client a fermé proprement la connexion
        std::cout << "Le client a fermé la connexion (recv() a retourné 0)." << std::endl;
        return false; 
    } else if (bytesRead == -1) {
        // Une erreur est survenue lors de la réception
        std::cerr << "Erreur lors de recv() (retourne -1). Fermeture de la connexion." << std::endl;
        return false;
    }
    return true;
}

void DataSocket::handleParseError(int errorCode) {
    std::cerr << "DataSocket::handleParseError: Detected parse error code " << errorCode << std::endl;
    RequestResult result;
    const Server* server = getAssociatedServer();
    if (server) {
        result.response = handleError(errorCode, server->getErrorPageFullPath(errorCode));
    } else {
        result.response = handleError(errorCode, config_->getErrorPageFullPath(errorCode));
    }
    sendBuffer_ = result.response.generateResponse();
    sendBufferOffset_ = 0;
    //An error happened during parsing so the socket need to be closed
    shouldCloseAfterSend_ = true;
}

const Server* DataSocket::getAssociatedServer() const {
    if (!associatedServers_.empty()) {
        return associatedServers_[0];
    }
    return NULL;
}

bool DataSocket::isRequestComplete() const {
    return requestComplete_;
}

void DataSocket::processRequest() {
    RequestHandler handler(*config_, associatedServers_);
    RequestResult result = handler.handleRequest(httpRequest_);

    if (result.responseReady) {
        sendBuffer_ = result.response.generateResponse();
        sendBufferOffset_ = 0;
    } else if (result.cgiProcess) {
        cgiProcess_ = result.cgiProcess;
        cgiPipeFd_ = cgiProcess_->getPipeFd();
        // cgiPid_ = cgiProcess_->getPid();
        // std::cout << CYAN <<"DataSocket::processRequest result.cgiprocess : " << cgiPipeFd_ << RESET <<std::endl;//test
        // std::cout << CYAN <<"DataSocket::processRequest result.cgipid: " << cgiPid_ << RESET <<std::endl;//test
        cgiComplete_ = false;
    } else {
        sendBuffer_ = result.response.generateResponse();
        sendBufferOffset_ = 0;
    }

    httpRequest_.reset();
    requestComplete_ = false;
}

bool DataSocket::sendData() {
    // std::cout << CYAN <<"DataSocket::sendData " RESET <<std::endl;
    
    if (sendBuffer_.empty()) {
        return true;
    }

    ssize_t bytesSent = send(client_fd_, sendBuffer_.c_str() + sendBufferOffset_, sendBuffer_.size() - sendBufferOffset_, 0);
    //Data hs been succesfully sent 
    if (bytesSent > 0) {
        lastActivityTime_ = time(NULL);
        sendBufferOffset_ += bytesSent;
        if (sendBufferOffset_ >= sendBuffer_.size()) {
            sendBuffer_.clear();
            sendBufferOffset_ = 0;
            //If an error detected : shouldCloseAfterSend_ = true
            if (shouldCloseAfterSend_) {
                return false;
            }
            return true;
        }
    } 
    //Sometines occur in non-blocking systems, we will retry to send data
    else if (bytesSent == 0) {
        return true;
    } 
    //error detected during send, we close the socket responsible
    else if (bytesSent == -1) {
        std::cerr << "Error occurs while sending data via a DataSocket" << std::endl;
        return false;
    }

    return true;
}

bool DataSocket::hasDataToSend() const {
    return !sendBuffer_.empty();
}

void DataSocket::closeSocket() {
    if (client_fd_ != -1) {
        close(client_fd_);
        client_fd_ = -1;
        // std::cout << RED <<"DataSocket::closeSocket: Socket closed."<< RESET << std::endl;
    }
}

int DataSocket::getSocket() const {
    return client_fd_;
}

time_t DataSocket::getLastActivityTime() const {
    return lastActivityTime_;
}


// CGI handling methods
bool DataSocket::hasCgiProcess() const {
    return cgiProcess_ != NULL;
}

int DataSocket::getCgiPipeFd() const {
    return cgiPipeFd_;
}

bool DataSocket::isCgiComplete() const {
    return cgiComplete_;
}

bool DataSocket::readFromCgiPipe() {
    // std::cout << RED << "DataSocket::readFromCgiPipe()" << RESET << std::endl;
    
    char buffer[4096];
    ssize_t bytesRead = read(cgiPipeFd_, buffer, sizeof(buffer));

    if (bytesRead > 0) {
        lastActivityTime_ = time(NULL);
        cgiOutputBuffer_.append(buffer, bytesRead);
        // std::cout << BLUE << "CGI added buffer: " << std::string(buffer, bytesRead) << RESET << std::endl;
        return true;
    } else if (bytesRead == 0) {
        handleCgiProcessExitStatus();
        return false;
    }else{
        std::cerr << "CGI Gateway Info : Error occured while reading on cgi Pipe" << std::endl;
        terminateCgiProcess(502);
        return false;
    }
}

void    DataSocket::handleCgiProcessExitStatus()
{
    // std::cout << YELLOW<< "DataSocket::handleCgiProcessExitStatus"<< RESET << std::endl;
    int status = cgiProcess_->getExitStatus();
    if(cgiProcess_ && status){
        // Vérify exit status
        if (WIFEXITED(status)) {
            int exitStatus = WEXITSTATUS(status);
            if (exitStatus != 0) {
                std::cerr << "CGI Gateway Info : CGI process exited with error code: " << exitStatus << std::endl;
                HttpResponse response = handleError(502, getAssociatedServer()->getErrorPageFullPath(502));
                sendBuffer_ = response.generateResponse();
                sendBufferOffset_ = 0;
            } 
        } else if (WIFSIGNALED(status)) {
            std::cerr << "CGI Gateway Info : CGI process was terminated by a signal." << std::endl;
            HttpResponse response = handleError(502, getAssociatedServer()->getErrorPageFullPath(502));
            sendBuffer_ = response.generateResponse();
            sendBufferOffset_ = 0;
        } else {
            std::cerr << "CGI Gateway Info : CGI process terminated abnormally." << std::endl;
            HttpResponse response = handleError(502, getAssociatedServer()->getErrorPageFullPath(502));
            sendBuffer_ = response.generateResponse();
            sendBufferOffset_ = 0;
        }
    } else {
        // CGI ended successfully
        HttpResponse response;
        response.setStatusCode(200);
        response.setBody(cgiOutputBuffer_);
        response.setHeader("Content-Type", "text/html; charset=UTF-8");
        response.setHeader("Connection", "close");
        sendBuffer_ = response.generateResponse();
        sendBufferOffset_ = 0;
        cgiOutputBuffer_.clear();
    }
}

bool DataSocket::cgiProcessIsRunning() const {
    if (cgiProcess_) {
        return cgiProcess_->isRunning();
    }
    return false;
}

bool DataSocket::cgiProcessHasTimedOut() const {
    if (cgiProcess_) {
        return cgiProcess_->hasTimedOut();
    }
    return false;
}

void DataSocket::terminateCgiProcess(int errorCode) {
    if (cgiProcess_) {
        cgiProcess_->terminate();
        closeCgiPipe();
        HttpResponse response = handleError(errorCode, getAssociatedServer()->getErrorPageFullPath(errorCode));
        sendBuffer_ = response.generateResponse();
        sendBufferOffset_ = 0;
        cgiOutputBuffer_.clear();
    }
}

void DataSocket::closeCgiPipe() {
    if (cgiPipeFd_ != -1) {
        close(cgiPipeFd_);
        cgiPipeFd_ = -1;
    }
    if (cgiProcess_) {
        delete cgiProcess_;
        cgiProcess_ = NULL;
    }
    cgiComplete_ = true;
}