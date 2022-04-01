/*
 * ipc_receivefile.c
 *
 *  Created on: Feb 17, 2022
 *      Author: kt
*/
#include "lib/other/Prog_init.h"

int main(int argc, char* argv[]) {
	try {
		std::shared_ptr<Ipc_info> info;
		Prog_init start;
		start.printInstruction();
		info = start.checkOptions(argc, argv);
		start.run_IPC(info, RECEIVE);
	}
	catch(const std::exception &e) {
		std::cout << "Error: " << e.what() << std::endl;
	}
	return 0;
}