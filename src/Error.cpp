#include <fstream>
#include <sstream>
#include <iostream>
#include "Utils.hpp"
#include "Server.hpp"
#include "HttpResponse.hpp"
#include "Color_Macros.hpp"
#include <sys/stat.h> // Pour la fonction stat

// std::string errorPagePath = root + errorPageUri;
HttpResponse handleError(int statusCode, const std::string &errorPagePath) 
{
    std::cout << RED <<"error page path : "<< errorPagePath << RESET << std::endl; // Debug
    HttpResponse response;
    response.setStatusCode(statusCode);

    if (!errorPagePath.empty()) {
        struct stat pathStat;
        if (stat(errorPagePath.c_str(), &pathStat) != 0) {
            // Erreur lors de l'accès au chemin (le fichier n'existe pas ou permissions insuffisantes)
            std::cout << "Erreur lors de l'accès au chemin : " << errorPagePath << std::endl; // Debug
            response.setBody("Error " + toString(statusCode));
        } else if (S_ISDIR(pathStat.st_mode)) {
            // Le chemin est un répertoire, traiter comme si le fichier d'erreur n'est pas trouvé
            std::cout << "Le chemin de la page d'erreur est un répertoire, non un fichier." << std::endl; // Debug
            response.setBody("Error " + toString(statusCode));
        } else {
            // Le chemin est un fichier régulier, tenter de l'ouvrir
            std::ifstream errorFile(errorPagePath.c_str(), std::ios::in | std::ios::binary);
            if (errorFile.is_open()) {
                std::cout << "Fichier d'erreur trouvé." << std::endl; // Debug
                std::stringstream buffer;
                buffer << errorFile.rdbuf();
                std::string errorContent = buffer.str();
                errorFile.close();
                response.setBody(errorContent);
            } else {
                std::cout << "Fichier d'erreur non trouvé ou impossible à ouvrir." << std::endl; // Debug
                response.setBody("Error " + toString(statusCode));
            }
        }
    } else {
        // Message d'erreur par défaut
        std::cout << "Message d'erreur par défaut." << std::endl; // Debug
        response.setBody(response.getDefaultReasonPhrase(statusCode));
    }

    // Définir les en-têtes HTTP appropriés
    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    // response.setHeader("Connection", "close");

    return response;
}
