#pragma once

#include <string>

namespace enc {
	std::string hash_password(const std::string& password);
	std::string decrypt_enc_file(std::string path, std::string hash);
	void write_enc_file(const std::string& hash, std::string content);
}