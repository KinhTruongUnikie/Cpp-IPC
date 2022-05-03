#include "Ipc_pipe.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <signal.h>

void Ipc_pipe::send() {
    std::cout << "Starting pipeSend.." << std::endl;
	// remove potentially existing FIFO
	unlink(name.c_str());    
	int fdp, bytes_read(0), bytes_write(0);
	off_t size, total(0);
	std::vector<char> buffer(PIPE_BUF);

	// create a name pipe
	if (mkfifo(name.c_str(), 0666) == -1) {
		throw(std::runtime_error("Ipc_pipe::send: mkfifo" + std::string(strerror(errno))));
	}

	// open name pipes
	t.startTimer();
	do {
		errno = 0;
		fdp = open(name.c_str(), O_WRONLY | O_NONBLOCK);
		if (fdp == -1) {
			if (errno == ENXIO) {
				t.checkTimer("Cannot connect to pipeReceive. Errno: " + std::string(strerror(errno)), "Connecting pipeReceive..");
			} else {
				throw(std::runtime_error("Ipc_pipe::send: open pipe. Errno: " + std::string(strerror(errno))));	
			}
		}
	}while (errno == ENXIO);
	
	std::cout << "Connected!" << std::endl;	
	sendHeader(fdp);

	if ((size = getFileSize(filename)) == -1) {
		throw(std::runtime_error("Ipc_method:: getFileSize"));
	}
	// Start read and write loop
	while (total < size) {
		// open and read file 
		if ((bytes_read = readFile(filename, total, buffer, PIPE_BUF)) == -1) {
			throw(std::runtime_error("Ipc_pipe::send: readFile"));
		}
		// write to pipe
		t.startTimer();   
		do {
			errno = 0;
			signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE in case read end is closed 
			bytes_write = write(fdp, buffer.data(), bytes_read);
			if (bytes_write == -1) {
				if (errno == EAGAIN) {
					t.checkTimer("Pipe is full for too long. Errno: " + std::string(strerror(errno)));
				} else {
					throw(std::runtime_error("Ipc_pipe::send: write. Errno: " + std::string(strerror(errno))));
				}
			}
		} while (errno == EAGAIN);
		// compare read and written bytes 
		if (bytes_read == bytes_write) {
			total += bytes_read;
		} else {
			throw(std::runtime_error("Ipc_pipe::send: read write difference"));
		}
	}  
 	close(fdp);
	std::cout << "File delivered by pipe successfully, exiting the program.." << std::endl;
}

void Ipc_pipe::receive() {
    std::cout << "Starting pipeReceive.." << std::endl;
	int fdp, bytes;
	off_t fileSize(0), total(0), sendFileSize(0);
	bool headerSent(false);
	std::vector<char> buffer(PIPE_BUF);
	// open pipe 
	t.startTimer();
	do {
		errno = 0;
		fdp = open(name.c_str(), O_RDONLY | O_NONBLOCK); 
		t.checkTimer("Cannot connect to pipeSend. Errno: " + std::string(strerror(errno)), "Connecting pipeSend..");
	} while(errno == ENOENT);

	std::cout << "Connected!" << std::endl;
	// Start read and write loop
	do {
		t.startTimer();
		do {
			errno = 0;
			bytes = read(fdp, buffer.data(), PIPE_BUF);
			if (bytes == -1) {
				if (errno == EAGAIN) {
					t.checkTimer("Pipe is empty for too long. Errno: " + std::string(strerror(errno)));
				} else {
					throw(std::runtime_error("Ipc_pipe::receive: read. Errno: " + std::string(strerror(errno))));
				}
			}
			else if (!headerSent) {
				if (bytes == 0) {
					t.checkTimer("Takes too long for write end to open");
				} else {
					auto sendFileName = std::string(buffer.begin(), buffer.begin() + bytes);
					if ((sendFileSize = getFileSize(sendFileName)) == -1) {
						throw(std::runtime_error("Ipc_pipe::receive: getFilesize-sendFile. Errno: " + std::string(strerror(errno))));
					}
					headerSent = true;
					total -= bytes;
				}
			}
		} while (errno == EAGAIN || !headerSent);
		// open and write to file
		
		if (total >= 0) {
			buffer.resize(bytes);
			if (writeFile(filename, total, buffer) == -1) {
				throw(std::runtime_error("Ipc_pipe::receive: writeFile"));
			}
		}
		total += bytes;
		
	} while (bytes != 0);

	if ((fileSize = getFileSize(filename)) == -1) {
		throw(std::runtime_error("Ipc_pipe::receive: getFileSize"));
	}
	if (fileSize == sendFileSize) {
		std::cout << "Write to file from pipe successfully, exiting the program.." << std::endl;
	} else {
		throw(std::runtime_error("Ipc_pipe::receive: Received file and send file different"));
	}

	close(fdp);
}

Ipc_pipe::~Ipc_pipe() {
	unlink(name.c_str());
}

void Ipc_pipe::sendHeader(int fd) {
	if (write(fd, filename.c_str(), filename.length()) == -1) {
		throw(std::runtime_error("Ipc_pipe::sendHeader fail!"));
	}
	// wait for receiver to retrieve the header
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}