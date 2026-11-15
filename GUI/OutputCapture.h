#ifndef OUTPUTCAPTURE_H
#define OUTPUTCAPTURE_H

#include <iostream>
#include <streambuf>
#include <string>

class StringBuffer : public std::streambuf {
public:
    std::string text;

protected:
    int_type overflow(int_type c) override {
        if (c != traits_type::eof()) text += static_cast<char>(c);
        return c;
    }

    std::streamsize xsputn(const char* s, std::streamsize n) override {
        text.append(s, static_cast<size_t>(n));
        return n;
    }
};

class OutputCapture {
private:
    StringBuffer buffer;
    std::streambuf* previous;

public:
    OutputCapture() : previous(std::cout.rdbuf(&buffer)) {}
    ~OutputCapture() { std::cout.rdbuf(previous); }
    std::string text() const { return buffer.text; }
};

#endif
