#include "Utils.h"
#include <ctime>

std::string intToString(int value) {
    if (value == 0) return "0";
    bool negative = value < 0;
    long long v = value;
    if (negative) v = -v;
    char buf[24];
    int pos = 0;
    while (v > 0) {
        buf[pos++] = static_cast<char>('0' + (v % 10));
        v /= 10;
    }
    std::string out;
    if (negative) out += '-';
    for (int i = pos - 1; i >= 0; --i) out += buf[i];
    return out;
}

std::string toLowerStr(const std::string& s) {
    std::string out = s;
    for (size_t i = 0; i < out.size(); ++i) {
        if (out[i] >= 'A' && out[i] <= 'Z') out[i] = static_cast<char>(out[i] - 'A' + 'a');
    }
    return out;
}

bool containsIgnoreCase(const std::string& haystack, const std::string& needle) {
    std::string a = toLowerStr(haystack);
    std::string b = toLowerStr(needle);
    if (b.size() == 0) return true;
    for (size_t i = 0; i + b.size() <= a.size(); ++i) {
        size_t j = 0;
        while (j < b.size() && a[i + j] == b[j]) ++j;
        if (j == b.size()) return true;
    }
    return false;
}

std::string trimSpaces(const std::string& s) {
    size_t start = 0;
    size_t end = s.size();
    while (start < end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n')) ++start;
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r' || s[end - 1] == '\n')) --end;
    return s.substr(start, end - start);
}

bool parseInt(const std::string& text, int& out) {
    std::string t = trimSpaces(text);
    if (t.size() == 0 || t.size() > 9) return false;
    int value = 0;
    for (size_t i = 0; i < t.size(); ++i) {
        if (t[i] < '0' || t[i] > '9') return false;
        value = value * 10 + (t[i] - '0');
    }
    out = value;
    return true;
}

std::string currentTimestamp() {
    time_t now = time(NULL);
    struct tm* local = localtime(&now);
    if (local == NULL) return "unknown time";
    char buf[40];
    if (strftime(buf, sizeof(buf), "%d %b %Y, %H:%M", local) == 0) return "unknown time";
    return std::string(buf);
}
