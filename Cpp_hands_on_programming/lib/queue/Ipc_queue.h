#ifndef IPC_QUEUE
#define IPC_QUEUE
#include "lib/abstract/Ipc_method.h"
#include "lib/timer/timer.h"
#include <mqueue.h>

class Ipc_queue : public Ipc_method{
private:
    std::string name;
    std::string filename;
    mqd_t msg_queue;
    mq_attr attr; //struct mq_attr
    Timer t;  // class Timer
public:
    Ipc_queue(std::string name0, std::string file0, int timer = 10000);
    virtual ~Ipc_queue();
    void send() override;
    void receive() override;
    void attribute_init(int msgSize, int maxMsg);
};

#endif  