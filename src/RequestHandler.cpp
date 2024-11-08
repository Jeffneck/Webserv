// RequestHandler.cpp
#include "RequestHandler.hpp"
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits.h>
#include "../includes/Utils.hpp"
#include "../includes/Error.hpp"
#include "../includes/Color_Macros.hpp"
#include <cerrno> // Pour errno
#include <string.h> // Pour errno


RequestHandler::RequestHandler(const Config& config, const std::vector<Server*>& associatedServers)
    : config_(config), associatedServers_(associatedServers)
{
}

RequestHandler::~RequestHandler() {}

RequestResult RequestHandler::handleRequest(const HttpRequest& request) {
    RequestResult result;
    const Server* server = selectServer(request);

    // Vérifier si le serveur est NULL
    if (!server) {
        result.response = handleError(400, config_.getErrorPageFullPath(400));
        result.responseReady = true;
        return result;
    }

    const Location* location = selectLocation(server, request);

    process(server, location, request, result);
    return result;
}

const Server* RequestHandler::selectServer(const HttpRequest& request) const {
    std::string hostHeader = request.getHeader("Host");
    if (hostHeader.empty()) {
        std::cerr << "No Host header found in the request." << std::endl;
        return NULL; // Retourner NULL pour indiquer une erreur
    }

    // Parcourir tous les serveurs pour trouver une correspondance
    for (size_t i = 0; i < associatedServers_.size(); ++i) {
        const std::vector<std::string>& serverNames = associatedServers_[i]->getServerNames();
        if (std::find(serverNames.begin(), serverNames.end(), hostHeader) != serverNames.end()) {
            std::cerr << "RequestHandler::selectServer   : Server found by name => first name of server : " << associatedServers_[i]->getServerNames()[0] << " ip:port : "<< associatedServers_[i]->getHost() << ":"<< associatedServers_[i]->getPort() << std::endl; //test
            return associatedServers_[i];
        }
    }

    // Si aucun serveur ne correspond, utiliser le premier serveur comme défaut
    if (!associatedServers_.empty()) {
        std::cerr << "RequestHandler::selectServer   : Default server Used => first name of server : " << associatedServers_[0]->getServerNames()[0] << " ip:port : "<< associatedServers_[0]->getHost() << ":"<< associatedServers_[0]->getPort() << std::endl;//test

        return associatedServers_[0];
    }

    std::cerr << "RequestHandler::selectServer   : No server found. Default server must exist." << std::endl;//test
    // Aucun serveur configuré
    return NULL;
}

const Location* RequestHandler::selectLocation(const Server* server, const HttpRequest& request) const {
    // Vérifier si le serveur est NULL
    if (!server) {
        return NULL;
    }

    std::string requestPath = request.getPath();
    const std::vector<Location>& locations = server->getLocations();

    const Location* matchedLocation = NULL;
    size_t longestMatch = 0;

    for (size_t i = 0; i < locations.size(); ++i) {
        std::string locPath = locations[i].getPath();
        if (requestPath.find(locPath) == 0 && locPath.length() > longestMatch) {
            matchedLocation = &locations[i];
            longestMatch = locPath.length();
        }
    }

    if (matchedLocation)
        std::cout << "Request Path: " << requestPath << " Matched location: " << matchedLocation->getPath() << std::endl; //test
    return matchedLocation;
}

