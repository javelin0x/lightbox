#include "ui.h"

#include <vector>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

#include "../cfg/cfg.h"
#include "../util/helpers.h"
#include "../db/db.h"
#include "../db/enc/enc.h"
#include "../pw/pw.h"

std::string lb_logo = R"(			┓ •    ┓  ┓     
			┃ ┓ ┏┓ ┣┓ ╋ ┣┓ ┏┓ ┓┏
			┗ ┗ ┗┫ ┛┗ ┗ ┗┛ ┗┛ ┛┗
			     ┛            )";

namespace ui {
	HANDLE h_con;
	HWND h_wnd;

	int mon_w, mon_h, wnd_w, wnd_h;

	bool logged_in = false;

	void init()
	{
		SetConsoleOutputCP(65001);
		SetConsoleCP(65001);
		SetConsoleTitle(L"lightbox");

		h_con = GetStdHandle(STD_OUTPUT_HANDLE);
		h_wnd = GetConsoleWindow();

		mon_w = GetSystemMetrics(SM_CXSCREEN);
		mon_h = GetSystemMetrics(SM_CYSCREEN);

		wnd_w = 580;
		wnd_h = 690;

		MoveWindow(h_wnd, (mon_w - wnd_w) / 2, (mon_h - wnd_h) / 2, wnd_w, wnd_h, TRUE);
		LONG style = GetWindowLong(h_wnd, GWL_STYLE);
		style = style & ~WS_THICKFRAME;
		style = style & ~WS_MAXIMIZEBOX;
		SetWindowLong(h_wnd, GWL_STYLE, style);

		CONSOLE_SCREEN_BUFFER_INFO scr_buf_info;
		GetConsoleScreenBufferInfo(h_con, &scr_buf_info);

		short win_h = scr_buf_info.srWindow.Bottom - scr_buf_info.srWindow.Top + 1;

		short scr_buf_w = scr_buf_info.dwSize.X;

		COORD new_size;
		new_size.X = scr_buf_w;
		new_size.Y = win_h;

		int status = SetConsoleScreenBufferSize(h_con, new_size);
		if (status == 0)
		{
			ui::message("SetConsoleScreenBufferSize error", ui::UI_ERR);
		}
		intro_animation();
	}

