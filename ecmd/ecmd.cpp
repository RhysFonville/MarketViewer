#include "CommandStorage.h"

int main(int argc, char *argv[]) {
	argument_handler.set_argument_variables(argc, argv);
	argument_handler.process_arguments();

	return 0;
}