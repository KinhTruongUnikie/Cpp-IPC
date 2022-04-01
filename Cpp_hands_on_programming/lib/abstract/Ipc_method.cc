#include "Ipc_method.h"
#include <iostream>

int Ipc_method::getFileSize(std::string filename) {
    int size;
    std::ifstream in(filename, std::ios::binary);
	if (!in) {
		perror("getFileSize");
		exit(EXIT_FAILURE);
	}
	in.seekg(0, std::ios::end);
	size = in.tellg();
    in.close();
    return size;
}

int Ipc_method::readFile(std::string filename, int offset, std::vector<char> &buffer, int size) {
    std::ifstream in(filename, std::ios::binary);
	if (!in.is_open()) {
		perror("ifstream::open\n");
		return -1;
	}
	// move file position according to offset
	in.seekg(offset, std::ios::beg);
	if (!in) {
		perror("seekg\n");
		return -1;
	}

	in.read(&buffer[0], size);
	// check read status, in.gcount condition set to make last read possible
	if (!in && in.gcount() == size) {
		perror("ifstream::read fail\n");
		return -1;
	}
	in.close();
	return in.gcount();
}

int Ipc_method::writeFile(std::string filename, int total, std::vector<char> &buffer, int size) {
	std::ofstream out;
	if (total == 0) {
    	out.open(filename, std::ios::binary); // clear the file content if it already exists
	} else {
		out.open(filename, std::ios::binary | std::ios::app);
	}
	if (!out.is_open()) {
		perror("ofstream::open\n");
		return -1;
	}

	out.write(&buffer[0], size);
	// check write status
	if (!out) {
		perror("ofstream::write fail\n");
		return -1;
	}
	out.close();
	return 0;
}