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
    if (pid_ > 0) waitpid(pid_, NULL, WNOHANG);
}

bool CgiProcess::start() {
    std::cout << "CgiProcess::start : path absolu repertoire : " << scriptWorkingDir_ << " path relatif fichier : " << relativeFilePath_ << std::endl;

    if (pipe(pipefd_) == -1) {
        std::cerr << "pipe failed: " << strerror(errno) << std::endl;
        return false;
    }

    // Rendre le descripteur de lecture non bloquant
    if (fcntl(pipefd_[0], F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "fcntl failed: " << strerror(errno) << std::endl;
        return false;
    }

    pid_ = fork();
    if (pid_ == -1) {
        std::cerr << "fork failed: " << strerror(errno) << std::endl;
        return false;
    }

    if (pid_ == 0) {
        // Processus enfant

        // Fermer le descripteur de lecture inutilisé
        close(pipefd_[0]);

        // Rediriger la sortie standard vers le descripteur d'écriture du pipe
        dup2(pipefd_[1], STDOUT_FILENO);
        close(pipefd_[1]);

        // Changer le répertoire de travail vers 'scriptWorkingDir_'
        if (chdir(scriptWorkingDir_.c_str()) == -1) {
            std::cerr << "chdir failed: " << strerror(errno) << std::endl;
            _exit(1);
        }

        // Exécuter le script Python avec les arguments et l'environnement
        std::cerr << CYAN << "before execve from child" << RESET << std::endl;

        if (execve(args_[0], &args_[0], &envp_[0]) == -1) {
            std::cerr << "execve failed: " << strerror(errno) << std::endl;
            _exit(1);
        }
    }

    // Processus parent

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
        waitpid(pid_, NULL, 0); // Éviter les processus zombies
        pid_ = -1;
    }
} 

bool CgiProcess::isRunning() {
    pid_t result = waitpid(pid_, &cgiExitStatus_, WNOHANG);
    if (result == 0) {
        // Le processus est toujours en cours d'exécution
        return true;
    } else {
        // Le processus est terminé
        pid_ = -1; // Mettre à jour pid_ pour indiquer que le processus n'est plus actif
        return false;
    }
}

int CgiProcess::getPipeFd() const {
    return pipefd_[0];
}

int CgiProcess::getExitStatus() const {
    return cgiExitStatus_;
}


// std::string CgiProcess::readOutput() {
//     char buffer[4096];
//     ssize_t bytesRead = read(pipefd_[0], buffer, sizeof(buffer));

//     if (bytesRead > 0) {
//         // Des données ont été lues avec succès
//         return std::string(buffer, bytesRead);
//     } else if (bytesRead == 0) {
//         // EOF atteint : le processus CGI a terminé son écriture
//         std::cout << "CgiProcess::readOutput(): EOF atteint sur le pipe (read() a retourné 0)." << std::endl;
//         // Fermer le descripteur de lecture du pipe
//         close(pipefd_[0]);
//         pipefd_[0] = -1;
//         // Indiquer que la lecture est terminée, par exemple en utilisant un drapeau
//         outputComplete_ = true;
//         return "";
//     } else if (bytesRead == -1) {
//         // Une erreur est survenue lors de la lecture
//         std::cerr << "CgiProcess::readOutput(): Erreur lors de read() sur le pipe (retourne -1)." << std::endl;
//         // Fermer le descripteur de lecture du pipe
//         close(pipefd_[0]);
//         pipefd_[0] = -1;
//         // Indiquer qu'une erreur s'est produite
//         outputError_ = true;
//         return "";
//     }
//     // Par sécurité, retourner une chaîne vide
//     return "";
// }

// bool CgiProcess::isOutputComplete() const
// {
//     return outputComplete_;
// }

// bool CgiProcess::isOutputError() const
// {
//     return outputError_;
// }

void CgiProcess::createEnvp(const std::vector<std::string>& envVars) {
    // Stocker les chaînes d'environnement pour assurer leur durée de vie
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
    // Chemin vers l'interpréteur Python
    std::string pythonInterpreter = "/usr/bin/python3";
    argStrings_.push_back(pythonInterpreter);
    args_.push_back(const_cast<char*>(argStrings_.back().c_str()));

    // Chemin relatif vers le script Python
    argStrings_.push_back(relativeFilePath_);
    args_.push_back(const_cast<char*>(argStrings_.back().c_str()));

    // Ajouter les paramètres du script en tant qu'arguments
    for (std::map<std::string, std::string>::const_iterator it = scriptParams.begin(); it != scriptParams.end(); ++it) {
        // Format des arguments : --key=value
        std::string arg = "--" + it->first + "=" + it->second;
        paramDecode(arg);
        argStrings_.push_back(arg);
        args_.push_back(const_cast<char*>(argStrings_.back().c_str()));
    }

    // Terminer le tableau d'arguments avec NULL
    args_.push_back(NULL);
}

void CgiProcess::cleanupArgv() {
    args_.clear();
    argStrings_.clear();
}

// Fonction pour décoder les caractères encodés au format %hexa dans la query string 
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