void RequestHandler::process(const Server* server, const Location* location, const HttpRequest& request, RequestResult& result) const {
    // Vérifier si le serveur est NULL
    if (!server) {
        result.response = handleError(400, config_.getErrorPageFullPath(400));
        result.responseReady = true;
        return;
    }

    // Collecte des méthodes autorisées
    std::vector<std::string> allowedMethods;
    if (location && !location->getAllowedMethods().empty()) {
        std::cout << "Using allowed methods found in location" << std::endl;
        allowedMethods = location->getAllowedMethods();
    } else {
        std::cout << "Using default allowed methods GET POST DELETE" << std::endl;
        allowedMethods.push_back("GET");
        allowedMethods.push_back("POST");
        allowedMethods.push_back("DELETE");
    }

    // Vérifier si la méthode HTTP est autorisée
    if (std::find(allowedMethods.begin(), allowedMethods.end(), request.getMethod()) == allowedMethods.end()) {
        // Méthode non autorisée
        std::string errorPagePath = getErrorPageFullPath(405, location, server);
        result.response = handleError(405, errorPagePath);
        // Ajouter l'en-tête Allow
        result.response.setHeader("Allow", join(allowedMethods, ", "));
        result.responseReady = true;
        return;
    }

    // Redirection
    if (location && !location->getRedirection().empty()) {
        std::string redirectionUrl = location->getRedirection();
        result.response.setStatusCode(302);
        result.response.setHeader("Location", redirectionUrl);
        result.response.setBody("Redirecting to " + redirectionUrl);
        result.responseReady = true;
        return;
    }

    // Gestion des CGI
    if (location && !location->getCgiExtension().empty() && location->getCGIEnable() && endsWith(request.getPath(), location->getCgiExtension())) {
        CgiProcess* cgiProcess = startCgiProcess(server, location, request);
        if (cgiProcess) {
            result.cgiProcess = cgiProcess;
            result.responseReady = false;
            return;
        } else {
            std::string errorPagePath = getErrorPageFullPath(500, location, server);
            result.response = handleError(500, errorPagePath);
            result.responseReady = true;
            return;
        }
    }

    // Gestion des fichiers statiques
    if (request.getMethod() == "GET") {
        result.response = serveStaticFile(server, location, request);
        result.responseReady = true;
        return;
    }

    // Gestion de l'upload de fichiers
    if (request.getMethod() == "POST" && location && location->getUploadEnable()) {
        result.response = handleFileUpload(request, location, server);
        result.responseReady = true;
        return;
    }

    // Gestion des delete de fichiers
    if (request.getMethod() == "DELETE" && location) {
        result.response = handleDeletion(request, location, server);
        result.responseReady = true;
        return;
    }

    // Si aucune condition précédente n'est satisfaite, retourner une erreur 405
    std::string errorPagePath = getErrorPageFullPath(405, location, server);
    result.response = handleError(405, errorPagePath);
    result.response.setHeader("Allow", join(allowedMethods, ", "));
    result.responseReady = true;
}

CgiProcess* RequestHandler::startCgiProcess(const Server* server, const Location* location , const HttpRequest& request) const {
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        std::cerr << RED <<"RequestHandler::startCgiProcess : Error getcwd: " << strerror(errno) << RESET << std::endl;
        return NULL; // Retourner NULL pour indiquer une erreur
    }
    std::string scriptWorkingDir = cwd ;
    scriptWorkingDir += "/" ;
    scriptWorkingDir += server->getRoot();
    if (location){
        scriptWorkingDir += "/" ;
        scriptWorkingDir += location->getPath();
        scriptWorkingDir += "/";
    }
    std::string relativeFilePath = request.getPath();
    if (location && relativeFilePath.compare(0, location->getPath().length(), location->getPath()) == 0) {
        relativeFilePath.erase(0, location->getPath().length());
    }
    relativeFilePath = "./" + relativeFilePath;

    std::map<std::string, std::string> params;
    if (request.getMethod() == "GET") {
        // Extraire les paramètres de la query string
        params = createScriptParamsGET(request.getQueryString());
    } else if (request.getMethod() == "POST") {
        std::string contentType = request.getHeader("Content-Type");
        if (contentType == "application/x-www-form-urlencoded") {
            // Extraire les paramètres du corps de la requête
            params = createScriptParamsPOST(request.getBody());
        } else {
            // Gérer d'autres types de contenu ou renvoyer une erreur
            std::cerr << "Type de contenu non pris en charge pour POST: " << contentType << std::endl;
            return NULL;
        }
    } else {
        // Méthode non prise en charge
        std::cerr << "Méthode HTTP non prise en charge: " << request.getMethod() << std::endl;
        return NULL;
    }

    std::vector<std::string> envVars;
    setupScriptEnvp(request, relativeFilePath, envVars);

    // Créer l'objet CgiProcess avec les paramètres
    CgiProcess* cgiProcess = new CgiProcess(scriptWorkingDir, relativeFilePath, params, envVars);
    if (!cgiProcess->start()) {
        delete cgiProcess;
        return NULL;
    }
    return cgiProcess;
}

