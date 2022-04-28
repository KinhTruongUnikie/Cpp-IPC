#ifndef IPC_PIPE
#define IPC_PIPE
#include "lib/abstract/Ipc_method.h"
#include <chrono>
#include "lib/timer/timer.h"

class Ipc_pipe : public Ipc_method{
private:
    std::string name;
    std::string filename;
    Timer t;
public:
    Ipc_pipe(std::string name0, std::string file0, int timer = 10000) : name(name0), filename(file0), t(timer) {}
    virtual ~Ipc_pipe();
    void send() override;
    void receive() override;
};

#endif 