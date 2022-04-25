#include <iostream>
#include <getopt.h>
#include "Prog_init.h"
#include "lib/pipe/Ipc_pipe.h"
#include "lib/queue/Ipc_queue.h"
#include "lib/shm/Ipc_shm.h"



// program initial instruction
void Prog_init::printInstruction() {
	std::cout << "This program is a file data transfer program in IPC environment, it is for IPC methods practice.\n"
			     "The program shall receive an argument starts with \"--\"\n"
			     "The options below are available for different file transfer usages(may require argument), --file option is mandatory:\n\n"
			     "--help(-h): display all commands instruction and description(no arguments)\n"
			     "--queue(-q): uses queue as IPC method(argument required)\n"
			     "--pipe(-p): uses name pipe(FIFO) as IPC method(argument required)\n"
			     "--shm(-s): uses shared memory buffer as IPC method(argument required)\n"
			     "--file(-f): file used for read/write data(argument required)\n\n"
				 "**NOTE**: The name of the file entered from the receiver side must be different with the folder name in workspace root\n"
			  << std::endl;
}

// check command line options and arguments and return ipc_info for ipc data transfer
Ipc_info Prog_init::checkOptions (int argc, char** argv) {
	struct option longopts[] = {
		{"help", 0, NULL, 'h'},
		{"message", 1, NULL, 'm'},
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
	Ipc_info info;
	// reset optind(the next element to be processed in argv), now it is possible to scan argv multiple times during one process
	optind = 1;
	while ((option = getopt_long(argc, argv, "hm:p:q:s:f:", longopts, NULL)) != -1) {
		switch(option) {
			case 'h': {
				std::cout << "<HELP>\n"
						     "--message: client-server model, carries priority, QNX native API(argument <serverName>)\n"
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
				std::cout << "Invalid options!" << std::endl;
				break;
		}

	}
	if (f_flag == 1) {      // fflag > 1 is invalid option
		if (p_flag + q_flag + s_flag != 1) {
			throw(std::runtime_error("Prog_init::checkoptions: One IPC method is required!(no duplication) e.g ./temp --pipe <pipeName> --file <fileName>"));
		} else {
			if(p_flag){
				std::cout << "<pipe is used>" << std::endl;
				info.setMethod(Methods::PIPES);
			} else if (q_flag) {
				std::cout << "<queue is used>" << std::endl;
				info.setMethod(Methods::QUEUE);
			} else {
				std::cout << "<shm is used>" << std::endl;
				info.setMethod(Methods::SHM);
			}
		}
	} else {
		throw(std::runtime_error("Prog_init::checkoptions: A filename must be given as an argument for option --file/-f(only one -f is allowed)"));
	}
	return info;
}

// run the corresponding ipc method for transfering data
void Prog_init::run_IPC(const Ipc_info &info, Send_or_receive side) {
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
    	m->send();
	} else {
		m->receive();
	}
}