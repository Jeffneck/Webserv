#include "Utils.hpp"

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