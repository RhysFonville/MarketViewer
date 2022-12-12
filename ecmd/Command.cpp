#include "Command.h"

NCommand::CommandHandler::CommandHandler(const std::vector<NCommand::Command> &commands)
	: commands(commands), argc(0), argv(std::vector<std::string>({ })) { }

void NCommand::CommandHandler::sort_arguments() {
	// I have no idea how to do this
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
void NCommand::CommandHandler::set_argument_variables(const std::string &input) {
	LPWSTR *args = CommandLineToArgvW(string_to_wstring(input).c_str(), &argc);

	for (int i = 0; i < argc; i++) {
		argv.push_back(wstring_to_string(args[i]));
	}
}
#endif

void NCommand::CommandHandler::set_argument_variables(int argc, char *argv[]) {
	this->argc = argc;
	this->argv = std::vector<std::string>(argv, argv+argc);
}

void NCommand::CommandHandler::process_command(bool clear_args) {
	if (argv.size() > 1) {
		for (size_t i = 0; i < argv.size(); i++) {
			if (argv[i].find('>') != -1) {
				output.open_file(argv[i+1]);
			}
		}

		bool found_command = false;
		for (Command command : commands) {
			if (command.name == argv[FIRST_ARG-1]) {
				found_command = true;

				size_t number_of_mandatory_args = 0;
				for (const std::shared_ptr<Argument> &arg : command.arguments) {
					if (arg->type == ArgumentType::Argument) number_of_mandatory_args++;
				}
				size_t first_optional_arg_index = number_of_mandatory_args;
				for (std::vector<std::string>::iterator it = argv.begin(); it == argv.end(); it++) {
					if ((*it)[0] == '-') {
						first_optional_arg_index = std::distance(argv.begin(), it);
						break;
					}
				}
				if (first_optional_arg_index == number_of_mandatory_args) {
					try {
						command.callback();
						output.print();
						output.close_file();
						output.out.clear();
					} catch (std::exception &e) {
						print_error(e.what(), output);
						output.print("Command aborted.\n\n");
					}
				} else {
					error<std::invalid_argument>("A mandatory argument was not specified.", "Mandatory arguments to not need a dashed dashed modifier (ex: -x) and need to be specified at the beginning of the arguments.");
				}
				break;
			}
		}
		if (!found_command)
			print_error("Command does not exist.", output);
	}

	if (clear_args) {
		argv.clear();
		argc = 0;
	}
}

void NCommand::CommandHandler::help_prompt(const NCommand::Command &command) {
	output.out += command.help_message + "\n\n";

	output.out += to_upper(command.name);

	for (const std::shared_ptr<Argument> &argument : command.arguments) {
		output.out += (argument->type == ArgumentType::Argument ? " {" : " [");
		output.out += argument->name;
		output.out += (argument->type == ArgumentType::Argument ? "}" : "]");
	}
	output.out += "\n\n";
	for (const std::shared_ptr<Argument> &argument : command.arguments) {
		output.out += argument->name + ": " + argument->description + '\n';
	}

	output.out += '\n';
}

NCommand::Argument::Argument(std::string name, std::string description,
	ArgumentType _type)
	: name(name), description(description), type(_type) { }

NCommand::OptionalArgument::OptionalArgument(std::string name,
	std::string description, ArgumentType _type)
	: NCommand::Argument(OPTIONAL_ARG_NAME(name), description, _type) { }

NCommand::ExpansiveOptionalArgument::ExpansiveOptionalArgument(std::string name,
	std::string expansion_name, std::string description, ArgumentType _type)
	: NCommand::OptionalArgument(name, description, _type),
	expansion_name(expansion_name) { }

NArgument::Argument::Argument(std::string name, std::string description,
	Callback callback, ArgumentType _type)
	: NCommand::Argument(name, description, _type), callback(callback) { }

NArgument::OptionalArgument::OptionalArgument(std::string name,
	std::string description, Callback callback, ArgumentType _type)
	: NCommand::OptionalArgument(name, description, _type),
	callback(callback) { }

NArgument::ExpansiveOptionalArgument::ExpansiveOptionalArgument(std::string name,
	std::string expansion_name, std::string description, Callback callback,
	ArgumentType _type)
	: NCommand::ExpansiveOptionalArgument(name, expansion_name, description, _type),
	callback(callback) { }

NArgument::ArgumentHandler::ArgumentHandler(const std::function<void()> &base_callback,
	const std::vector<std::shared_ptr<Argument>> &arguments)
	: base_callback(base_callback), arguments(arguments), argc(0),
	argv(std::vector<std::string>()) { }

void NArgument::ArgumentHandler::set_argument_variables(int argc, char *argv[]) {
	this->argc = argc;
	this->argv = std::vector<std::string>(argv, argv+argc);
}

void NArgument::ArgumentHandler::process_arguments() {
	for (size_t i = 0; i < argv.size(); i++) {
		if (argv[i].find('>') != -1) {
			output.open_file(argv[i+1]);
		}
	}

	int found_arg_location = -1;
	for (int i = 1; i < argv.size(); i++) {
		for (int j = found_arg_location+1; j < arguments.size(); j++) {
			if (arguments[j]->type == ArgumentType::OptionalArgument ||
				arguments[j]->type == ArgumentType::ExpansiveOptionalArgument) {
				if (arguments[j]->name != argv[i]) {
					continue;
				}
			}
			try {
				arguments[j]->callback(argv.begin()+i);
			} catch (std::exception &e) {
				print_error(e.what());
				output.print("Command aborted.\n\n");
			}
			found_arg_location = j;
			break;
		}
	}

	base_callback();

	output.print();
	output.close_file();
	output.out.clear();
}

void NArgument::ArgumentHandler::help_prompt(const NCommand::Argument &argument) {
	output.out += argument.name +
		(argument.type == ArgumentType::ExpansiveOptionalArgument ? argument.name : "") +
		"\n\n" + argument.description + '\n';
}
