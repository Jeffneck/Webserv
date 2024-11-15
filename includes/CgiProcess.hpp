// CgiProcess.hpp
#ifndef CGIPROCESS_HPP
#define CGIPROCESS_HPP

#include <string>
#include <vector>
#include <map>

class CgiProcess {
public:
    // Modifié le constructeur pour accepter les arguments sous forme de map
    CgiProcess(const std::string& scriptWorkingDir, const std::string& relativeFilePath, const std::map<std::string, std::string>& params, const std::vector<std::string>& envVars);
    ~CgiProcess();

    bool start();
    bool isRunning();
    int getPipeFd() const;
    int getExitStatus() const;
    // std::string readOutput();

    bool hasTimedOut() const;
    bool isOutputComplete() const;
    bool isOutputError() const;
    void terminate();

private:
    pid_t pid_;
    int pipefd_[2];

    std::string scriptWorkingDir_;
    std::string relativeFilePath_;

    // Ajout des membres pour stocker les arguments et l'environnement
    std::vector<char*> args_;
    std::vector<char*> envp_;

    // Stockage des chaînes pour assurer leur durée de vie
    std::vector<std::string> argStrings_;
    std::vector<std::string> envStrings_;

    // Gerer le temps d' execution max 
    time_t startTime_;
    int maxExecutionTime_;

    //stocker le statut en sortie de waitpid
    int cgiExitStatus_;

    //verifier la sortie de read
    // bool outputComplete_;
    // bool outputError_;

    // Méthodes pour créer et nettoyer les arguments et l'environnement
    void createArgv(const std::map<std::string, std::string>& scriptParams);
    void cleanupArgv();

    std::map<std::string, std::string> createScriptParams(const std::string& queryString);
    void paramDecode(std::string& arg) const;

    void createEnvp(const std::vector<std::string>& envVars);
    void cleanupEnvp();
};

#endif // CGIPROCESS_HPP
