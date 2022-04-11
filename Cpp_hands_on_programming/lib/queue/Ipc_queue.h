#ifndef IPC_QUEUE
#define IPC_QUEUE
#include "lib/abstract/Ipc_method.h"
#include <mqueue.h>

class Ipc_queue : public Ipc_method{
private:
    std::string name;
    std::string filename;
    mqd_t msg_queue;
    mq_attr attr; //struct mq_attr
public:
    Ipc_queue(std::string name0, std::string file0) : name(name0), filename(file0) {}
    virtual ~Ipc_queue();
    void send() override;
    void receive() override;
    void attribute_init(int msgSize, int maxMsg);
    void check_Slash();
};

#endif  