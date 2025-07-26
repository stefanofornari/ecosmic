#ifndef CPE_UTILS_H
#define CPE_UTILS_H

#include <string>
#include <vector>

// Helper function to parse a line of space-separated doubles
bool parseDoubles(const std::string& line, std::vector<double>& doubles);

#endif // CPE_UTILS_H
