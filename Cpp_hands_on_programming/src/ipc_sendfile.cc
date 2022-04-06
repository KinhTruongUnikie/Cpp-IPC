/*
 * ipc_sendfile.c
 *
 *  Created on: Feb 17, 2022
 *      Author: kt
*/
#include "lib/other/Prog_init.h"

int main(int argc, char* argv[]) {
	try {
		Ipc_info info;
		Prog_init start;
		start.printInstruction();
		info = start.checkOptions(argc, argv);
		start.run_IPC(info, Send_or_receive::SEND);
	}
	catch(const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::rethrow_exception(std::current_exception());
	}
	return 0;
}