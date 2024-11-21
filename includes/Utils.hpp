// Utils.hpp
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sstream>
#include <fstream>

// Fonction d'aide pour convertir des entiers en chaînes de caractères
std::string toString(int value);

std::string toString(long value);

bool endsWith(const std::string& fullString, const std::string& ending);
void decodeURI(std::string &toDecode);

#endif // UTILS_HPP
