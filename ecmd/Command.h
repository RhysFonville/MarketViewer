#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <vector>
#include <memory>
#include <variant>
#include "util.h"

/*
* "ecmd" stands for "Epic Command". As in, "My Epic Command Prompt"
* Began as a linux like command prompt, it is now an API to make creating commands easy.
* 
* Made September 18, 2022, at 2:07 PM
*/

#define MAKE_ARG(NAMESPACE, ARG) std::make_shared<NAMESPACE::Argument>(NAMESPACE::ARG)
#define OPTIONAL_ARG_NAME(NAME) (NAME[0] == '-' ? NAME : '-'+NAME)
#define FIRST_ARG 1

class Output {
public:
	Output() {
		
	}

	void print() {
		if (toggle == 0) {
			cout_stream << out;
		} else {
			file_stream << out;
		}
	}

	void print(const std::string &string) {
		if (toggle == 0) {
			cout_stream << string;
		} else {
			file_stream << string;
		}
	}

	const std::ofstream & get_file() const noexcept {
		return file_stream;
	}

	void open_file(const std::string &name) {
		file_stream.open(name);
		toggle = 1;
	}

	void close_file() {
		if (file_stream.good() && file_stream.is_open()) file_stream.close();
		toggle = 0;
	}

	friend std::ostream & operator<<(std::ostream &os, const Output &output);

	std::string out = "";

private:
	int toggle = 0;

	std::ostream &cout_stream = std::cout;
	std::ofstream file_stream = std::ofstream();
};

inline std::ostream & operator<<(std::ostream &os, const Output &output) {
	os << output.out;
	return os;
}

enum class ArgumentType {
	Argument,
	OptionalArgument,
	ExpansiveOptionalArgument,
};

namespace NCommand {
class Argument {
public:
	Argument(std::string name, std::string description,
		ArgumentType _type = ArgumentType::Argument);

	std::string name = "";
	std::string description = "";

	ArgumentType type;
};

class OptionalArgument : public Argument {
public:
	OptionalArgument(std::string name, std::string description,
		ArgumentType _type = ArgumentType::OptionalArgument);
};

class ExpansiveOptionalArgument : public OptionalArgument {
public:
	ExpansiveOptionalArgument(std::string name, std::string expansion_name,
		std::string description, ArgumentType _type = ArgumentType::ExpansiveOptionalArgument);
	
	std::string expansion_name;
};

class Command {
public:
	Command(std::string name, std::vector<std::shared_ptr<Argument>> arguments,
		std::string help_message, std::function<void()> callback)
		: name(name), arguments(arguments), help_message(help_message),
		callback(callback) { }

	std::string name;
	std::vector<std::shared_ptr<Argument>> arguments;
	std::string help_message;
	std::function<void()> callback;
};

class CommandHandler {
public:
	CommandHandler(const std::vector<Command> &commands);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	void set_argument_variables(const std::string &input);
#endif
	
	void set_argument_variables(int argc, char *argv[]);

	void process_command(bool clear_args = true);

	void help_prompt(const Command &command);

	std::string & get_arg(size_t index) { return argv[FIRST_ARG + index]; }

	std::vector<Command> commands;

	int argc;
	std::vector<std::string> argv = { };
	Output output;
private:
	void sort_arguments();
};
}

namespace NArgument {

using ArgvLocation = const std::vector<std::string>::iterator&;
using Callback = std::function<void(ArgvLocation)>;

class Argument : public NCommand::Argument {
public:
	Argument(std::string name, std::string description, Callback callback,
		ArgumentType _type = ArgumentType::Argument);

	Callback callback;
};
class OptionalArgument : public NCommand::OptionalArgument {
public:
	OptionalArgument(std::string name, std::string description,
		Callback callback, ArgumentType _type = ArgumentType::OptionalArgument);

	operator NArgument::Argument() {
		return NArgument::Argument(name, description, callback,  ArgumentType::OptionalArgument);
	}

	Callback callback;
};
class ExpansiveOptionalArgument : public NCommand::ExpansiveOptionalArgument {
public:
	ExpansiveOptionalArgument(std::string name, std::string expansion_name,
		std::string description, Callback callback,
		ArgumentType _type = ArgumentType::ExpansiveOptionalArgument);

	operator NArgument::OptionalArgument() {
		return NArgument::OptionalArgument(name, description, callback, ArgumentType::ExpansiveOptionalArgument);
	}

	operator NArgument::Argument() {
		return NArgument::OptionalArgument(name, description, callback, ArgumentType::ExpansiveOptionalArgument);
	}

	Callback callback;
};

class ArgumentHandler {
public:
	ArgumentHandler(const std::function<void()> &base_callback,
		const std::vector<std::shared_ptr<Argument>> &arguments);

	void set_argument_variables(int argc, char *argv[]);

	void process_arguments();

	void help_prompt(const NCommand::Argument &argument);

	std::function<void()> base_callback;

	std::vector<std::shared_ptr<Argument>> arguments;

	int argc = 0;
	std::vector<std::string> argv = { };
	Output output;
};
}

static void print_error(const std::string &str) {
	std::vector<std::string> message = split(str, "====SUGGESTIONS====");

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	SetConsoleTextAttribute(console, ERROR_TEXT_COLOR);
#endif

	std::cerr << message[0];

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	SetConsoleTextAttribute(console, ERROR_SUGGESTION_TEXT_COLOR);
#endif

	for (int i = 1; i < message.size(); i++) {
		std::cerr << message[i];
	}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	SetConsoleTextAttribute(console, DEFAULT_TEXT_COLOR);
#endif

	std::cout << std::endl;
}

static void print_error(const std::string &str, Output &output) {
	std::vector<std::string> message = split(str, "====SUGGESTIONS====");

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	SetConsoleTextAttribute(console, ERROR_TEXT_COLOR);
#endif

	output.print(message[0]);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	SetConsoleTextAttribute(console, ERROR_SUGGESTION_TEXT_COLOR);
#endif

	for (int i = 1; i < message.size(); i++) {
		output.print(message[i]);
	}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	SetConsoleTextAttribute(console, DEFAULT_TEXT_COLOR);
#endif

	output.print("\n");
}
