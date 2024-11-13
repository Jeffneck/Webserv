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
        switch (statusCode) {
            case 400: response.setBody("Bad Request"); break;
            case 401: response.setBody("Unauthorized"); break;
            case 403: response.setBody("Forbidden"); break;
            case 404: response.setBody("Not Found"); break;
            case 405: response.setBody("Method Not Allowed"); break;
            case 408: response.setBody("Request Timeout"); break;
            case 500: response.setBody("Internal Server Error"); break;
            case 501: response.setBody("Not Implemented"); break;
            case 502: response.setBody("Bad Gateway"); break;
            case 503: response.setBody("Service Unavailable"); break;
            case 504: response.setBody("Gateway Timeout"); break;
            case 505: response.setBody("HTTP Version Not Supported"); break;
            default: response.setBody("Error " + toString(statusCode));
        }
    }

    // Définir les en-têtes HTTP appropriés
    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    response.setHeader("Connection", "close");

    return response;
}

// HttpResponse handleError(int statusCode, std::string &errorPagePath) 
// {
//     std::cout << "handle error" << std::endl;//test
//     HttpResponse response;
//     response.setStatusCode(statusCode);

//     // Récupérer la page d'erreur personnalisée si disponible
//     std::string errorPageUri = "";
//     std::string root = "";
//     if (server) {
//         errorPageUri = server->getErrorPage(statusCode);
//         root = server->getRoot();
//     } else if (config) {
//         errorPageUri = config->getErrorPage(statusCode);
//         root = config->getRoot();

//     }

//     if (!errorPageUri.empty()) {
//         // Construire le chemin complet de la page d'erreur
//         std::string errorPagePath = root + errorPageUri;
//         std::cout << "error page path : "<< errorPagePath << std::endl;//test
//         std::ifstream errorFile(errorPagePath.c_str(), std::ios::in | std::ios::binary);
//         if (errorFile.is_open()) {
//             std::cout << "error File found" << std::endl;//test
//             std::stringstream buffer;
//             buffer << errorFile.rdbuf();
//             std::string errorContent = buffer.str();
//             errorFile.close();
//             response.setBody(errorContent);
//         } else {
//             response.setBody("Error " + toString(statusCode));
//         }
//     } else {
//         // Message d'erreur par défaut
//         switch (statusCode) {
//             case 400: response.setBody("Bad Request"); break;
//             case 401: response.setBody("Unauthorized"); break;
//             case 403: response.setBody("Forbidden"); break;
//             case 404: response.setBody("Not Found"); break;
//             case 405: response.setBody("Method Not Allowed"); break;
//             case 408: response.setBody("Request Timeout"); break;
//             case 500: response.setBody("Internal Server Error"); break;
//             case 501: response.setBody("Not Implemented"); break;
//             case 502: response.setBody("Bad Gateway"); break;
//             case 503: response.setBody("Service Unavailable"); break;
//             case 504: response.setBody("Gateway Timeout"); break;
//             default: response.setBody("Error " + toString(statusCode));
//         }
//     }

//     // Définir le Content-Type
//     response.setHeader("Content-Type", "text/html");

//     return response;
// }
// // Ajoutez d'autres surcharges si nécessaire pour différents types