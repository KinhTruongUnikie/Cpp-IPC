#include "Ipc_queue.h"
#include <iostream>
#include <cstring>


// define static member variable

void Ipc_queue::send()
{
    std::cout << "Starting queueSend.." << std::endl;
    #if 0
    check_leadingSlash();
    
    std::vector<char> buffer(DATA_SIZE);
    int size, total(0), bytes;
    std::string filesize;

   	if ((size = getFileSize(filename)) == -1) {
		throw(std::runtime_error("Ipc_method:: getFileSize"));
	}
    // initialize queue attributes
    attribute_init(DATA_SIZE, 10); // max size 8k, max msg 10

    // open create msg queue
    msg_queue = mq_open(name.c_str(), O_WRONLY | O_CREAT, 0666, &attr);
    if (msg_queue == -1)
    {
        throw(std::runtime_error("Ipc_queue::send: mq_open"));
    }
    filesize = std::to_string(size);
    // send first message(file size) to client
    mq_send(msg_queue, filesize.c_str(), filesize.size(), 0);
    // Start read send loop
    while (total < size)
    {
        if ((bytes = readFile(filename, total, buffer, DATA_SIZE)) == -1)
        {
            throw(std::runtime_error("Ipc_queue::send: readFile"));
        }
        
        if (mq_send(msg_queue, &buffer[0], bytes, 0) == -1) {
            throw(std::runtime_error("Ipc_queue::send: mq_send"));
        }
        
        total += bytes;
        // if (total == size) {
        //     mq_send(msg_queue, &buffer[0], 0, 0);
        // }

        // if (bytes != DATA_SIZE) {
        //     mq_attr att;
        //     attr.mq_msgsize = bytes;
        //     std::cout << attr.mq_msgsize << std::endl;
        //     mq_getattr(msg_queue, &att);
        //     std::cout << att.mq_msgsize << std::endl;

        // } //THE ABOVE CODE DOES NOT CHANGE THE MSGSIZE VALUE RETRIEVED BY MQ_GETATTR
    }

    if (total == size) {
        std::cout << "Data successfully delivered by queue" << std::endl;
    } else {
        throw(std::runtime_error("Ipc_queue::send: total bytes sent and file size different"));
    }
    mq_close(msg_queue);
    #endif
}

void Ipc_queue::receive()
{
    std::cout << "Starting queueReceive.." << std::endl;
    #if 0
    check_leadingSlash();

    std::vector<char> buffer;
    int total(0), bytes, size(0), newFileSize;
    // open msg queue loop
	while ((msg_queue = mq_open(name.c_str(), O_RDONLY)) == -1) {
		std::this_thread::sleep_for (std::chrono::seconds(1));
	}
    
    // get queue attr 
    mq_getattr(msg_queue, &attr);
    // initialize buffer size
    buffer.resize(attr.mq_msgsize);
    // get file size
    mq_receive(msg_queue, &buffer[0], attr.mq_msgsize, 0);
    std::string filesize(&buffer[0]);
    size = stoi(filesize);

    // Start receive write loop 
    do {
        if ((bytes = mq_receive(msg_queue, &buffer[0], attr.mq_msgsize, 0)) == -1) {
            throw(std::runtime_error("Ipc_queue::receive: mq_receive"));
        }
        if (writeFile(filename, total, buffer, bytes) == -1) {
            throw(std::runtime_error("Ipc_queue::receive: writeFile"));
        }
        total += bytes;
    } while (total < size);
    // remove the queue name
    if ((newFileSize = getFileSize(filename)) == -1) {
		throw(std::runtime_error("Ipc_method:: getFileSize"));
	}
    if (newFileSize == size) {
        std::cout << "File is successfully transferred" << std::endl;
    } else {
        throw(std::runtime_error("Ipc_queue::receive: File sizes difference between sent file and receive file"));
    }
    mq_unlink(name.c_str());
    mq_close(msg_queue);
    #endif
}   

#if 1
void Ipc_queue::attribute_init(int msgSize, int maxMsg)
{
    //memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = maxMsg;
    attr.mq_msgsize = msgSize;
}

Ipc_queue::~Ipc_queue() {
    mq_unlink(name.c_str());
}

void Ipc_queue::check_leadingSlash() {
    if (name[0] != '/') {
        throw(std::runtime_error("Ipc_queue::check_leadingSlash: queue name must starts with leading slash '/'"));
    }
}
#endif