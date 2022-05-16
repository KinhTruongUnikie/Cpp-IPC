#ifndef IPC_METHOD
#define IPC_METHOD

#include "lib/ipc_info/Ipc_info.h"
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>

class Ipc_method{
public:
    virtual void send() = 0;
    virtual void receive() = 0;
    ssize_t getFileSize(const std::string &filename);
    //int readFile(const std::string &filename, int offset, std::vector<char> &buffer, int size);
    int readFile(std::ifstream &in, const std::string &filename, std::vector<char> &buffer, int size);
    //int writeFile(const std::string &filename, int total, const std::vector<char> &buffer);
    int writeFile(std::ofstream &out, const std::string &filename, const std::vector<char> &buffer);
    void clearFile(std::string filename);
    std::string getFileName_absolute(const std::string &filename);
    virtual ~Ipc_method() = default;
};

#endif