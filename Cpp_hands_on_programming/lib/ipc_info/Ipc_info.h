#ifndef IPC_INFO
#define IPC_INFO

#include <iostream>
#include <string>
#include <memory>
#include "lib/other/Enum.h"
#include <exception>

class Ipc_info {
private:
    std::string filename;
    std::string argument;
    Methods method;
public:
    Ipc_info(std::string f = "", std::string arg = "", Methods m = Methods::NONE) : filename(f), argument(arg), method(m) {}
    Methods getMethod() const;
    std::string getFilename() const;
    std::string getArgument() const;
    void setMethod(Methods m);
    void setFilename(std::string f);
    void setArgument(std::string arg);
};

#endif