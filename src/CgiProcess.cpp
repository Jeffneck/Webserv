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
}


/**
 * Starts the CGI process by creating a pipe, forking a child process, and executing the script.
 * The function sets up the necessary pipe for communication and executes the script (e.g., a Python script).
 * If any step fails (e.g., pipe creation, fork, changing the working directory, or executing the script), the function returns false.
 * 
 * @return true if the CGI process is successfully started, false otherwise.
 */
bool CgiProcess::start() {
    // std::cout << "CgiProcess::start : path absolu repertoire : " << scriptWorkingDir_ << " path relatif fichier : " << relativeFilePath_ << std::endl;

    //Simulate an error of pipe/fork/fcntl here to make err 500 happen
    // return false; //remove this to make the function works normally

    if (pipe(pipefd_) == -1) {
        std::cerr << "Error : CGI pipe failed: " << strerror(errno) << std::endl;
        return false;
    }

    // Make reading descriptor non-blocking
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
        // Child process
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
    close(pipefd_[1]);
    // Used to monitor the time of the process (Inactive process = timeout)
    startTime_ = time(NULL);
    return true;
}

/**
 * Checks if the CGI process has exceeded the maximum allowed execution time.
 * This function compares the current time with the start time of the process to determine if the process has timed out.
 * 
 * @return true if the CGI process has timed out, false otherwise.
 */
bool CgiProcess::hasTimedOut() const {
    time_t currentTime = time(NULL);
    return difftime(currentTime, startTime_) > maxExecutionTime_;
}

/**
 * Terminates the CGI process by sending a kill signal to the child process.
 * This function ensures the child process is forcefully terminated and avoids the creation of a zombie process by calling `waitpid`.
 * 
 * @return void
 */
void CgiProcess::terminate() {
    if (pid_ > 0) {
        kill(pid_, SIGKILL);
        // Avoid Zombie process
        waitpid(pid_, &cgiExitStatus_, 0); 
        pid_ = -1;
    }
} 

/**
 * Checks if the CGI process is still running.
 * This function uses `waitpid` with the `WNOHANG` option to check if the child process has completed.
 * 
 * @return true if the process is still running, false if it has ended.
 */
bool CgiProcess::isRunning() {
    pid_t result = waitpid(pid_, &cgiExitStatus_, WNOHANG);
    if (result == 0) {
        // process still executing
        return true;
    } else {
        // process ended
        pid_ = -1;
        return false;
    }
}

/**
 * Returns the file descriptor of the pipe used for reading the output of the CGI process.
 * This function provides the read end of the pipe, which is used to retrieve the CGI process's output.
 * 
 * @return The file descriptor for reading from the CGI process's pipe.
 */
int CgiProcess::getPipeFd() const {
    return pipefd_[0];
}

/**
 * Retrieves the exit status of the CGI process.
 * This function waits for the CGI process to terminate and returns its exit status.
 * 
 * @return The exit status of the CGI process.
 */
int CgiProcess::getExitStatus(){
    waitpid(pid_, &cgiExitStatus_, 0);
    return cgiExitStatus_;
}

/**
 * Creates the environment variables (envp) for the CGI process from a list of environment variables.
 * The environment variables are formatted and added to the `envp_` vector to be passed to the CGI process.
 * 
 * @param envVars A vector of environment variables to be passed to the CGI process.
 * @return void
 */
void CgiProcess::createEnvp(const std::vector<std::string>& envVars) {
    for (size_t i = 0; i < envVars.size(); ++i) {
        envStrings_.push_back(envVars[i]);
        envp_.push_back(const_cast<char*>(envStrings_.back().c_str()));
    }
    envp_.push_back(NULL);
}

/**
 * Cleans up the environment variables by clearing the `envp_` and `envStrings_` vectors.
 * This function is used to reset the environment variables after they have been used.
 * 
 * @return void
 */
void CgiProcess::cleanupEnvp() {
    envp_.clear();
    envStrings_.clear();
}

/**
 * Creates the argument vector (argv) for the CGI process from the provided script parameters.
 * This function constructs the argument list required to execute the CGI script, including the script path and parameters.
 * 
 * @param scriptParams A map of script parameters to be passed to the CGI process as arguments.
 * @return void
 */
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


/**
 * Decodes a URL-encoded parameter string (e.g., decoding percent-encoded characters).
 * This function handles decoding of percent-encoded characters (e.g., %20 for space) and replaces '+' with space.
 * 
 * @param param The parameter string to be decoded.
 * @return void
 */
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