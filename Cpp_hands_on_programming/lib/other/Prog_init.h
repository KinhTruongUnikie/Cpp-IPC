#ifndef PROG_INIT
#define PROG_INIT

#include "lib/ipc_info/Ipc_info.h"

class Prog_init {
public:
    void printInstruction();
    Ipc_info checkOptions (int argc, char** argv);
    void run_IPC(const Ipc_info &info,  Send_or_receive side);
};
    
#endif