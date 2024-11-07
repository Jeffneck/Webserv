// DataSocket.cpp
#include "DataSocket.hpp"
#include "RequestHandler.hpp"
#include "Color_Macros.hpp"
#include "Error.hpp"
#include <unistd.h>
#include <iostream>
#include <errno.h>//debug
#include <cstring>//debug

DataSocket::DataSocket(int fd, const std::vector<Server*>& servers, const Config& config)
    : client_fd_(fd), associatedServers_(servers), requestComplete_(false), config_(config),
      sendBufferOffset_(0), cgiProcess_(NULL), cgiPipeFd_(-1), cgiComplete_(true),
      shouldCloseAfterSend_(false) { }


DataSocket::~DataSocket() {
    closeSocket();
    if (cgiProcess_) {
        delete cgiProcess_;
        cgiProcess_ = NULL;
    }
}

bool DataSocket::receiveData() {
    char buffer[4096];
    ssize_t bytesRead = recv(client_fd_, buffer, sizeof(buffer), 0);

    if (bytesRead > 0) {
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
        return false;
    } else {
        return false;
    }
}

void DataSocket::handleParseError(int errorCode) {
    std::cerr << "DataSocket::handleParseError: Detected parse error code " << errorCode << std::endl;
    RequestResult result;
    const Server* server = getAssociatedServer();
    if (server) {
        result.response = handleError(errorCode, server->getErrorPageFullPath(errorCode));
    } else {
        result.response = handleError(errorCode, config_.getErrorPageFullPath(errorCode));
    }
    sendBuffer_ = result.response.generateResponse();
    sendBufferOffset_ = 0;
    // Indiquer que la connexion doit être fermée après l'envoi
    shouldCloseAfterSend_ = true;
}

const Server* DataSocket::getAssociatedServer() const {
    // Implémentez cette méthode pour retourner le serveur associé à cette DataSocket
    // Cela dépend de votre architecture, mais souvent vous pouvez obtenir le serveur à partir des associations
    // passées lors de la création de DataSocket dans WebServer::runEventLoop()
    if (!associatedServers_.empty()) {
        return associatedServers_[0]; // Exemple simplifié
    }
    return NULL;
}

bool DataSocket::isRequestComplete() const {
    return requestComplete_;
}

void DataSocket::processRequest() {
    RequestHandler handler(config_, associatedServers_);
    RequestResult result = handler.handleRequest(httpRequest_);

    if (result.responseReady) {
        sendBuffer_ = result.response.generateResponse();
        sendBufferOffset_ = 0;
    } else if (result.cgiProcess) {
        cgiProcess_ = result.cgiProcess;
        cgiPipeFd_ = cgiProcess_->getPipeFd();
        // std::cout << GREEN <<"DataSocket::processRequest result.cgiprocess : " << cgiPipeFd_ << RESET <<std::endl;//test
        cgiComplete_ = false;
    } else {
        sendBuffer_ = result.response.generateResponse();
        sendBufferOffset_ = 0;
    }

    httpRequest_.reset();
    requestComplete_ = false;
}

bool DataSocket::sendData() {
    if (sendBuffer_.empty()) {
        return true;
    }

    ssize_t bytesSent = send(client_fd_, sendBuffer_.c_str() + sendBufferOffset_, sendBuffer_.size() - sendBufferOffset_, 0);
    if (bytesSent > 0) {
        sendBufferOffset_ += bytesSent;
        if (sendBufferOffset_ >= sendBuffer_.size()) {
            sendBuffer_.clear();
            sendBufferOffset_ = 0;
            if (shouldCloseAfterSend_) {
                return false;
            }
            return true;
        }
    } else if (bytesSent == 0) {
        return false;
    } else {
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
        std::cout << RED <<"DataSocket::closeSocket: Socket closed."<< RESET << std::endl;
    } else {
        std::cout <<RED << "Tentative de fermeture d'une socket déjà fermée." << RESET << std::endl;
    }
}

int DataSocket::getSocket() const {
    return client_fd_;
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

void DataSocket::readFromCgiPipe() {
    std::cout << RED << "DataSocket::readFromCgiPipe()" << RESET << std::endl; // Debug
    
    char buffer[4096];
    ssize_t bytesRead = read(cgiPipeFd_, buffer, sizeof(buffer));

    if (bytesRead > 0) {
        cgiOutputBuffer_.append(buffer, bytesRead);
        std::cout << BLUE << "CGI added buffer: " << std::string(buffer, bytesRead) << RESET << std::endl; // Debug
    } else if (bytesRead == 0) {
        // EOF atteint, le processus CGI a terminé
        closeCgiPipe();

        HttpResponse response;
        response.setStatusCode(200);
        response.setBody(cgiOutputBuffer_);
        response.setHeader("Content-Type", "text/html; charset=UTF-8");
        sendBuffer_ = response.generateResponse();
        sendBufferOffset_ = 0;
        cgiOutputBuffer_.clear();
    } else {
        // bytesRead < 0, une erreur s'est produite
        if (!cgiProcess_->isRunning()) {
                // Le processus CGI s'est terminé sans envoyer EOF
                std::cerr << "CGI process terminated without sending EOF." << std::endl;
                closeCgiPipe();

                HttpResponse response;
                response = handleError(500, getAssociatedServer()->getErrorPageFullPath(500));
                sendBuffer_ = response.generateResponse();
                sendBufferOffset_ = 0;
                cgiOutputBuffer_.clear();
            }
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

void DataSocket::terminateCgiProcess() {
    if (cgiProcess_) {
        cgiProcess_->terminate();
        closeCgiPipe();
        // Envoyer une réponse d'erreur au client
        HttpResponse response = handleError(500, getAssociatedServer()->getErrorPageFullPath(500));
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