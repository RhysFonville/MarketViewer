#pragma once

#include <fstream>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <Windows.h>
#include <filesystem>
#endif
#include <vector>
#include <string>
#include <codecvt>
#include <locale>
#include <type_traits>

#define PATH_DOES_NOT_EXIST "The specified path does not exist.", "Try checking for any typos."

static constexpr char DEFAULT_TEXT_COLOR = 15;
static constexpr char ERROR_TEXT_COLOR = 12;
static constexpr char ERROR_SUGGESTION_TEXT_COLOR = 4;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
namespace fs = std::filesystem;

static HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

static std::string user_folder_path;

static void error(const std::string &str, const std::string &suggestion = "") {
	std::string message;
	message += "ERROR: " + str + '\n';

	message += "====SUGGESTIONS====";

	if (!suggestion.empty()) {
		message += "  - " + suggestion + '\n';
	}

	throw std::runtime_error(message);
}

template <typename T, typename = std::enable_if_t<std::is_base_of_v<std::exception, T>>>
static void error(const std::string &str, const std::string &suggestion = "") {
	std::string message;
	message += "ERROR: " + str + '\n';

	message += "====SUGGESTIONS====";

	if (!suggestion.empty()) {
		message += "  - " + suggestion + '\n';
	}

	throw T(message);
}

template <typename T>
static void insert_block(std::vector<T> &target, const std::vector<T> &vec, int index = 0) {
	target.insert(target.begin()+index, vec.begin(), vec.end());
}

static std::string to_upper(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), [&](char c) { return toupper(c); });
	return s;
}

static int find_arg(std::vector<std::string> args, std::string value) {
	for (int i = 0; i < args.size(); i++) {
		if (value == args[i]) {
			return i;
		}
	}
	return -1;
}

static std::vector<std::string> split(const std::string &s, const std::string &delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
		token = s.substr (pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr (pos_start));
	return res;
}

static std::wstring string_to_wstring(const std::string &str) {
	return std::wstring(str.begin(), str.end());
}

static std::string wstring_to_string(const std::wstring &wstr) {
	return std::string(wstr.begin(), wstr.end());
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
static std::string file_type_to_string(const fs::file_type &type) {
	switch (type) {
	case fs::file_type::none:
		return "none";
		break;
	case fs::file_type::not_found:
		return "not_found";
		break;
	case fs::file_type::regular:
		return "regular";
		break;
	case fs::file_type::directory:
		return "directory";
		break;
	case fs::file_type::symlink:
		return "symlink";
		break;
	case fs::file_type::block:
		return "block";
		break;
	case fs::file_type::character:
		return "character";
		break;
	case fs::file_type::fifo:
		return "fifo";
		break;
	case fs::file_type::socket:
		return "socket";
		break;
	default:
		return "unknown";
		break;
	}
}

namespace std {
	namespace filesystem {
		static fs::file_type get_file_type(const fs::path &path) noexcept {
			if (fs::is_block_file(path)) {
				return fs::file_type::block;
			} else if (fs::is_character_file(path)) {
				return fs::file_type::character;
			} else if (fs::is_directory(path)) {
				return fs::file_type::directory;
			} else if (fs::is_fifo(path)) {
				return fs::file_type::fifo;
			} else if (fs::is_other(path)) {
				return fs::file_type::none;
			} else if (fs::is_regular_file(path)) {
				return fs::file_type::regular;
			} else if (fs::is_socket(path)) {
				return fs::file_type::socket;
			} else if (fs::is_symlink(path)) {
				return fs::file_type::symlink;
			} else {
				return fs::file_type::unknown;
			}
		}
	}
}
#endif

static std::string words_to_string(const std::vector<std::string> &words) noexcept {
	std::string ret;
	std::for_each(words.begin(), words.end(), [&](const std::string &str) {
		ret += str;
		});

	return ret;
}