void RequestHandler::setupScriptEnvp(const HttpRequest& request, const std::string& relativeFilePath,  std::vector<std::string>& envVars) const{
    envVars.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
    envVars.push_back("REQUEST_METHOD=" + request.getMethod());
    envVars.push_back("SCRIPT_FILENAME=" + relativeFilePath);
    envVars.push_back("CONTENT_TYPE=" + request.getHeader("Content-Type"));
    envVars.push_back("CONTENT_LENGTH=" + request.getHeader("Content-Length"));
    // envVars.push_back("QUERY_STRING=" + request.getQueryString());//inutile puisque deja envoyees dans les arguments
    
}


std::map<std::string, std::string> RequestHandler::createScriptParamsGET(const std::string& queryString) const {
    std::map<std::string, std::string> params;
    std::string::size_type last_pos = 0, amp_pos;

    while ((amp_pos = queryString.find('&', last_pos)) != std::string::npos) {
        std::string key_value_pair = queryString.substr(last_pos, amp_pos - last_pos);
        std::string::size_type eq_pos = key_value_pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = key_value_pair.substr(0, eq_pos);
            std::string value = key_value_pair.substr(eq_pos + 1);
            params[key] = value;
        } else if (!key_value_pair.empty()) {
            // If there's no '=', treat the entire string as a key with an empty value
            params[key_value_pair] = "";
        }
        last_pos = amp_pos + 1;
    }

    // Handle the last parameter (or only parameter if no '&' was found)
    std::string key_value_pair = queryString.substr(last_pos);
    if (!key_value_pair.empty()) {
        std::string::size_type eq_pos = key_value_pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = key_value_pair.substr(0, eq_pos);
            std::string value = key_value_pair.substr(eq_pos + 1);
            params[key] = value;
        } else {
            // If there's no '=', treat the entire string as a key with an empty value
            params[key_value_pair] = "";
        }
    }

    return params;
}

std::map<std::string, std::string> RequestHandler::createScriptParamsPOST(const std::string& postBody) const {
    std::map<std::string, std::string> params;
    std::string::size_type last_pos = 0, amp_pos;

    while ((amp_pos = postBody.find('&', last_pos)) != std::string::npos) {
        std::string key_value_pair = postBody.substr(last_pos, amp_pos - last_pos);
        std::string::size_type eq_pos = key_value_pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = key_value_pair.substr(0, eq_pos);
            std::string value = key_value_pair.substr(eq_pos + 1);
            params[key] = value;
        } else if (!key_value_pair.empty()) {
            params[key_value_pair] = "";
        }
        last_pos = amp_pos + 1;
    }

    // Traiter le dernier paramètre
    std::string key_value_pair = postBody.substr(last_pos);
    if (!key_value_pair.empty()) {
        std::string::size_type eq_pos = key_value_pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = key_value_pair.substr(0, eq_pos);
            std::string value = key_value_pair.substr(eq_pos + 1);
            params[key] = value;
        } else {
            params[key_value_pair] = "";
        }
    }

    return params;
}