	void pre_login_ui()
	{
		int menu_option = 0;
		std::string menu_profile_input;

		while (!logged_in)
		{
			ui::set_color(ui::colors::UI_GOLD_BLACK);
			ui::print_logo();

			switch (menu_option)
			{
			case options::MAIN_MENU: { 
				ui::print_text_color("[1] ", ui::colors::UI_CYAN_BLACK);
				std::cout << "• load default DB (" << cfg::_path << ")\n";

				ui::print_text_color("[2] ", ui::colors::UI_CYAN_BLACK);
				std::cout << "• load from alternate path\n";

				ui::print_text_color("[3] ", ui::colors::UI_CYAN_BLACK);
				std::cout << "• create new DB\n";

				ui::print_text_color("[4] ", ui::colors::UI_CYAN_BLACK);
				std::cout << "• settings\n";

				ui::print_text_color("[5] ", ui::colors::UI_PURPLE_BLACK);
				std::cout << "• ｡:｡*. d e b u g .*｡:｡\n\n";

				ui::print_text_color("→ ", ui::colors::UI_GOLD_BLACK);

				std::cin >> menu_profile_input;

				if (!helpers::is_digit(menu_profile_input) || std::stoi(menu_profile_input) < 1 || std::stoi(menu_profile_input) > 5)
				{
					ui::message("load a valid option to continue...", ui::UI_ERR);
					Sleep(1500);
					menu_option = 0;
				}
				else {
					ui::message("loading...", ui::UI_OK);
					Sleep(200);
					menu_option = std::stoi(menu_profile_input);
				}
			}break;

			case options::DEFAULT_PATH: { 
				if (!helpers::file_exists(cfg::_path) || !database::is_db(cfg::_path))
				{
					ui::message("couldn't read the file in default path, reloading...", ui::UI_ERR);
					Sleep(1500);
					menu_option = 0;
					break;
				}

				//database::load_db();
				std::cin.get();
			}break;

			case options::CUSTOM_PATH: { 
				std::cout << "BACK to go back\npath: ";
				std::cin >> cfg::_path;

				if (cfg::_path == "BACK" || cfg::_path == "back")
				{
					menu_option = 0;
					cfg::init();
					break;
				}

				if (!helpers::file_exists(cfg::_path) || !database::is_db(cfg::_path))
				{
					ui::message("load a valid path to continue...", ui::UI_ERR);
					Sleep(1500);
					menu_option = 2;
				}
				else {
					ui::message("loading...\n", ui::UI_OK);
					Sleep(200);
					//database::load_db();
				}
			}break;

			case options::CREATE_DB: { 
				
				//database::write_db();

			}break;

			case options::SETTINGS: {

			}break;

			case options::DEBUG: {
				ui::print_text_color("1. hash pw\n2. gen pw\n3. decrypt db\n4. create db\n", ui::colors::UI_WHITE_BLACK);
				ui::print_text_color("→ ", ui::colors::UI_GOLD_BLACK);
				int opt;
				std::cin >> opt;
				if (opt == 1)
				{
					std::string pw_ng;
					std::cin >> pw_ng;
					std::string hash = enc::hash_password(pw_ng);
					//enc::write_enc_file(hash);

					ui::print_text_color(hash, ui::colors::UI_GREEN_BLACK);
				}
				if(opt == 2) {
					int len;
					std::cin >> len;
					ui::print_text_color(pw::random_pw(pw::options::ALL_CHARS, len), ui::colors::UI_GREEN_BLACK);
				}
				if (opt == 3)
				{
					std::cout << "input path\n";
					std::string path;
					std::cin >> path;
					
					std::string pw;
					std::cout << "input pw\n";
					std::cin >> pw;

					std::string hash = enc::hash_password(pw);

					enc::decrypt_enc_file(path, hash);
				}
				if (opt == 4)
				{
					ui::print_text_color("enter a master password for your database:\n", ui::colors::UI_GREEN_BLACK);
					ui::print_text_color("→ ", ui::colors::UI_GOLD_BLACK);
					std::string pw_1, pw_2, content;
					std::cin >> pw_1;

					system("cls");

					ui::print_logo();
					ui::print_text_color("confirm the master password:\n", ui::colors::UI_GREEN_BLACK);
					ui::print_text_color("→ ", ui::colors::UI_GOLD_BLACK);
					std::cin >> pw_2;

					system("cls");

					ui::print_logo();
					ui::print_text_color("set content:\n", ui::colors::UI_GREEN_BLACK);
					ui::print_text_color("→ ", ui::colors::UI_GOLD_BLACK);
					std::cin >> content;

					if (pw_1 != pw_2)
					{
						ui::message("the passwords do not match! try again", ui::UI_ERR);
						Sleep(1500);
						break;
					}

					std::string hash = enc::hash_password(pw_1);
					ui::print_text_color(hash + '\n', ui::colors::UI_YELLOW_BLACK);

					ui::message("creating database!", ui::UI_OK);
					Sleep(1400);

					enc::write_enc_file(hash, content);
				}
				
				Sleep(3000);
			}break;
			}

			system("cls");
		}

	}

	void intro_animation()
	{
		std::vector<std::string> colors = { "83", "8C", "37", "76", "06" };

		for (int i = 0; i < 5; i++)
		{
			std::string color_curr = "Color " + colors[i];
			system(color_curr.c_str());

			std::cout << lb_logo;
			Sleep(200);
			system("cls");
		}
	}

	void print_logo()
	{
		std::cout << lb_logo;
		set_color(ui::colors::UI_GREY_BLACK);
		std::cout << "BETA v0.1\n\n";
	}
	
	void message(const std::string& msg, int type)
	{
		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
		std::tm now_tm;
		localtime_s(&now_tm, &now_time);
		std::ostringstream oss;
		oss << std::put_time(&now_tm, "%H:%M");

		switch (type)
		{
			case 0: {

			}break;
			case 1: {
				set_color(ui::colors::UI_GREY_BLACK);
				std::cout << "(" << oss.str() << ") ";
				set_color(ui::colors::UI_GREEN_BLACK);
				std::cout << "[OK] ~ " << msg;
			}break;
			case 2: {
				set_color(ui::colors::UI_GREY_BLACK);
				std::cout << "(" << oss.str() << ") ";
				set_color(ui::colors::UI_RED_BLACK);
				std::cout << "[ERROR] ~ " << msg;
			}break;
			case 3: {
				set_color(ui::colors::UI_GREY_BLACK);
				std::cout << "(" << oss.str() << ") ";
				set_color(ui::colors::UI_YELLOW_BLACK);
				std::cout << "[WARN] ~ " << msg;
			}break;
		}

		set_color(ui::colors::UI_GREY_BLACK);
	}

	void set_color(int color)
	{
		SetConsoleTextAttribute(ui::h_con, helpers::int_to_word(color));
	}

	void print_text_color(const std::string& msg, int color)
	{
		SetConsoleTextAttribute(ui::h_con, helpers::int_to_word(color));
		std::cout << msg;
		set_color(ui::colors::UI_GREY_BLACK);
	}
}