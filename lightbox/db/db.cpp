#include "db.h"

#include <iostream>
#include <filesystem>

#include "../util/helpers.h"
#include "io/json.hpp"
#include "../cfg/cfg.h"
#include "../ui/ui.h"
#include "enc/enc.h"

/*
JSON struct
{
    creds 
    {
     1
     {
        "id": 954,
        "user": pepe@gmail.com,
        "password": abarajirew891
     }
     2 
     {
        "id": 943,
        "user": rere@gmail.com,
        "password": rea321ba13!
     }
    }

}
*/

namespace database {
    int creds_count;
    std::string db_name;
    std::string j_dump;

	bool is_db(std::string filepath)
	{
        if (filepath.substr(filepath.size() - 5) == ".lbdb")
        {
            try {
                nlohmann::json j = nlohmann::json::parse(helpers::file_to_str(cfg::_path));
            }
            catch (nlohmann::json::parse_error& e) {
                (void)e;
                return false;
            }

            return true;
        }

        return false;
	}

    std::string default_db()
    {
        nlohmann::json j;
        j["creds"]["1"] = {
           {"id", 954},
           {"user", "example@1.com"},
           {"password", "example"},
           {"title", "exampleeeeeee"}
        };
        
        return j.dump(4);
    }

    std::vector<credential> load_db(std::string file_content) //TODO: implement decryption & loading taking as paramter master pw
    {
        std::vector <credential> credentials;
        int file_len = file_content.length();
        nlohmann::json j;
        try {
            j = nlohmann::json::parse(file_content);
        }
        catch (nlohmann::json::parse_error& e) {
            //ui::message(e.what(), ui::UI_ERR);
            credential err_cred;
            err_cred.id = 0;
            err_cred.user = "ERROR";
            err_cred.password = "ERROR";
            credentials.push_back(err_cred);
            return credentials;
        }

        cfg::update_db_path();
        cfg::db_name = static_cast<std::filesystem::path>(cfg::_path).filename().string();

        if (j["creds"].empty())
        {
            return credentials;
        }
        for (const auto& cred : j["creds"])
        {
            credential credential;
            credential.id = cred["id"];
            credential.user = cred["user"];
            credential.password = cred["password"];
            credential.title = cred["title"];

            credentials.push_back(credential);
        }

        return credentials;
    }
   
    std::string add_credential(std::vector<credential>& creds, credential new_credential, std::string db_content)
    {
        nlohmann::json j;
        try {
            j = nlohmann::json::parse(db_content);
        }
        catch (nlohmann::json::parse_error& e) {
            ui::message(e.what(), ui::UI_ERR);
            //Sleep(3000);
            return "";
        }

        creds.push_back(new_credential);

        int highest_key = 0;
        int count = 0;
        nlohmann::json creds_j = j["creds"];
        for (auto i = creds_j.begin(); i != creds_j.end(); i++)
        {
            if (std::stoi(i.key()) > highest_key)
            {
                highest_key = std::stoi(i.key());
            }
            count++;
        }

        j["creds"][std::to_string(highest_key + 1)] = {
           {"id", new_credential.id},
           {"user", new_credential.user},
           {"password", new_credential.password},
           {"title", new_credential.title}
        };

        return j.dump(4);
    }

    std::string remove_credential(std::vector<credential>& creds, credential cred, int idx, std::string db_content)
    {
        nlohmann::json j;
        try {
            j = nlohmann::json::parse(db_content);
        }
        catch (nlohmann::json::parse_error& e) {
            //ui::message(e.what(), ui::UI_ERR);
            //Sleep(3000);
            return "";
        }

        nlohmann::json creds_j = j["creds"];
        int count = 0;
        for (auto i = creds_j.begin(); i != creds_j.end(); i++)
        {
            if (count == idx)
            {
                j["creds"].erase(i.key());
            }
            count++;
        }

        creds.erase(creds.begin() + idx);
        return j.dump(4);
    }

    void save_db(std::string db_content, std::string key)
    {
        nlohmann::json j;
        try {
            j = nlohmann::json::parse(db_content);
        }
        catch (nlohmann::json::parse_error& e) {
            return;
        }

        int bb = db_content.length();

        enc::write_enc_file(key, db_content);
    }

    std::string new_db_path()
    {
        char exe_path_c[MAX_PATH];
        GetModuleFileNameA(NULL, exe_path_c, MAX_PATH);
        std::filesystem::path exe_parent_path = static_cast<std::filesystem::path>(exe_path_c).parent_path();
        std::string prefix = "db";
        int count = 0;

        for (const auto& entry : std::filesystem::directory_iterator(exe_parent_path)) {
            if (entry.is_regular_file() && entry.path().filename().string().substr(0, prefix.size()) == prefix) {
                ++count;
            }
        }

        std::string finalpath = exe_parent_path.string() + "\\db-" + std::to_string(count) + ".lbdb";
        return finalpath;
    }
}