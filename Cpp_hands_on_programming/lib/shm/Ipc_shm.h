#ifndef IPC_SHM
#define IPC_SHM
#include "lib/abstract/Ipc_method.h"
#include "lib/timer/timer.h"
#include <pthread.h>
#

class Ipc_shm : public Ipc_method {
private:
    std::string name;
    std::string filename;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char buffer[DATA_SIZE];
    int data_size;
    bool init;
    bool sent;
    bool end;
    Timer t;
    int returnErrno;
public:
    Ipc_shm(std::string name0, std::string file0, int timer = 10000);
    virtual ~Ipc_shm();
    void send() override;
    void receive() override;
    Ipc_shm * get_shared_memory_pointer(std::string name);
    void mutex_init();
    void mutex_lock();
    void mutex_unlock();
    void condvar_init();
    void condvar_wait();
    void condvar_broadcast();
};

#endif 