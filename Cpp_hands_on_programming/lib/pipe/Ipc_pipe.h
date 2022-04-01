#ifndef IPC_PIPE
#define IPC_PIPE
#include "lib/abstract/Ipc_method.h"

class Ipc_pipe : public Ipc_method{
private:
    std::string name;
    std::string filename;
public:
    Ipc_pipe(std::string name0 = "", std::string file0 = "") : name(name0), filename(file0) {}
    virtual ~Ipc_pipe();
    void send() override;
    void receive() override;
};

#endif 