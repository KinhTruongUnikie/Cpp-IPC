#include "Ipc_queue.h"
#include <iostream>
#include <cstring>


// define static member variable

void Ipc_queue::send()
{
    std::cout << "Starting queueSend.." << std::endl;
    
    std::vector<char> buffer(DATA_SIZE);
    int bytes;
    ssize_t size(0), total(0);
    bool connected(false);
    std::ifstream in(filename, std::ios::binary);

   	if ((size = getFileSize(filename)) == -1) {
		throw(std::runtime_error("Ipc_method::send: getFileS ize"));
	}
    // initialize queue attributes
    attribute_init(DATA_SIZE, 10);

    // open create msg queue
    msg_queue = mq_open(name.c_str(), O_WRONLY | O_CREAT, 0666, &attr);
    if (msg_queue == -1)
    {
        throw(std::runtime_error("Ipc_queue::send: mq_open"));
    }
  
    // Start read send loop
    while (total < size)
    {
        if ((bytes = readFile(in, filename, buffer, DATA_SIZE)) == -1) {
            throw(std::runtime_error("Ipc_queue::send: readFile"));
        }

        if (!connected){
            auto header = getFileName_absolute(filename);
            if (mq_send(msg_queue, header.c_str(), header.length(), 0) == -1) {
                throw(std::runtime_error("Ipc_queue::send: mq_send. Errno: " + std::string(strerror(errno))));
            }
            t.startTimer();
            do {
                t.checkTimer("Cannot connect to queueReceive", "Connecting queueReceive..");
                mq_getattr(msg_queue, &attr);
            } while (attr.mq_curmsgs == 1);
            connected = true; 
        }

        if (mq_timedsend(msg_queue, buffer.data(), bytes, 0, &t.useTimespec()) == -1) {
            throw(std::runtime_error("Ipc_queue::send: mq_timedsend. Errno: " + std::string(strerror(errno))));
        }
        
        total += bytes;
        
        if (total == size) {
            if (mq_timedsend(msg_queue, buffer.data(), 0, 0, &t.useTimespec()) == -1) {
                throw(std::runtime_error("Ipc_queue::send: mq_send 0 byte message. Errno: " + std::string(strerror(errno))));
            }
        }
    }
    in.close();
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
    ssize_t size(0), total(0), sendFileSize(0);
    std::string sendFileName;
    bool headerSent(false);
    std::ofstream out(filename, std::ios::binary | std::ios::app);
    // open msg queue loop
    t.startTimer();
	do {
		errno = 0;
		msg_queue = mq_open(name.c_str(), O_RDONLY);
		t.checkTimer("Cannot connect to queueSend. Errno: " + std::string(strerror(errno)), "Connecting queueSend..");
	} while(errno == ENOENT);
    // get queue attr 
    mq_getattr(msg_queue, &attr);
    // initialize buffer size
    buffer.resize(attr.mq_msgsize);
    // Start receive write loop 
    do {
        if ((bytes = mq_timedreceive(msg_queue, buffer.data(), attr.mq_msgsize, 0, &t.useTimespec())) == -1) {
            throw(std::runtime_error("Ipc_queue::receive: mq_timedreceive. Errno: " + std::string(strerror(errno))));
        }

        if (!headerSent) {
  			sendFileName = std::string(buffer.begin(), buffer.begin() + bytes);
			if ((sendFileSize = getFileSize(sendFileName)) == -1) {
				throw(std::runtime_error("Ipc_queue::receive: getFilesize-sendFile. Errno: " + std::string(strerror(errno))));
			}
            if (sendFileName == getFileName_absolute(filename)) {
                throw(std::runtime_error("Ipc_queue::receive: Received file and send file have to have different names"));
            }
            clearFile(filename);
			headerSent = true;
            bytes = -1;
        }

        if (bytes != -1) {
            buffer.resize(bytes);
            if (writeFile(out, filename, buffer) == -1) {
                throw(std::runtime_error("Ipc_queue::receive: writeFile"));
            }
            total += bytes;
        }
    } while (bytes != 0);
    out.close();
    // get newly written file size
    if ((size = getFileSize(filename)) == -1) {
		throw(std::runtime_error("Ipc_method::receive: getFileSize"));
	}
    if (size == sendFileSize) {
        std::cout << "Write to file from queue successfully, exiting the program.."  << std::endl;
    } else {
        throw(std::runtime_error("Ipc_queue::receive: File sizes difference between sent file and receive file"));
    }
}   

Ipc_queue::Ipc_queue(std::string name0, std::string file0, int time) {
    if (name0[0] != '/' || name0.find('/', 1) != std::string::npos || name0.length() <= 1) {
        throw(std::runtime_error("Ipc_queue::Constructor: queue name must starts with leading slash '/' and followed by non-slash characters"));
    }   
    name = name0;
    filename = file0;
    t = Timer(time);
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