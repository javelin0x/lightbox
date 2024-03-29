#pragma once

#include <string>
#include <vector>

struct credential {
	int id;
	std::string title;
	std::string password;
	std::string user;
};

namespace database {
	extern int creds_count;
	bool is_db(std::string filepath);
	std::string default_db();
	std::vector<credential> load_db(std::string file_content);
	std::string add_credential(std::vector<credential>& creds, credential new_credential, std::string db_content);
	std::string remove_credential(std::vector<credential>& creds, credential cred, int idx, std::string db_content);
	void save_db(std::string db_content, std::string key);
	std::string new_db_path();
}