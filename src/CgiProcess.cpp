#include "CgiProcess.hpp"
#include "Color_Macros.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <iostream>
#include <cstdlib> // Pour _exit()
#include <errno.h>
#include <cstring> // Pour strerror()

CgiProcess::CgiProcess(const std::string& scriptWorkingDir, const std::string& relativeFilePath,
                       const std::map<std::string, std::string>& scriptParams,
                       const std::vector<std::string>& envVars)
    : pid_(-1), 
    scriptWorkingDir_(scriptWorkingDir), 
    relativeFilePath_(relativeFilePath), 
    maxExecutionTime_(11),
    cgiExitStatus_(0)
{
    createArgv(scriptParams);
    createEnvp(envVars);
    pipefd_[0] = pipefd_[1] = -1;
}

CgiProcess::~CgiProcess() {
    cleanupArgv();
    cleanupEnvp();
    if (pipefd_[0] != -1) close(pipefd_[0]);
    if (pipefd_[1] != -1) close(pipefd_[1]);
    if (pid_ > 0) waitpid(pid_, &cgiExitStatus_, WNOHANG);
    std::cout << "Waitpid destruct"<< cgiExitStatus_ << std::endl;
}

bool CgiProcess::start() {
    // std::cout << "CgiProcess::start : path absolu repertoire : " << scriptWorkingDir_ << " path relatif fichier : " << relativeFilePath_ << std::endl;

    if (pipe(pipefd_) == -1) {
        std::cerr << "Error : CGI pipe failed: " << strerror(errno) << std::endl;
        return false;
    }

    // Rendre le descripteur de lecture non bloquant
    if (fcntl(pipefd_[0], F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "Error : CGI fcntl pipe failed: " << strerror(errno) << std::endl;
        return false;
    }

    pid_ = fork();
    if (pid_ == -1) {
        std::cerr << "Error : CGI fork failed: " << strerror(errno) << std::endl;
        return false;
    }

    if (pid_ == 0) {
        // Processus enfant
        close(pipefd_[0]);
        dup2(pipefd_[1], STDOUT_FILENO);
        close(pipefd_[1]);

        // change working dir to 'scriptWorkingDir_'
        if (chdir(scriptWorkingDir_.c_str()) == -1) {
            std::cerr << "Error : CGI chdir failed: " << strerror(errno) << std::endl;
            _exit(1);
        }

        // Exec script with python and args
        if (execve(args_[0], &args_[0], &envp_[0]) == -1) {
            std::cerr << "Error : CGI execve failed: " << strerror(errno) << std::endl;
            _exit(1);
        }
    }

    // Parent process

    // Fermer le descripteur d'écriture inutilisé
    close(pipefd_[1]);
    startTime_ = time(NULL);
    return true;
}

bool CgiProcess::hasTimedOut() const {
    time_t currentTime = time(NULL);
    return difftime(currentTime, startTime_) > maxExecutionTime_;
}

void CgiProcess::terminate() {
    if (pid_ > 0) {
        kill(pid_, SIGKILL);
        waitpid(pid_, &cgiExitStatus_, 0); // Éviter les processus zombies
        std::cout << "Waitpid terminate"<< cgiExitStatus_ << std::endl;
        pid_ = -1;
    }
} 

bool CgiProcess::isRunning() {
    pid_t result = waitpid(pid_, &cgiExitStatus_, WNOHANG);
    std::cout << "Waitpid running" << cgiExitStatus_<< std::endl;
    if (result == 0) {
        // process still executing
        return true;
    } else {
        // process ended
        pid_ = -1;
        return false;
    }
}

int CgiProcess::getPipeFd() const {
    return pipefd_[0];
}

int CgiProcess::getExitStatus(){
    waitpid(pid_, &cgiExitStatus_, 0);
    // std::cout << "get exit status" << cgiExitStatus_<< std::endl;
    return cgiExitStatus_;
}

void CgiProcess::createEnvp(const std::vector<std::string>& envVars) {
    for (size_t i = 0; i < envVars.size(); ++i) {
        envStrings_.push_back(envVars[i]);
        envp_.push_back(const_cast<char*>(envStrings_.back().c_str()));
    }
    envp_.push_back(NULL);
}

void CgiProcess::cleanupEnvp() {
    envp_.clear();
    envStrings_.clear();
}


void CgiProcess::createArgv(const std::map<std::string, std::string>& scriptParams) {
    // Path to Python interpretor
    std::string pythonInterpreter = "/usr/bin/python3";
    argStrings_.push_back(pythonInterpreter);
    args_.push_back(const_cast<char*>(argStrings_.back().c_str()));

    // Relative path to python script
    argStrings_.push_back(relativeFilePath_);
    args_.push_back(const_cast<char*>(argStrings_.back().c_str()));

    // Add params in the script as arguments
    for (std::map<std::string, std::string>::const_iterator it = scriptParams.begin(); it != scriptParams.end(); ++it) {
        // Format des arguments : --key=value
        std::string arg = "--" + it->first + "=" + it->second;
        paramDecode(arg);
        argStrings_.push_back(arg);
        args_.push_back(const_cast<char*>(argStrings_.back().c_str()));
    }

    // end arguments tab with NULL
    args_.push_back(NULL);
}

void CgiProcess::cleanupArgv() {
    args_.clear();
    argStrings_.clear();
}

// Decode encoded chars (%hexa format) in the QueryString
void CgiProcess::paramDecode(std::string& param) const {
    std::string decoded;
    char hex[3];
    hex[2] = '\0';
    for (std::string::size_type i = 0; i < param.length(); ++i) {
        if (param[i] == '%') {
            if (i + 2 < param.length()) {
                hex[0] = param[i + 1];
                hex[1] = param[i + 2];
                decoded += static_cast<char>(std::strtol(hex, NULL, 16));
                i += 2;
            }
        } else if (param[i] == '+') {
            decoded += ' ';
        } else {
            decoded += param[i];
        }
    }
    param = decoded;
}