HttpResponse RequestHandler::serveStaticFile(const Server* server, const Location* location, const HttpRequest& request) const {
    HttpResponse response;

    // Déterminer le répertoire racine et le fichier index
    std::string root = server->getRoot();
    std::string index = server->getIndex();

    if (location) {
        root = location->getRoot();
        index = location->getIndex();
    }

    // Construire le chemin complet du fichier demandé
    std::string requestPath = request.getPath();

    // Vérifier que requestPath n'est pas vide
    if (requestPath.empty()) {
        requestPath = "/";
    }

    // Gérer le cas où le chemin se termine par un '/'
    if (requestPath[requestPath.size() - 1] == '/') {
        if (location && !location->getIndexIsSet() && location->getAutoIndex()) {
            // Générer l'auto-index si l'index n'est pas défini et que l'auto-index est activé
            return generateAutoIndex(root + requestPath, requestPath);
        }
        // Ajouter le fichier index au chemin
        requestPath += index;
    }

    // Retirer le chemin de la location du requestPath si root est défini dans la location
    if (location && location->getRootIsSet()) {
        std::string to_remove = location->getPath();
        size_t pos = requestPath.find(to_remove);
        if (pos != std::string::npos) {
            requestPath.erase(pos, to_remove.length());
        }
    }

    // Construire le chemin complet vers le fichier
    std::string fullPath = root + requestPath;
    std::cout << "Serving file: " << fullPath << std::endl;

    // Vérifier la sécurité du chemin
    if (!isPathSecure(root, fullPath)) {
        std::string errorPagePath = getErrorPageFullPath(403, location, server);
        return handleError(403, errorPagePath);
    }

    // Vérifier si le fichier existe et est accessible
    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) != 0) {
        if (errno == EACCES) {
            std::string errorPagePath = getErrorPageFullPath(403, location, server);
            return handleError(403, errorPagePath); // Forbidden
        } else if (errno == ENOENT || errno == ENOTDIR) {
            std::string errorPagePath = getErrorPageFullPath(404, location, server);
            return handleError(404, errorPagePath); // Not Found
        } else {
            std::string errorPagePath = getErrorPageFullPath(500, location, server);
            return handleError(500, errorPagePath); // Internal Server Error
        }
    }

    // Vérifier que c'est un fichier régulier
    if (!S_ISREG(fileStat.st_mode)) {
        std::string errorPagePath = getErrorPageFullPath(403, location, server);
        return handleError(403, errorPagePath); // Forbidden
    }

    // Ouvrir le fichier demandé
    std::ifstream file(fullPath.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        if (errno == EACCES) {
            std::string errorPagePath = getErrorPageFullPath(403, location, server);
            return handleError(403, errorPagePath); // Forbidden
        } else {
            std::string errorPagePath = getErrorPageFullPath(404, location, server);
            return handleError(404, errorPagePath); // Not Found
        }
    }

    // Lire le contenu du fichier
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContent = buffer.str();
    file.close();

    // Définir les en-têtes et le corps de la réponse
    response.setStatusCode(200);
    response.setBody(fileContent);

    // Définir le Content-Type en fonction de l'extension du fichier
    size_t dotPos = fullPath.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string extension = fullPath.substr(dotPos + 1);
        std::string contentType = getMimeType(extension);
        if (!contentType.empty()) {
            response.setHeader("Content-Type", contentType);
            response.setHeader("Connection", "close");
        }
    }

    return response;
}


