// CgiProcess.hpp
#ifndef CGIPROCESS_HPP
#define CGIPROCESS_HPP

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/time.h>


class CgiProcess {
public:
    CgiProcess(const std::string& scriptWorkingDir, const std::string& relativeFilePath, const std::map<std::string, std::string>& params, const std::vector<std::string>& envVars);
    ~CgiProcess();

    bool start();
    bool isRunning();
    int getPipeFd() const;
    int getExitStatus();

    bool hasTimedOut() const;
    bool isOutputComplete() const;
    bool isOutputError() const;
    void terminate();

private:
    pid_t pid_;
    int pipefd_[2];

    //create the envirronnement wherewe want to execute the file
    std::string scriptWorkingDir_;
    std::string relativeFilePath_;

    std::vector<char*> args_;
    std::vector<char*> envp_;

    std::vector<std::string> argStrings_;
    std::vector<std::string> envStrings_;

    // check timeouts
    time_t startTime_;
    int maxExecutionTime_;

    // keep the exit status of the CGI
    int cgiExitStatus_;


    // Manage arguments to give to the CGI
    void createArgv(const std::map<std::string, std::string>& scriptParams);
    void cleanupArgv();

    std::map<std::string, std::string> createScriptParams(const std::string& queryString);
    void paramDecode(std::string& arg) const;

    void createEnvp(const std::vector<std::string>& envVars);
    void cleanupEnvp();
};

#endif // CGIPROCESS_HPP
