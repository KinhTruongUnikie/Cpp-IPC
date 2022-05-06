#ifndef CAPTURE_STDOUT
#define CAPTURE_STDOUT  

#include <iostream>
#include <sstream>
class CaptureStdOut {
private:
    std::streambuf *cout_stream_buff;
    std::stringstream strm;
public:
    CaptureStdOut() {
        cout_stream_buff = std::cout.rdbuf();
        std::cout.rdbuf(strm.rdbuf());
    }
    std::string getString() {
        return strm.str();
    }
    ~CaptureStdOut() {
        std::cout.rdbuf(cout_stream_buff);
    }
};

#endif