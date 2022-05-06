#include "Ipc_method.h"
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

ssize_t Ipc_method::getFileSize(const std::string &filename) {
	// Use stat to find the size of the file
	struct stat st;
	if (stat(filename.c_str(), &st) == -1) {
		return -1;
	}	
	return st.st_size;
}

int Ipc_method::readFile(const std::string &filename, int offset, std::vector<char> &buffer, int size) {
    std::ifstream in(filename, std::ios::binary);
	if (!in.is_open()) {
		std::cerr << "ifstream::open" << std::endl;
		return -1;
	}
	// move file position according to offset
	in.seekg(offset, std::ios::beg);
	if (!in) {
		std::cerr << "ifstream::seekg" << std::endl;
		return -1;
	}

	in.read(buffer.data(), size);
	// check read status, in.gcount condition set to make last read possible
	if (!in && in.gcount() == size) {
		std::cerr << "ifstream::read" << std::endl;
		return -1;
	}
	in.close();
	return in.gcount();
}

int Ipc_method::readFilePipe(std::ifstream &in, const std::string &filename, std::vector<char> &buffer, int size) {
	if (!in.is_open()) {
		std::cerr << "ifstream::open" << std::endl;
		return -1;
	}

	in.read(buffer.data(), size);
	// check read status, in.gcount condition set to make last read possible
	if (!in && in.gcount() == size) {
		std::cerr << "ifstream::read" << std::endl;
		return -1;
	}
	
	return in.gcount();
}

int Ipc_method::writeFile(const std::string &filename, int total, const std::vector<char> &buffer) {
	std::ofstream out;
	if (total == 0) {
    	out.open(filename, std::ios::binary); // clear the file content if it already exists
	} else {
		out.open(filename, std::ios::binary | std::ios::app);
	}
	if (!out.is_open()) {
		std::cerr << "ofstream::open" << std::endl;
		return -1;
	}

	out.write(buffer.data(), buffer.size());
	// check write status
	if (!out) {
		std::cerr << "ofstream::write" << std::endl;
		return -1;
	}
	out.close();
	return 0;
}

int Ipc_method::writeFilePipe(std::ofstream &out, const std::string &filename, const std::vector<char> &buffer) {
	if (!out.is_open()) {
		std::cerr << "ofstream::open" << std::endl;
		return -1;
	}

	out.write(buffer.data(), buffer.size());
	// check write status
	if (!out) {
		std::cerr << "ofstream::write" << std::endl;
		return -1;
	}
	return 0;
}

void Ipc_method::clearFile(std::string filename) {
	std::ofstream clear(filename, std::ios::binary);
}

std::string Ipc_method::getFileName_absolute(const std::string &filename) {
	auto dir = get_current_dir_name();
	std::string absoluteName(std::string(dir) + "/" + filename);
	if (dir == NULL) {
		throw(std::runtime_error("get_current_dir_name" + std::string(strerror(errno))));
	}
	free(dir);
	return absoluteName;
}