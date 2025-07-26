#include "utils.h"
#include <sstream>
#include <iostream>

bool parseDoubles(const std::string& line, std::vector<double>& doubles) {
    doubles.clear();
    std::istringstream iss(line);
    double val;

    while (iss >> val) {
        doubles.push_back(val);
    }

    if (iss.fail() && !iss.eof()) {
        return false;
    }

    std::string remaining_chars;
    iss >> remaining_chars;
    if (!remaining_chars.empty()) {
        return false;
    }

    return true;
}
