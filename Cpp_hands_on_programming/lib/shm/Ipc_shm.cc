#include "Ipc_shm.h"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>

void Ipc_shm::send() {
    std::cout << "Starting shmSend.." << std::endl;

    int fd, bytes_read(0);
	off_t total(0), size(0);
	std::vector<char> v(DATA_SIZE);
    
 	if ((size = getFileSize(filename)) == -1) {
		throw(std::runtime_error("Ipc_method:: getFileSize"));
	}

    fd = shm_open(name.c_str(), O_RDWR | O_CREAT, 0666);
	if (fd == -1) { 
		throw(std::runtime_error("Ipc_shm::send: shm_open"));
	}
	// allocate the size of the shared memory object
	if (ftruncate(fd, sizeof(Ipc_shm)) == -1) {
		throw(std::runtime_error("Ipc_shm::send: ftruncate"));	
	}
	// get a pointer to shared memory region
	auto shm_ptr = (Ipc_shm *)mmap(NULL, sizeof(Ipc_shm), PROT_WRITE, MAP_SHARED, fd, 0);
	if (shm_ptr == MAP_FAILED) {
		throw(std::runtime_error("Ipc_shm::send: mmap"));
	}
    //close fd
	close(fd);

	shm_ptr->mutex_init();
	shm_ptr->condvar_init();
	// initialized shared memory object member values
	shm_ptr->mutex_lock();
	shm_ptr->sent = false;
	shm_ptr->end = false;
	shm_ptr->init = true;
	// wait for receiver to connect the shared memory region
	while (shm_ptr->init) {
		std::cout << "Connecting to shmReceive.." << std::endl;
		shm_ptr->condvar_wait();
	}
	shm_ptr->mutex_unlock();
    while (total < size) {
		shm_ptr->mutex_lock();	
		// check sent status and blocked if data still has not been retrieved 
		while (shm_ptr->sent) {
			shm_ptr->condvar_wait();
		}
		//read the file into shared buffer;
		if ((bytes_read = readFile(filename, total, v, DATA_SIZE)) == -1) {
			throw(std::runtime_error("Ipc_shm::send: readFile"));
		}
		copy(v.begin(), v.end(), shm_ptr->buffer);
		total += bytes_read;
		if (total == size) {
			shm_ptr->end = true;
		}
		// update shared memory object
		shm_ptr->sent = true;
		shm_ptr->data_size = bytes_read; 

		shm_ptr->mutex_unlock();
		shm_ptr->condvar_broadcast();
	}
	
	if (total == size) {
		std::cout << "File delivered by shm successfully, exiting the program.."  << std::endl;
	} else {
		throw(std::runtime_error("Ipc_shm::send: total and size difference"));
	}
}

void Ipc_shm::receive() {
    std::cout << "Starting shmReceive.." << std::endl;

	auto shm_ptr = get_shared_memory_pointer(name);
	std::cout << "Found shm!"<< std::endl;
    off_t total(0), size(0);
    bool done(false);
	std::vector<char> v(DATA_SIZE);
	while (!done) {
		// enter critical section, lock the mutex
		shm_ptr->mutex_lock();
		// check sent status and blocked if shmSend still has not sent new data at this point
		while (!shm_ptr->sent) {
			shm_ptr->condvar_wait();
		}
		// check end status
		if (shm_ptr->end) {
			done = shm_ptr->end;
		}
		// write into file
		copy(shm_ptr->buffer, shm_ptr->buffer + shm_ptr->data_size, v.begin());
		v.resize(shm_ptr->data_size);
		
		if (writeFile(filename, total, v) == -1) {
			throw("Ipc_shm::receive: writeFile");
		}
        total += shm_ptr->data_size;
		// reset sent status
		shm_ptr->sent = false;
		shm_ptr->mutex_unlock();
		shm_ptr->condvar_broadcast();
	}
	// get size of newly written file name
	if ((size = getFileSize(filename)) == -1) {
		throw(std::runtime_error("Ipc_method:: getFileSize"));
	}
	if (total == size) {
		std::cout << "Write to file from shm successfully, exiting the program.." << std::endl;
	} else {
		throw(std::runtime_error("Ipc_shm::receive: file size and total difference"));
	}
}

Ipc_shm::~Ipc_shm() {
    shm_unlink(name.c_str());
}

Ipc_shm * Ipc_shm::get_shared_memory_pointer(std::string name) {
	Ipc_shm * ptr = nullptr;
	int fd;

	t.startTimer();
	while ((fd = shm_open(name.c_str(), O_RDWR, 0)) == -1) {
		/* wait one second then try again */
		t.checkTimer("Shm region does not exist", "Connecting to shmSend..");
	}

	while((ptr = (Ipc_shm *)mmap(0, sizeof(Ipc_shm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		t.checkTimer("Shm region does not exist", "Connecting to shmSend..");
	}
	/* no longer need fd */
	close(fd);

	while (!ptr->init) {
		t.checkTimer("Shm region does not exist", "Connecting to shmSend..");
	}
	ptr->mutex_lock();
	ptr->init = false;
	ptr->mutex_unlock();
	ptr->condvar_broadcast();
	return ptr;
}

void Ipc_shm::mutex_init() {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	if (pthread_mutex_init(&mutex, &attr) != 0) {
		throw(std::runtime_error("pthread_mutex_init. Errno: " + std::string(strerror(errno))));
	}
}

void Ipc_shm::mutex_lock() {
	if ((returnErrno = pthread_mutex_timedlock(&mutex, &t.useTimespec())) != 0) {
		throw(std::runtime_error("pthread_mutex_timedlock. Errno: " + std::string(strerror(returnErrno))));
	}
}

void Ipc_shm::mutex_unlock() {
	if (pthread_mutex_unlock(&mutex) != 0) {
		throw(std::runtime_error("pthread_mutex_unlock. Errno: " + std::string(strerror(errno))));
	}
}

void Ipc_shm::condvar_init() {
	pthread_condattr_t attr;
	pthread_condattr_init(&attr);
	pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	if (pthread_cond_init(&cond, &attr) != 0) {
		throw(std::runtime_error("pthread_cond_init. Errno: " + std::string(strerror(errno))));
	}
}

void Ipc_shm::condvar_wait() {
	if ((returnErrno = pthread_cond_timedwait(&cond, &mutex, &t.useTimespec())) != 0) {
		throw(std::runtime_error("pthread_cond_timedwait. Errno: " + std::string(strerror(returnErrno))));
	}
}

void Ipc_shm::condvar_broadcast() {
	if (pthread_cond_broadcast(&cond) != 0) {
		throw(std::runtime_error("pthread_cond_broadcast"));	
	}
}

Ipc_shm::Ipc_shm(std::string name0, std::string file0, int time) {
    if (name0[0] != '/' || name0.find('/', 1) != std::string::npos || name0.length() <= 1) {
        throw(std::runtime_error("Ipc_shm::Constructor: shm name must start with leading slash '/' followed by non-slash characters"));
    }
    name = name0;
    filename = file0;
	t = Timer(time);
}