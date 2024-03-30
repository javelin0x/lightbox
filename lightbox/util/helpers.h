#pragma once

#include <iostream>
#include <filesystem>
#include <Windows.h>
#include <vector>

namespace helpers {
	bool directory_exists(std::string path);
	bool file_exists(std::string path);
	std::vector<unsigned char> str_to_bytes(const std::string& str);
	std::string bytes_to_str(const std::vector<unsigned char>& bytes);
	bool is_digit(const std::string& s);
	WORD int_to_word(int val);
	std::string file_to_str(const std::string& filepath);
	void str_to_file(const std::string& filename, const std::string& content);
	std::string b64_encode(const std::string& input);
	std::vector<unsigned char> add_padding(const std::vector<unsigned char>& data);
	std::vector<unsigned char> remove_padding(const std::vector<unsigned char>& data);
	std::string tchar_to_utf8(TCHAR* tchar_arr);
	void to_clipboard(const std::string& s);
	std::vector<unsigned char> file_to_vec(const std::string& path);
	void bytes_to_file(const char* file_path, std::vector<unsigned char>& bytes);
	void clear_clipboard();
	std::vector<char> uint_to_char(const unsigned int* data, size_t size);
	void clear_string(std::string& s);
}