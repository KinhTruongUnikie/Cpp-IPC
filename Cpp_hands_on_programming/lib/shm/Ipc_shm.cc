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
	ssize_t total(0), size(0);
	std::vector<char> v(DATA_SIZE);
	std::ifstream in(filename, std::ios::binary);
    
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
	shm_ptr->t = t;
	// initialized shared memory object member values
	shm_ptr->mutex_lock();
	shm_ptr->sent = false;
	shm_ptr->end = false;
	auto header = getFileName_absolute(filename);
	copy(header.begin(), header.end(), shm_ptr->sendFileName);
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
		if ((bytes_read = readFile(in, filename, v, DATA_SIZE)) == -1) {
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
	in.close();
	if (total == size) {
		std::cout << "File delivered by shm successfully, exiting the program.."  << std::endl;
	} else {
		throw(std::runtime_error("Ipc_shm::send: total and size difference"));
	}
}

void Ipc_shm::receive() {
    std::cout << "Starting shmReceive.." << std::endl;
	std::ofstream out(filename, std::ios::binary | std::ios::app);
	auto shm_ptr = get_shared_memory_pointer(name);
	std::cout << "Found shm!"<< std::endl;
    ssize_t total(0), size(0), sendFileSize(0);
    bool done(false);
	std::vector<char> v(DATA_SIZE);
	shm_ptr->mutex_lock();
	if ((sendFileSize = getFileSize(shm_ptr->sendFileName)) == -1) {
		throw(std::runtime_error("Ipc_method:: getFileSizeNew"));
	}
	shm_ptr->mutex_unlock();
	clearFile(filename);
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
		
		if (writeFile(out, filename, v) == -1) {
			throw("Ipc_shm::receive: writeFile");
		}
        total += shm_ptr->data_size;
		// reset sent status
		shm_ptr->sent = false;
		shm_ptr->mutex_unlock();
		shm_ptr->condvar_broadcast();
	}
	out.close();
	// get size of newly written file name
	if ((size = getFileSize(filename)) == -1) {
		throw(std::runtime_error("Ipc_method:: getFileSize"));
	}
	
	if (sendFileSize == size) {
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
	if (getFileName_absolute(filename) == ptr->sendFileName) {
		throw(std::runtime_error("Received file and send file have to have different names"));
	}
	ptr->init = false;
	ptr->mutex_unlock();
	ptr->condvar_broadcast();
	return ptr;
}

void Ipc_shm::mutex_init() {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	if ((returnErrno = pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST)) != 0) {
		throw(std::runtime_error("pthread_mutex_init. Errno: " + std::string(strerror(returnErrno))));
	}
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	if ((returnErrno = pthread_mutex_init(&mutex, &attr)) != 0) {
		throw(std::runtime_error("pthread_mutex_init. Errno: " + std::string(strerror(returnErrno))));
	}
}

void Ipc_shm::mutex_lock() {
	if ((returnErrno = pthread_mutex_timedlock(&mutex, &t.useTimespec())) != 0) {
		throw(std::runtime_error("pthread_mutex_timedlock. Errno: " + std::string(strerror(returnErrno))));
	}
}

void Ipc_shm::mutex_unlock() {
	if ((returnErrno = pthread_mutex_unlock(&mutex)) != 0) {
		throw(std::runtime_error("pthread_mutex_unlock. Errno: " + std::string(strerror(returnErrno))));
	}
}

void Ipc_shm::condvar_init() {
	pthread_condattr_t attr;
	pthread_condattr_init(&attr);
	pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	if ((returnErrno = pthread_cond_init(&cond, &attr)) != 0) {
		throw(std::runtime_error("pthread_cond_init. Errno: " + std::string(strerror(returnErrno))));
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

Ipc_shm::Ipc_shm(std::string name0, std::string file0, int timer) {
    if (name0[0] != '/' || name0.find('/', 1) != std::string::npos || name0.length() <= 1) {
        throw(std::runtime_error("Ipc_shm::Constructor: shm name must start with leading slash '/' followed by non-slash characters"));
    }
    name = name0;
    filename = file0;
	t = timer;
}