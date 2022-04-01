#ifndef PROG_INIT
#define PROG_INIT

#include "lib/ipc_info/Ipc_info.h"

class Prog_init {
public:
    void printInstruction();
    std::shared_ptr<Ipc_info> checkOptions (int argc, char** argv);
    void run_IPC(std::shared_ptr<Ipc_info> info,  send_or_receive send);
};

#endif