HttpResponse RequestHandler::handleFileUpload(const HttpRequest& request, const Location* location, const Server* server) const {
    std::cout << RED << "RequestHandler::handleFileUpload" << RESET << std::endl; // Test
    HttpResponse response;

    // Vérifier que le Content-Type est multipart/form-data
    std::string contentType = request.getHeader("Content-Type");
    if (contentType.find("multipart/form-data") != 0 ) {
        std::string errorPagePath = getErrorPageFullPath(400, location, server);
        response = handleError(400, errorPagePath);
        return response;
    }

    // Extraire la limite (boundary) de l'en-tête Content-Type
    std::string boundaryPrefix = "boundary=";
    std::string::size_type boundaryPos = contentType.find(boundaryPrefix);
    if (boundaryPos == std::string::npos) {
        // Pas de boundary trouvé
        std::string errorPagePath = getErrorPageFullPath(400, location, server);
        response = handleError(400, errorPagePath);
        return response;
    }
    boundaryPos += boundaryPrefix.length();
    std::string boundary = "--" + contentType.substr(boundaryPos);

    // Lire le corps de la requête
    std::string body = request.getBody();

    // Vérifier que le répertoire d'upload existe
    std::string uploadDirectory = location->getUploadStore();  // Obtenir le répertoire d'upload depuis la configuration
    struct stat dirStat;
    if (stat(uploadDirectory.c_str(), &dirStat) != 0 || !S_ISDIR(dirStat.st_mode)) {
        std::cerr << "Upload directory does not exist: " << uploadDirectory << std::endl;
        std::string errorPagePath = getErrorPageFullPath(500, location, server);
        response = handleError(500, errorPagePath);
        return response;
    }

    // Séparer les différentes parties
    std::string::size_type start = 0;
    while ((start = body.find(boundary, start)) != std::string::npos) {
        start += boundary.length();
        std::string::size_type end = body.find(boundary, start);
        if (end == std::string::npos) break;

        std::string part = body.substr(start, end - start);

        // Extraire les en-têtes de la partie
        std::string::size_type headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos) continue;

        std::string headers = part.substr(0, headerEnd);
        std::string fileData = part.substr(headerEnd + 4);

        // Vérifier si cette partie est un fichier
        std::string filenamePrefix = "filename=\"";
        std::string::size_type filenamePos = headers.find(filenamePrefix);
        if (filenamePos != std::string::npos) {
            std::string::size_type filenameEndPos = headers.find("\"", filenamePos + filenamePrefix.length());
            if (filenameEndPos != std::string::npos) {
                std::string filename = headers.substr(filenamePos + filenamePrefix.length(), filenameEndPos - (filenamePos + filenamePrefix.length()));

                // Construire le chemin complet pour sauvegarder le fichier
                std::string fullPath = uploadDirectory + "/" + filename;

                // Sauvegarder le fichier
                std::ofstream file(fullPath.c_str(), std::ios::binary);
                if (file.is_open()) {
                    file.write(fileData.c_str(), fileData.size());
                    file.close();
                    std::cout << "File saved: " << fullPath << std::endl;
                } else {
                    std::cerr << "Failed to save file: " << fullPath << std::endl;
                    std::string errorPagePath = getErrorPageFullPath(500, location, server);
                    response = handleError(500, errorPagePath);
                    return response;
                }
            }
        }
    }

    response.setStatusCode(201);
    response.setBody("File upload successful.");
    return response;
}

HttpResponse RequestHandler::handleDeletion(const HttpRequest& request, const Location* location, const Server* server) const {
    std::cout << RED << "RequestHandler::handleDeletion" << RESET << std::endl; // Debug

    HttpResponse response;

    // Déterminer le répertoire racine
    std::string root = server->getRoot();
    if (location) {
        root = location->getRoot();
    }

    // Construire le chemin complet du fichier à supprimer
    std::string requestPath = request.getPath();

    // Gérer le cas où le chemin se termine par un '/'
    if (!requestPath.empty() && requestPath[requestPath.size() - 1] == '/') {
        // Les requêtes DELETE ne devraient pas se terminer par un '/', renvoyer une erreur
        std::string errorPagePath = getErrorPageFullPath(400, location, server);
        response = handleError(400, errorPagePath);
        return response;
    }

    // Retirer le chemin de la location du requestPath si root est défini dans la location
    if (location && location->getRootIsSet()) {
        std::string to_remove = location->getPath();
        if (requestPath.find(to_remove) == 0) {
            requestPath.erase(0, to_remove.length());
        }
    }

    // Construire le chemin absolu vers le fichier
    std::string fullPath = root + requestPath;
    std::cout << "Attempting to delete file: " << fullPath << std::endl; // Debug

    // Sécuriser le chemin
    if (!isPathSecure(root, fullPath)) {
        std::string errorPagePath = getErrorPageFullPath(403, location, server);
        response = handleError(403, errorPagePath); // Forbidden
        return response;
    }

    // Vérifier si le fichier existe
    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) != 0) {
        if (errno == ENOENT) {
            std::string errorPagePath = getErrorPageFullPath(404, location, server);
            response = handleError(404, errorPagePath); // Not Found
            return response;
        } else {
            std::cerr << "RequestHandler::handleDeletion : Error accessing file: " << strerror(errno) << std::endl;
            std::string errorPagePath = getErrorPageFullPath(500, location, server);
            response = handleError(500, errorPagePath); // Internal Server Error
            return response;
        }
    }

    // Vérifier que c'est un fichier régulier
    if (!S_ISREG(fileStat.st_mode)) {
        std::cerr << "RequestHandler::handleDeletion : Target is not a regular file." << std::endl;
        std::string errorPagePath = getErrorPageFullPath(403, location, server);
        response = handleError(403, errorPagePath); // Forbidden
        return response;
    }

    // Vérifier les permissions de suppression
    if (access(fullPath.c_str(), W_OK) != 0) {
        std::cerr << "RequestHandler::handleDeletion : No permission to delete the file: " << strerror(errno) << std::endl;
        std::string errorPagePath = getErrorPageFullPath(403, location, server);
        response = handleError(403, errorPagePath); // Forbidden
        return response;
    }

    // Tenter de supprimer le fichier
    if (unlink(fullPath.c_str()) != 0) {
        std::cerr << "RequestHandler::handleDeletion : Failed to delete file: " << strerror(errno) << std::endl;
        std::string errorPagePath = getErrorPageFullPath(500, location, server);
        response = handleError(500, errorPagePath); // Internal Server Error
        return response;
    }

    // Construire la réponse de succès
    response.setStatusCode(200);
    response.setBody("File deleted successfully.");
    response.setHeader("Content-Type", "text/plain; charset=UTF-8");
    response.setHeader("Connection", "close");

    return response;
}


