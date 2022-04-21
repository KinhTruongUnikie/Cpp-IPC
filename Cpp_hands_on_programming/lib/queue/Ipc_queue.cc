#include "Ipc_queue.h"
#include <iostream>
#include <cstring>


// define static member variable

void Ipc_queue::send()
{
    std::cout << "Starting queueSend.." << std::endl;
    
    std::vector<char> buffer(DATA_SIZE);
    int bytes;
    off_t size(0), total(0);

   	if ((size = getFileSize(filename)) == -1) {
		throw(std::runtime_error("Ipc_method::send: getFileSize"));
	}
    // initialize queue attributes
    attribute_init(DATA_SIZE, 10); // default max size 8192, max msg 10

    // open create msg queue
    msg_queue = mq_open(name.c_str(), O_WRONLY | O_CREAT, 0666, &attr);
    if (msg_queue == -1)
    {
        throw(std::runtime_error("Ipc_queue::send: mq_open"));
    }

    // Start read send loop
    while (total < size)
    {
        if ((bytes = readFile(filename, total, buffer, DATA_SIZE)) == -1)
        {
            throw(std::runtime_error("Ipc_queue::send: readFile"));
        }
        
        if (mq_send(msg_queue, buffer.data(), bytes, 0) == -1) {
            throw(std::runtime_error("Ipc_queue::send: mq_send"));
        }
        
        total += bytes;
        if (total == size) {
            if (mq_send(msg_queue, buffer.data(), 0, 0) == -1) {
                throw(std::runtime_error("Ipc_queue::send: mq_send 0 byte message"));
            }
        }
    }

    if (total == size) {
        std::cout << "File delivered by queue successfully, exiting the program.."  << std::endl;
    } else {
        throw(std::runtime_error("Ipc_queue::send: total bytes sent and file size different"));
    }
}

void Ipc_queue::receive()
{
    std::cout << "Starting queueReceive.." << std::endl;

    std::vector<char> buffer;
    int bytes;
    off_t size(0), total(0);
    // open msg queue loop
	while ((msg_queue = mq_open(name.c_str(), O_RDONLY)) == -1) {
		std::this_thread::sleep_for (std::chrono::seconds(1));
	}
    
    // get queue attr 
    mq_getattr(msg_queue, &attr);
    // initialize buffer size
    buffer.resize(attr.mq_msgsize);

    // Start receive write loop 
    do {
        if ((bytes = mq_receive(msg_queue, buffer.data(), attr.mq_msgsize, 0)) == -1) {
            throw(std::runtime_error("Ipc_queue::receive: mq_receive"));
        }
        buffer.resize(bytes);
        if (writeFile(filename, total, buffer) == -1) {
            throw(std::runtime_error("Ipc_queue::receive: writeFile"));
        }
        total += bytes;
    } while (bytes != 0);
    // get newly written file size
    if ((size = getFileSize(filename)) == -1) {
		throw(std::runtime_error("Ipc_method::receive: getFileSize"));
	}
    if (size == total) {
        std::cout << "Write to file from queue successfully, exiting the program.."  << std::endl;
    } else {
        throw(std::runtime_error("Ipc_queue::receive: File sizes difference between sent file and receive file"));
    }
}   

Ipc_queue::Ipc_queue(std::string name0, std::string file0) {
    if (name0[0] != '/' || name0.find('/', 1) != std::string::npos || name0.length() <= 1) {
        throw(std::runtime_error("Ipc_queue::Constructor: queue name must starts with leading slash '/' and followed by non-slash characters"));
    }
    name = name0;
    filename = file0;
}

void Ipc_queue::attribute_init(int msgSize, int maxMsg)
{
    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = maxMsg;
    attr.mq_msgsize = msgSize;
}

Ipc_queue::~Ipc_queue() {
    mq_close(msg_queue);
    mq_unlink(name.c_str());
}