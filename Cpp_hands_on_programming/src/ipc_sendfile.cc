/*
 * ipc_sendfile.c
 *
 *  Created on: Feb 17, 2022
 *      Author: kt
*/
#include "lib/other/Prog_init.h"

int main(int argc, char* argv[]) {
	try {
		Prog_init start;
		start.printInstruction();
		start.checkOptions(argc, argv);
		start.run_IPC(Send_or_receive::SEND);
	}
	catch(const std::exception &e) {
		std::rethrow_exception(std::current_exception());
	}
	return 0;
}