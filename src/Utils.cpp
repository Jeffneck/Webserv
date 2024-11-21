#include "Utils.hpp"
#include <cstdlib>

std::string toString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string toString(long value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

bool endsWith(const std::string& fullString, const std::string& ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

void decodeURI(std::string &toDecode) 
{
    std::string decoded;
    char hex[3];
    hex[2] = '\0';
    for (std::string::size_type i = 0; i < toDecode.length(); ++i) {
        if (toDecode[i] == '%') {
            if (i + 2 < toDecode.length()) {
                hex[0] = toDecode[i + 1];
                hex[1] = toDecode[i + 2];
                decoded += static_cast<char>(std::strtol(hex, NULL, 16));
                i += 2;
            }
        } else if (toDecode[i] == '+') {
            decoded += ' ';
        } else {
            decoded += toDecode[i];
        }
    }
    toDecode = decoded;
}