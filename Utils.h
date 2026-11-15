#ifndef UTILS_H
#define UTILS_H

#include <string>

std::string intToString(int value);
std::string toLowerStr(const std::string& s);

bool containsIgnoreCase(const std::string& haystack, const std::string& needle);
std::string trimSpaces(const std::string& s);
bool parseInt(const std::string& text, int& out);
std::string currentTimestamp();

#endif
