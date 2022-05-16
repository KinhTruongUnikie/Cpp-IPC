#include "Ipc_method.h"
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <cstring>

ssize_t Ipc_method::getFileSize(const std::string &filename) {
	// Use stat to find the size of the file
	struct stat st;
	if (stat(filename.c_str(), &st) == -1) {
		return -1;
	}	
	return st.st_size;
}

int Ipc_method::readFile(std::ifstream &in, const std::string &filename, std::vector<char> &buffer, int size) {
	if (!in.is_open()) {
		std::cerr << "ifstream::open" << std::endl;
		return -1;
	}

	in.read(buffer.data(), size);
	// check read status, in.gcount condition set to make last read possible
	if ((!in && in.gcount() == size) || in.gcount() == 0)  {
		std::cerr << "ifstream::read" << std::endl;
		return -1;
	}
	
	return in.gcount();
}

int Ipc_method::writeFile(std::ofstream &out, const std::string &filename, const std::vector<char> &buffer) {
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
	char absolute[PATH_MAX];
	if (realpath(filename.c_str(), absolute) == NULL) {
		throw(std::runtime_error("getFileName_absolute: realpath(). Errno: " + std::string(strerror(errno))));
	}
	return std::string(absolute);
}