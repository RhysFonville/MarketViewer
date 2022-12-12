#include <iostream>
#include <string>
#include <string.h>
#include <functional>
#include <fstream>
#include <algorithm>
#include "curl/curl.h"
#include "gumbo.h"
#include "CommandStorage.h"

int main(int argc, char *argv[]) {
	argument_handler.set_argument_variables(argc, argv);
	argument_handler.process_arguments();
}
