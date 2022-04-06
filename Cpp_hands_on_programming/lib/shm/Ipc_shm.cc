#include "Ipc_shm.h"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <algorithm>

void Ipc_shm::send() {
    std::cout << "Starting shmSend.." << std::endl;
	#if 0
    check_leadingSlash();

    int fd, total(0), size, bytes_read(0);
	std::vector<char> v(DATA_SIZE);

    //Ipc_shm shm_t(name, filename);
    auto shm_ptr = this;
    
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
	shm_ptr = (Ipc_shm *)mmap(NULL, sizeof(Ipc_shm), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (shm_ptr == MAP_FAILED) {
		throw(std::runtime_error("Ipc_shm::send: mmap"));
	}
    //close fd
	close(fd);
	
	if ((bytes_read = readFile(filename, total, v, DATA_SIZE) == -1)) {
		throw(std::runtime_error("Ipc_shm::send: readFile"));
	}
	std::cout << bytes_read << std::endl;
	shm_ptr->mutex_init();
	shm_ptr->condvar_init();
	// initialized shared memory object member values
	shm_ptr->sent = false;
	shm_ptr->end = false;
	shm_ptr->init = true;

    while (total < size) {
        // shm_ptr->m.lock();
		shm_ptr->mutex_lock();
		// check sent status and blocked if data still has not been retrieved 
		while (shm_ptr->sent) {
            //std::unique_lock<std::mutex> ul(shm_ptr->m);
			//shm_ptr->cv.wait(ul);
			shm_ptr->condvar_wait();
		}
		//read the file into shared buffer;
		if ((bytes_read = readFile(filename, total, v, DATA_SIZE) == -1)) {
			throw(std::runtime_error("Ipc_shm::send: readFile"));
		}
		std::cout << bytes_read << std::endl;
		//copy(v.begin(), v.end(), buffer);
		total += bytes_read;
		if (total == size) {
			shm_ptr->end = true;
		}
		// update shared memory object
		shm_ptr->sent = true;
		shm_ptr->data_size = bytes_read; // actually bytes read is different with the default DATA_SIZE

		shm_ptr->mutex_unlock();
		//wake up process which is currently condvar blocked
		shm_ptr->condvar_broadcast();
	}
	#endif
}

void Ipc_shm::receive() {
    std::cout << "Starting shmReceive.." << std::endl;
	#if 0
    check_leadingSlash();

    auto shm_ptr = get_shared_memory_pointer(name);
	std::cout << "Found shm!"<< std::endl;
    int total(0);
    bool done(false);
	std::vector<char> v(DATA_SIZE);
	while (!done) {
		// enter critical section, lock the mutex
		shm_ptr->mutex_lock();
		if (shm_ptr->end) {
			done = shm_ptr->end;
		}
		// check sent status and blocked if shmSend still has not sent new data at this point
		while (!shm_ptr->sent) {
            //std::unique_lock<std::mutex> ul(shm_ptr->m);
			//shm_ptr->cv.wait(ul);
			shm_ptr->condvar_wait();
		}
		//v.resize()
		// write into file
		std::copy(shm_ptr->buffer, shm_ptr->buffer + DATA_SIZE, v.begin());

		if (writeFile(filename, total, v, shm_ptr->data_size) == -1) {
			throw("Ipc_shm::receive: writeFile");
		}
        total += shm_ptr->data_size;
		// reset sent status
		shm_ptr->sent = false;
		// unlock the mutex
		shm_ptr->mutex_unlock();
		//wake up process which is currently condvar blocked
		shm_ptr->condvar_broadcast();
	}
	#endif
}

void Ipc_shm::check_leadingSlash() {
    if (name[0] != '/') {
        throw(std::runtime_error("Ipc_queue::check_leadingSlash: queue name must starts with leading slash '/'"));
    }
}

Ipc_shm::~Ipc_shm() {
    shm_unlink(name.c_str());
}

Ipc_shm * Ipc_shm::get_shared_memory_pointer(std::string name) {
	Ipc_shm * ptr;
	int fd;

	std::cout << "Getting access to shared memory region..(Program would stuck in a loop for non-exist shm name" << std::endl;

	while ((fd = shm_open(name.c_str(), O_RDWR, 0)) == -1) {
		/* wait one second then try again */
		std::this_thread::sleep_for (std::chrono::seconds(1));
	}

	while ((ptr = (Ipc_shm *)mmap(0, sizeof(Ipc_shm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		std::this_thread::sleep_for (std::chrono::seconds(1));
	}
	/* no longer need fd */
	close(fd);

	while (!ptr->init) {
		std::this_thread::sleep_for (std::chrono::seconds(1));
	}
	return ptr;
}

void Ipc_shm::mutex_init() {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	if (pthread_mutex_init(&mutex, &attr) != 0) {
		throw(std::runtime_error("pthread_mutex_init"));
	}
}

void Ipc_shm::mutex_lock() {
	if (pthread_mutex_lock(&mutex) != 0) {
		throw(std::runtime_error("pthread_mutex_lock"));
	}
}

void Ipc_shm::mutex_unlock() {
	if (pthread_mutex_unlock(&mutex) != 0) {
		throw(std::runtime_error("pthread_mutex_unlock"));
	}
}

void Ipc_shm::condvar_init() {
	pthread_condattr_t attr;
	pthread_condattr_init(&attr);
	pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	if (pthread_cond_init(&cond, &attr) != 0) {
		throw(std::runtime_error("pthread_cond_init"));
	}
}

void Ipc_shm::condvar_wait() {
	if (pthread_cond_wait(&cond, &mutex) != 0) {
		throw(std::runtime_error("pthread_cond_wait"));	
	}
}

void Ipc_shm::condvar_broadcast() {
	if (pthread_cond_broadcast(&cond) != 0) {
		throw(std::runtime_error("pthread_cond_broadcast"));	
	}
}