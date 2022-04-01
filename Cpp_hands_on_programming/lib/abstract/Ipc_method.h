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
    int getFileSize(std::string filename);
    int readFile(std::string filename, int offset, std::vector<char> &buffer, int size);
    int writeFile(std::string filename, int total, std::vector<char> &buffer, int size);
    virtual ~Ipc_method() = default;
};

#endif