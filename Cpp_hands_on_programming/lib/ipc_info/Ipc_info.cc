#include "Ipc_info.h"

Methods Ipc_info::getMethod() const {
    return method;
}

std::string Ipc_info::getArgument() const {
    return argument;
}

std::string Ipc_info::getFilename() const {
    return filename;
}

void Ipc_info::setMethod(Methods m) {
    method = m;
}

void Ipc_info::setFilename(std::string f) {
    filename = f;
}
void Ipc_info::setArgument(std::string arg) {
    argument = arg;
}

