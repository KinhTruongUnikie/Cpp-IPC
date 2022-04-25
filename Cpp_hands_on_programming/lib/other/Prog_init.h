#ifndef PROG_INIT
#define PROG_INIT

#include "lib/ipc_info/Ipc_info.h"

class Prog_init {
private:
    Ipc_info info;
public:
    void printInstruction();
    Ipc_info & checkOptions (int argc, char** argv);
    void run_IPC(Send_or_receive side);
    bool fileExist();
    bool overwritable();
};
    
#endif