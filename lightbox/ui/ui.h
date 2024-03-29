#pragma once

#include <string>
#include <Windows.h>

namespace ui {
	extern HANDLE h_con;
	extern HWND h_wnd;

	extern bool logged_in;

	extern int mon_w, mon_h, wnd_w, wnd_h;

	void init();
	void intro_animation();
	void print_logo();
	void message(const std::string& msg, int type);
	void set_color(int color);
	void print_text_color(const std::string& msg, int color);
	void pre_login_ui();

	const int UI_OK   = 1;
	const int UI_ERR  = 2;
	const int UI_WARN = 3;

	namespace colors {
		const int UI_YELLOW_BLACK = 14;
		const int UI_RED_BLACK = 4;
		const int UI_GREY_BLACK = 8;
		const int UI_WHITE_BLACK = 7;
		const int UI_GREEN_BLACK = 2;
		const int UI_GOLD_BLACK = 6;
		const int UI_CYAN_BLACK = 11;
		const int UI_PURPLE_BLACK = 13;
	}

	namespace options {
		const int MAIN_MENU = 0;
		const int DEFAULT_PATH = 1;
		const int CUSTOM_PATH = 2;
		const int CREATE_DB = 3;
		const int SETTINGS = 4;
		const int DEBUG = 5;
	}
}