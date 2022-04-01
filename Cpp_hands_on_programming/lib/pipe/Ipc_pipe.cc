#include "Ipc_pipe.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

void Ipc_pipe::send() {
    std::cout << "Starting pipeSend.." << std::endl;
	#if 1
	// remove potentially existing FIFO
	remove(name.c_str());    
	int fdp, size, total(0), bytes_read(0), bytes_write(0);
	std::vector<char> buffer(PIPE_BUF);
	std::ifstream in;

	// create a name pipe
	if (mkfifo(name.c_str(), 0666) == -1) {
		perror("mkfifo");
		exit(EXIT_FAILURE);
	}

	// open name pipe
	fdp = open(name.c_str(), O_WRONLY);
	if (fdp == -1) {
		throw(std::runtime_error("Ipc_pipe::send: open pipe"));
	}

	size = getFileSize(filename);
	// Start read and write loop
	while (total < size) {
		// open and read file 
		if ((bytes_read = readFile(filename, total, buffer, PIPE_BUF)) == -1) {
			throw(std::runtime_error("Ipc_pipe::send: readFile"));
		}
		// write to pipe
		if ((bytes_write = write(fdp, &buffer[0], bytes_read)) == -1) {
	    	throw(std::runtime_error("Ipc_pipe::send: write to pipe"));
		}
		// compare read and written bytes 
		if (bytes_read == bytes_write) {
			total += bytes_read;
		} else {
			throw(std::runtime_error("Ipc_pipe::send: read write difference"));
		}
	}

	in.close();
	close(fdp);
	#endif
	std::cout << "File delivered by pipe successfully, exiting the program.." << std::endl;
}

void Ipc_pipe::receive() {
    std::cout << "Starting pipeReceive.." << std::endl;
	int fdp, bytes, fileSize(0), total(0);
	std::vector<char> buffer(PIPE_BUF);
	// open pipe 
	while ((fdp = open(name.c_str(), O_RDONLY)) == -1) {
		std::this_thread::sleep_for (std::chrono::seconds(1));
	}
	// Start read and write loop
	while ((bytes = read(fdp, &buffer[0], PIPE_BUF)) != 0) {	
		if (bytes == -1) {
			throw(std::runtime_error("Ipc_pipe::receive: read"));
		}
		// open and write to file
		if (writeFile(filename, total, buffer, bytes) == -1) {
			throw(std::runtime_error("Ipc_pipe::receive: writeFile"));
		}
		total += bytes;
	}
	
	fileSize = getFileSize(filename);
	if (total == fileSize) {
		std::cout << "Write to file from pipe successfully, exiting the program.." << std::endl;
	} else {
		throw(std::runtime_error("Ipc_pipe::receive: fileSize and total read difference"));
	}
	
	close(fdp);
	//server remove the FIFO after data is written into file
	remove(name.c_str());
}

Ipc_pipe::~Ipc_pipe() {
	remove(name.c_str());
}