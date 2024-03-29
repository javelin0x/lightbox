#include "cfg.h"

#include <fstream>
#include <Windows.h>
#include <string>
#include <filesystem>

#include "../db/io/json.hpp"

#include "../util/helpers.h"
#include "../ui/ui.h"
#include "../gui/gui.h"

namespace cfg {
	std::string _path = "";
	std::string cfg_path = "";
	std::string db_name = "";
	std::string exe_path = "";
	int lang = 0;

	void init()
	{
		char exe_path_c[MAX_PATH];
		GetModuleFileNameA(NULL, exe_path_c, MAX_PATH);
		std::filesystem::path exe_path_full = std::string(exe_path_c);
		std::filesystem::path exe_parent_path = exe_path_full.parent_path();
		exe_path = exe_parent_path.string();
		cfg_path = exe_parent_path.string() + "\\lightbox.on";

		if (!helpers::file_exists(cfg_path)) //no cfg file
		{
			nlohmann::json j;

			if (!helpers::file_exists(_path)) //no db file
			{
				 j = {
					{"db_path", "NONE"},
					{"wnd_border", "1.000000,1.000000,1.000000"}
				};		
				 _path = "NONE";
			}
			else {
				j = {
					{"db_path", _path},
					{"wnd_border", "1.000000,1.000000,1.000000"}
				};
			}

			gui::wnd_border_color.push_back(1.0f);
			gui::wnd_border_color.push_back(1.0f);
			gui::wnd_border_color.push_back(1.0f);

 			_path = j["db_path"];
			helpers::str_to_file(cfg_path, j.dump(4));
		}
		else { //cfg file exists, read default db path from file
			nlohmann::json j;
			try {
				j = nlohmann::json::parse(helpers::file_to_str(cfg_path));
			}
			catch (nlohmann::json::parse_error& e) {
				ui::message(e.what(), ui::UI_ERR);
				Sleep(3000);
				return;
			}
			_path = j["db_path"];
			if (!helpers::file_exists(_path)) //if the default path doesnt point to a db
			{
				j["db_path"] = "NONE";
				_path = "NONE";
			}

			std::string color_str = j["wnd_border"];

			std::stringstream ss(color_str);
			std::string item;
			//std::vector<float> rgb_vals;

			while (std::getline(ss, item, ',')) {
				gui::wnd_border_color.push_back(std::stof(item));
			}

			helpers::str_to_file(cfg_path, j.dump(4));
		}
	}

	void update_db_path()
	{
		nlohmann::json j;
		try {
			j = nlohmann::json::parse(helpers::file_to_str(cfg_path));
		}
		catch (nlohmann::json::parse_error& e) {
			ui::message(e.what(), ui::UI_ERR);
			Sleep(3000);
			return;
		}

		if (!helpers::file_exists(_path))
		{
			j["db_path"] = "NONE";
			_path = "NONE";
		}
		else {
			j["db_path"] = _path;
		}

		helpers::str_to_file(cfg_path, j.dump(4));
	}

	void load_db()
	{

	}

	void update_color(int item, std::string color_str)
	{
		nlohmann::json j;
		try {
			j = nlohmann::json::parse(helpers::file_to_str(cfg_path));
		}
		catch (nlohmann::json::parse_error& e) {
			ui::message(e.what(), ui::UI_ERR);
			Sleep(3000);
			return;
		}

		switch (item)
		{
			case WND_BORDER_COLOR: {
				j["wnd_border"] = color_str;
			}break;
		}

		helpers::str_to_file(cfg_path, j.dump(4));
	}

	
}