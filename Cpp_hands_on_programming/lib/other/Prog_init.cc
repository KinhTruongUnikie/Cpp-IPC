#include <iostream>
#include <getopt.h>
#include "Prog_init.h"
#include "lib/pipe/Ipc_pipe.h"
#include "lib/queue/Ipc_queue.h"
#include "lib/shm/Ipc_shm.h"
#include <sys/stat.h>



// program initial instruction
void Prog_init::printInstruction() {
	std::cout << "This program is a file data transfer program in IPC environment, it is for IPC methods practice.\n"
			     "The program shall receive an argument starts with \"--\"\n"
			     "The options below are available for different file transfer usages(may require argument), --file option is mandatory:\n\n"
			     "--help(-h): display all commands instruction and description(no arguments)\n"
			     "--queue(-q): uses queue as IPC method(argument required)\n"
			     "--pipe(-p): uses name pipe(FIFO) as IPC method(argument required)\n"
			     "--shm(-s): uses shared memory buffer as IPC method(argument required)\n"
			     "--file(-f): file used for read/write data(argument required)\n"
			  << std::endl;
}

// check command line options and arguments and return ipc_info for ipc data transfer
Ipc_info & Prog_init::checkOptions (int argc, char** argv) {
	struct option longopts[] = {
		{"help", 0, NULL, 'h'},
		{"pipe", 1, NULL, 'p'},
		{"queue", 1, NULL, 'q'},
		{"shm", 1, NULL, 's'},
		{"file", 1, NULL, 'f' },
		{0, 0, 0, 0}
	};

	int option;
	int p_flag(0);
	int q_flag(0);
	int s_flag(0);
	int f_flag(0);
	// ipc methods and file info
	// reset optind(the next element to be processed in argv), now it is possible to scan argv multiple times during one process
	optind = 1;
	while ((option = getopt_long(argc, argv, "hm:p:q:s:f:", longopts, NULL)) != -1) {
		if (optarg != nullptr) {
			auto arg = std::string(optarg);
			if (arg[0] == '-') {
				throw(std::runtime_error("Missing required argument"));
			}
		}
		switch(option) {
			case 'h': {
				std::cout << "<HELP>\n"
						     "--pipe: POSIX, portable, does not carry priority(argument <pipeName>)\n"
						     "--queue: POSIX, basically pipe with extra feature(argument <queueName>)\n"
						     "--shm: use shared memory region for message passing, required synchronization measure, e.g mutex, condvar(argument <shmName>)\n"
						     "--file: add the file for data transfer (argument <fileName>)\n"
						     "</HELP>\n"
						  << std::endl;
				break;
			}

			case 'p': {
				p_flag++;
				info.setArgument(optarg); 
				break;
			}

			case 'q': {
				q_flag++;
				info.setArgument(optarg);
				break;
			}

			case 's': {
				s_flag++;
				info.setArgument(optarg);
				break;
			}

			case 'f': {
				f_flag++;
				info.setFilename(optarg);
				break;
			}

			default:
				throw(std::runtime_error("Invalid option!"));
		}

	}
	if (f_flag == 1) {      // fflag > 1 is invalid option
		if (p_flag + q_flag + s_flag != 1) {
			throw(std::runtime_error("Prog_init::checkoptions: One IPC method is required!(no duplication) e.g ./temp --pipe <pipeName> --file <fileName>"));
		} else {
			if(p_flag){
				info.setMethod(Methods::PIPES);
			} else if (q_flag) {
				info.setMethod(Methods::QUEUE);
			} else {
				info.setMethod(Methods::SHM);
			}
		}
	} else {
		throw(std::runtime_error("Prog_init::checkoptions: A filename must be given as an argument for option --file/-f(only one -f is allowed)"));
	}
	return info;
}

// run the corresponding ipc method for transfering data
void Prog_init::run_IPC(Send_or_receive side) {
	std::shared_ptr<Ipc_method> m;
	if (info.getMethod() == Methods::PIPES) {
		m = std::make_shared<Ipc_pipe>(info.getArgument(), info.getFilename());
	} else if (info.getMethod() == Methods::QUEUE) {
		m = std::make_shared<Ipc_queue>(info.getArgument(), info.getFilename());
	} else if (info.getMethod() == Methods::SHM) {
		m = std::make_shared<Ipc_shm>(info.getArgument(), info.getFilename());
	} else {
		throw(std::runtime_error("Prog_init::run_IPC: ipc method does not exist"));
	}

	if (side == Send_or_receive::SEND) {
		if (!fileExist()) {
			throw(std::runtime_error("Send file does not exist!"));
		}
    	m->send();
	} else {
		if (fileExist()) {
			if (!overwritable()) {	
				throw(std::runtime_error("Received file already exist!"));
			}
		}
		m->receive();
	}
}

bool Prog_init::fileExist() {
	struct stat st;
	bool exist = (stat(info.getFilename().c_str(), &st) == 0);
	if (exist) {
		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			throw(std::runtime_error("File name is an already exist directory name!"));
		}
		return true;
	} else {
		return false;
	}
}

 bool Prog_init::overwritable() {
	char input(' ');
	std::cout << "This program will overwrite the content of " << info.getFilename() << ". \n"
	"Do you wanna continue(Y/n)?";	
	std::cin >> input;
	if (!std::cin) {
		throw(std::runtime_error("cin fail"));
	} 
	switch(input) {
		case 'y': case 'Y':
			return true;
			break;
		default:
			return false;
			break;
	}
 }