HttpResponse RequestHandler::generateAutoIndex(const std::string& fullPath, const std::string& requestPath) const {
    std::cout << RED << "RequestHandler::generateAutoIndex" << RESET << std::endl; // test
    std::stringstream ss;

    
    ss << "<html><head><title>Index of " << requestPath << "</title></head><body>";
    ss << "<h1>Index of " << requestPath << "</h1><ul>";

    DIR* dir = opendir(fullPath.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string name = entry->d_name;
            if (name == ".")
                continue;
            ss << "<li><a href=\"" << name << "\">" << name << "</a></li>";
        }
        closedir(dir);
    } else {
        ss << "<li>Error reading directory.</li>";
    }

    ss << "</ul></body></html>";

    
    HttpResponse response;
    response.setStatusCode(200);
    response.setBody(ss.str());
    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    response.setHeader("Connection", "close");

    return response; 
}

std::string RequestHandler::getMimeType(const std::string& extension) const {
    if (extension == "html" || extension == "htm")
        return "text/html";
    if (extension == "css")
        return "text/css";
    if (extension == "js")
        return "application/javascript";
    if (extension == "png")
        return "image/png";
    if (extension == "jpg" || extension == "jpeg")
        return "image/jpeg";
    if (extension == "gif")
        return "image/gif";
    if (extension == "txt")
        return "text/plain";
    // Ajouter d'autres types MIME selon les besoins
    return "application/octet-stream"; // Type par défaut
}

bool RequestHandler::isPathSecure(const std::string& root, const std::string& fullPath) const {
    char realRoot[PATH_MAX];
    char realFullPath[PATH_MAX];

    if (realpath(root.c_str(), realRoot) == NULL) {
        std::cerr << "Invalid root path: " << root << std::endl;
        return false;
    }
    realpath(fullPath.c_str(), realFullPath);

    std::string realRootStr(realRoot);
    std::string realFullPathStr(realFullPath);
    // std::cout << YELLOW << "root: "<< realRootStr << " fullpath : "<< realFullPathStr << std::endl;//debug
    // Vérifier que fullPath commence par root
    if (realFullPathStr.find(realRootStr) != 0) {
        std::cerr << "Path traversal attempt detected: " << fullPath << std::endl;
        return false;
    }

    return true;
}

std::string RequestHandler::getErrorPageFullPath(int statusCode, const Location* location, const Server* server) const {
    if (location && !location->getErrorPageFullPath(statusCode).empty()) {
        return location->getErrorPageFullPath(statusCode);
    } else if (server && !server->getErrorPageFullPath(statusCode).empty()) {
        return server->getErrorPageFullPath(statusCode);
    } else {
        return config_.getErrorPageFullPath(statusCode);
    }
}

std::string RequestHandler::join(const std::vector<std::string>& elements, const std::string& delimiter) const {
    std::ostringstream os;
    for (size_t i = 0; i < elements.size(); ++i) {
        os << elements[i];
        if (i < elements.size() - 1) {
            os << delimiter;
        }
    }
    return os.str();
}
