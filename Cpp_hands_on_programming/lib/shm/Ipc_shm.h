#ifndef IPC_SHM
#define IPC_SHM
#include "lib/abstract/Ipc_method.h"

class Ipc_shm : public Ipc_method{
public:
    virtual ~Ipc_shm() = default;
    void send() override;
    void receive() override;
};

#endif 