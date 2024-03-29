#pragma once

#include <string>

namespace cfg {
	extern std::string _path;
	extern std::string cfg_path;
	extern std::string db_name;
	extern std::string exe_path;

	extern int lang;
	void update_db_path();
	void init();
	void update_color(int item, std::string color_str);

	const int WND_BORDER_COLOR = 0;
}