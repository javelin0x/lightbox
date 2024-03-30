#include "gui.h"

#include <string>
#include <Windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <vector>
#include <thread>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_opengl3_loader.h"
#include "imgui/imgui_impl_glfw.h"

#include "window.h"
#include "../util/helpers.h"
#include "../cfg/cfg.h"
#include "../db/db.h"
#include "../db/enc/enc.h"
#include "../pw/pw.h"

#include "font/icon_font.cpp"
#include "font/lexend_bold.cpp"
#include "font/lexend_regular.cpp"

//i strongly recommend avoiding looking at this file 

ImFont* lexend_bold;
ImFont* lexend_regular;
ImFont* lexend_bold_medium;
ImFont* lexend_bold_small;
ImFont* lexend_regular_small;
ImFont* icon_font_regular;
ImFont* lexend_bold_tab;
ImFont* lexend_regular_smallest;

bool dragging = false;
ImVec2 dragStart;

int menu_opt = 0;
bool logged_in = false;
bool adding_cred = false;

//buffers
static char cred_title_buf[256] = "";
static char cred_user_buf[256] = "";
static char cred_pw_buf[256] = "";
static char master_pw_buf[256] = "";

std::string login_txt = "login";
std::string logged_in_txt = "database";
std::string add_cred_txt = "add credential";
std::string settings_txt = "settings";
std::string debug_txt = "debug";
std::string about_txt = "about";

std::string db_path_hint = "C:/...";

//DB
std::string pw_hash;
std::string db_content;

//wnd properties (cfg)
float wnd_border_color_f[4] = { 0.29f, 0.64f, 0.64f };

void colored_text(const char* text)
{
	int numSegments = strlen(text);

	for (int i = 0; i < numSegments; ++i) {
		float colorValue = (float)i / (float)numSegments;
		ImVec4 color(0.2f, 1.0f - (colorValue / 2.0f), 0.8f, 1.0f);

		ImGui::PushStyleColor(ImGuiCol_Text, color);

		ImGui::SameLine();
		if (i > 0)
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
		}
		ImGui::TextUnformatted(text + i, text + i + 1);

		ImGui::PopStyleColor();
	}
}

void clear_strings(int logged)
{
	if (logged)
	{
		SecureZeroMemory(cred_pw_buf, sizeof(cred_pw_buf));
		SecureZeroMemory(cred_user_buf, sizeof(cred_user_buf));
		SecureZeroMemory(master_pw_buf, sizeof(master_pw_buf));
		SecureZeroMemory(cred_title_buf, sizeof(cred_title_buf));
	} else {
		helpers::clear_string(pw_hash);
		helpers::clear_string(db_content);
		pw_hash.clear();
		db_content.clear();

		SecureZeroMemory(cred_pw_buf, sizeof(cred_pw_buf));
		SecureZeroMemory(cred_user_buf, sizeof(cred_user_buf));
		SecureZeroMemory(master_pw_buf, sizeof(master_pw_buf));
		SecureZeroMemory(cred_title_buf, sizeof(cred_title_buf));
	}
}

void wnd_controls(ImDrawList* draw_list)
{
	ImVec2 window_pos = ImGui::GetWindowPos();
	ImVec2 window_size = ImGui::GetWindowSize();
	float rounding = 15.0f;
	ImVec2 rect_min = ImVec2(ImGui::GetWindowSize().x - 50.0f, 17.0f); 
	ImVec2 rect_max = ImVec2(rect_min.x +  40.0f, rect_min.y + 20.0f); 
	draw_list->AddRectFilled(rect_min, rect_max, ImColor(201, 201, 201, 200), rounding);

	ImGui::SetNextWindowPos(rect_min);
	ImGui::SetNextWindowSize(ImVec2(rect_max.x - rect_min.x, rect_max.y - rect_min.y));
	ImGui::BeginChild("controlsrect", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImVec4 transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); 
	ImGui::PushStyleColor(ImGuiCol_Button, transparent); 
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, transparent); 
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, transparent); 
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(64, 64, 64).operator ImVec4());
	ImGui::PushFont(lexend_bold_small);

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6.5f);
	if (ImGui::Button("_", ImVec2(14.0f, 20.0f))) {
		glfwIconifyWindow(window::_window);
	}
	 ImGui::SameLine();
	 ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
	 ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.5f);
	if (ImGui::Button("x", ImVec2(14.0f, 20.0f))) {
		gui::closed = true;

		clear_strings(false);

		ImGui::PopFont();
		ImGui::PopStyleColor(4);

		ImGui::EndChild();
		return;
	}

	ImGui::PopFont();
	ImGui::PopStyleColor(4);

	ImGui::EndChild();
}

void glow_dot()
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 dot_pos = ImGui::GetCursorPos(); 
	dot_pos.x -= 10.0f;
	dot_pos.y += 12.0f;
	//dot_pos.x += 50.0f;

	ImU32 dot_color = IM_COL32(255, 255, 255, 255); 

	float initial_size = 3.0f;
	float initial_alpha = 255;

	for (int i = 0; i < 5; ++i) {
		float size = initial_size * (1.0f - (float)i / 5.0f);
		float alpha = initial_alpha * (1.0f - (float)i / 5.0f);
		ImU32 glow_color = (menu_opt == 3) ? IM_COL32(191, 187, 73, (int)alpha) : IM_COL32(130, 195, 255, (int)alpha);
		//glow_color = (menu_opt == 0 && logged_in) ? IM_COL32(69, 212, 53, (int)alpha) : glow_color;
		//draw_list->_FringeScale = 3.0f;
		draw_list->AddCircle(dot_pos, size, glow_color);
		draw_list->_FringeScale = 1.0f;
	}
}
std::vector<credential> creds;
bool db_loaded = false;

void login_tab(ImVec2 drawing_area_size)
{
	ImGui::PushFont(lexend_regular_small);
	const char* db_path_txt = "db path:";
	ImVec2 db_path_txt_size = ImGui::CalcTextSize(db_path_txt);
	ImGui::SetCursorPos(ImVec2((drawing_area_size.x - db_path_txt_size.x) * 0.5f + 3.0f, ImGui::GetCursorPosY() + 75.0f));
	ImGui::Text(db_path_txt);
	ImGui::PopFont();

	ImGui::PushFont(lexend_bold_medium);
	static char db_path_buf[256] = "";
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(31, 31, 31).operator ImVec4());
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(201, 201, 201).operator ImVec4());

	ImGui::InputTextWithHint("##dbpathinput", db_path_hint.c_str(), db_path_buf, sizeof(db_path_buf), ImGuiInputTextFlags_None, NULL, NULL);
	
	ImGui::PopStyleColor(2);
	ImGui::PopFont();

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImColor(70, 148, 142).operator ImVec4());
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(44, 94, 91).operator ImVec4());
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(32, 74, 71).operator ImVec4());
	if (ImGui::Button("..."))
	{
		OPENFILENAME ofn;
		TCHAR szFile[MAX_PATH] = L"";
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = L"Lightbox database\0*.lbdb\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn) == TRUE)
		{
			std::string selected_path = helpers::tchar_to_utf8(ofn.lpstrFile);
			db_path_hint = selected_path;
			strcpy_s(db_path_buf, selected_path.c_str());
			cfg::_path = selected_path;
			
			
		}
	}

	ImGui::PopStyleVar();

	ImGui::PushFont(lexend_regular_small);
	const char* master_pw_txt = "master password:";
	ImVec2 master_pw_txt_size = ImGui::CalcTextSize(master_pw_txt);
	ImGui::SetCursorPos(ImVec2((drawing_area_size.x - master_pw_txt_size.x) * 0.5f + 3.0f, ImGui::GetCursorPosY() + 20.0f));
	ImGui::Text(master_pw_txt);
	ImGui::PopFont();

	ImGui::PushFont(lexend_bold_medium);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(31, 31, 31).operator ImVec4());
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(201, 201, 201).operator ImVec4());

	ImGui::InputTextWithHint("##masterpwinput", "********", master_pw_buf, sizeof(master_pw_buf), ImGuiInputTextFlags_Password, NULL, NULL);

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();
	ImGui::PopFont();

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 70.0f, ImGui::GetCursorPosY() + 5.0f));
	if (ImGui::Button("unlock"))
	{
		std::string master_pw_str(master_pw_buf);

		if (master_pw_str == "") 
		{
			logged_in = false;
			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();
			return;
		}
		/*TODO: AUTH*/

		if (helpers::file_exists(cfg::_path))
		{
			logged_in = true;
			//MessageBoxA(
			// NULL, enc::hash_password(master_pw_str).c_str(), "hey", NULL);
			pw_hash = enc::hash_password(master_pw_str);
			//helpers::to_clipboard(pw_hash); //WRRW
			db_content = enc::decrypt_enc_file(cfg::_path, pw_hash);
			creds = database::load_db(db_content);
			db_loaded = true;
			int sizzz = creds.size();
			if (creds.size() == 1 && creds.at(0).id == 0 && creds.at(0).user == "ERROR" && creds.at(0).password == "ERROR")
			{
				logged_in = false;
				db_loaded = false;
				clear_strings(false);
			}

			clear_strings(true);
		}
		
	}
	ImGui::SameLine();
	//ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 5.0f));
	if (ImGui::Button("create"))
	{
		std::string master_pw_str(master_pw_buf);
		pw_hash = enc::hash_password(master_pw_str);
		cfg::_path = database::new_db_path();
		enc::write_enc_file(pw_hash, database::default_db());
		db_content = enc::decrypt_enc_file(cfg::_path, pw_hash);
		creds = database::load_db(db_content);
		db_loaded = true;
		clear_strings(true);
		logged_in = true;
	}
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar();
}

void about_tab(ImVec2 drawing_area_size)
{
	ImGui::PushFont(lexend_regular_small);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
	ImGui::Text("lightbox is an open-source password\nmanager which was created in early\n2024, using argon2 for hashing and\nAES for database encryption, while\nthe UI side implements imgui.\nThis project is meant for learning, and\nit should be safe enough for offline\nstorage of credentials.\nHuge thanks to endenewi for coming\nup with the idea of this GUI, which I\nended up liking a lot.");


	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
	ImGui::SetCursorPosY(drawing_area_size.y - 55.0f);
	ImGui::PushStyleColor(ImGuiCol_Header, ImColor(255, 0, 0).operator ImVec4());
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

	ImGui::Text("libraries used: ");
	//ImGui::SameLine();
	ImVec2 argon_txt_size = ImGui::CalcTextSize("- argon2 (phc-winner-argon2)");
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
	if (ImGui::Selectable("- argon2 (phc-winner-argon2)", false, ImGuiSelectableFlags_None, argon_txt_size))
	{
		ShellExecute(NULL, L"open", L"https://github.com/P-H-C/phc-winner-argon2", NULL, NULL, SW_SHOWNORMAL);
	}

	ImVec2 aes_txt_size = ImGui::CalcTextSize("- AES (SergeyBel)");
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
	if (ImGui::Selectable("- AES (SergeyBel)", false, ImGuiSelectableFlags_None, argon_txt_size))
	{
		ShellExecute(NULL, L"open", L"https://github.com/SergeyBel/AES", NULL, NULL, SW_SHOWNORMAL);
	}

	ImVec2 imgui_txt_size = ImGui::CalcTextSize("- Dear ImGui (ocornut)");
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
	if (ImGui::Selectable("- Dear ImGui (ocornut)", false, ImGuiSelectableFlags_None, imgui_txt_size))
	{
		ShellExecute(NULL, L"open", L"https://github.com/ocornut/imgui", NULL, NULL, SW_SHOWNORMAL);
	}

	ImVec2 json_txt_size = ImGui::CalcTextSize("- json (nlohmann)");
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
	if (ImGui::Selectable("- json (nlohmann)", false, ImGuiSelectableFlags_None, json_txt_size))
	{
		ShellExecute(NULL, L"open", L"https://github.com/nlohmann/json", NULL, NULL, SW_SHOWNORMAL);
	}

	ImGui::PopStyleColor(3);
	ImGui::PopFont();
	
}

void settings_tab(ImVec2 drawing_area_size)
{
	ImGui::PushFont(lexend_regular_small);
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 10.0f, ImGui::GetCursorPosY() + 15.0f));
	ImGui::Text("window border");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.5f, 0.5f));
	
	if (ImGui::ColorEdit3("##wndborderpicker", wnd_border_color_f, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview))
	{
		std::string color_str = std::to_string(wnd_border_color_f[0]) + "," + std::to_string(wnd_border_color_f[1]) + "," + std::to_string(wnd_border_color_f[2]);
		cfg::update_color(cfg::WND_BORDER_COLOR, color_str);
	}
	ImGui::PopStyleVar(2);
}

bool is_copying = false;
int opening_copying = 0; //0 = none, 1 = opening, 2 = closing
float copy_rect_max_x = 343.0f;
std::string creds_status = "ready";

void creds_table(ImDrawList* draw_list)
{
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

	ImVec2 rect_min = ImVec2(p.x + 10.0f, p.y);
	ImVec2 rect_max = ImVec2(p.x + 265.0f, p.y + 300.0f); // calc y based on creds size

	draw_list->AddRectFilled(rect_min, rect_max, IM_COL32(41, 43, 43, 255), 5.0f);

	draw_list->AddRectFilled(rect_min, ImVec2(rect_max.x, rect_min.y + 20.0f), IM_COL32(69, 69, 69, 255), 5.0f);
	draw_list->AddRect(rect_min, ImVec2(rect_max.x, rect_min.y + 20.0f), IM_COL32(70, 128, 128, 255), 5.0f);

	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 55.0f, ImGui::GetCursorPosY()));

	ImGui::PushFont(lexend_regular_small);
	ImGui::Text("title");
	ImGui::SameLine();

	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 75.0f, ImGui::GetCursorPosY()));
	ImGui::Text("username");

	ImGui::PopFont();
		
	draw_list->AddRect(rect_min, rect_max, IM_COL32(70, 128, 128, 255), 5.0f);
	float row_height = 20.0f;

	for (int i = 1; i < creds.size() + 1; i++)
	{
		//	ImVec2 rect_max = ImVec2(rect_min.x + ImGui::GetContentRegionAvailWidth(), rect_min.y + row_height);
		ImVec2 rect_min_d = ImVec2(rect_min.x + 2.0f, rect_min.y + (row_height * i));
		ImVec2 rect_max_d = ImVec2(rect_max.x - 2.0f, rect_min.y + row_height + (row_height * i));

		ImVec2 rect_min_next;
		ImVec2 rect_max_next;

		if (i < creds.size())
		{
			rect_min_next = ImVec2(rect_min.x + 2.0f, rect_min.y + (row_height * (i + 1)));
			rect_max_next = ImVec2(rect_max.x - 2.0f, rect_min.y + row_height + (row_height * (i + 1)));
		}
		
		if (db_loaded)
		{
			
			if (i < 14)
			{
				ImGui::PushFont(lexend_regular_small);
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 15.0f, ImGui::GetCursorPosY() - 1.0f));
				ImGui::Text((creds.at(i - 1).title.size() > 12) ? std::string(creds.at(i - 1).title.substr(0, 10) + "...").c_str() : creds.at(i - 1).title.c_str());
				//ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 15.0f, ImGui::GetCursorPosY() - 1.0f));

				ImGui::SameLine();
				ImGui::SetCursorPosX(125.0f);
				(ImGui::IsMouseHoveringRect(rect_min_d, rect_max_d) && creds.at(i - 1).user.size() > 6) ? ImGui::Text(std::string(creds.at(i - 1).user.substr(0, 6) + "...").c_str()) : ImGui::Text(creds.at(i - 1).user.c_str());
				ImGui::PopFont();

				//ImGui::IsMouseHoveringRect(rect_min_d, rect_max_d) rect_min_d = rect_min_d : 

				if (ImGui::IsMouseHoveringRect(rect_min_d, rect_max_d))
				{
					ImGui::SameLine();
					ImGui::PushFont(icon_font_regular);
					ImGui::SetCursorPos(ImVec2(195.0f, ImGui::GetCursorPosY() + 2.0f));
					ImVec2 copy_rect_min = ImVec2(323.0f, ImGui::GetCursorPosY() + row_height - 2.5f);
					ImVec2 copy_rect_max = ImVec2(copy_rect_max_x, ImGui::GetCursorPosY() + 15.0f + row_height);
					draw_list->AddRectFilled(copy_rect_min, copy_rect_max, IM_COL32(82, 91, 92, 255), 8.0f);
					if (!is_copying)
					{
						ImGui::Text("A"); //copy
						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
							is_copying = !is_copying;
							if (is_copying && opening_copying == 0)
								opening_copying = 1;
							else if (!is_copying && opening_copying == 0)
								opening_copying = 2;
						}
					}
					else {
						ImGui::Text("E"); //copy user
						helpers::to_clipboard(creds.at(i - 1).user);
						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
							is_copying = !is_copying;
							if (opening_copying == 0)
								opening_copying = 2;
							creds_status = "username copied!";
						}
					}
					
					if (opening_copying == 1)
					{
						copy_rect_max_x += 3.0f;
						if (copy_rect_max_x > 382.0f)
						{
							opening_copying = 0;
						}
					}
					if (opening_copying == 2)
					{
						copy_rect_max_x -= 3.0f;
						if (copy_rect_max_x <= 343.0f)
						{
							opening_copying = 0;
							copy_rect_max_x = 343.0f;
						}
					}
					if (!is_copying)
					{
						ImGui::SameLine();
						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() - 12.0f, ImGui::GetCursorPosY() + 2.8f));
						ImGui::Text("B"); //edit
						ImGui::SameLine();
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.8f);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 14.0f);
						ImGui::Text("C"); //remove
						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
							db_content = database::remove_credential(creds, creds.at(i - 1), i - 1, db_content);
							creds_status = "removed credential!";
						}
					}
					else {
						ImGui::SameLine();
						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() - 12.0f, ImGui::GetCursorPosY() + 2.8f));
						ImGui::Text("D"); //copy password
						helpers::to_clipboard(creds.at(i - 1).password);
						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
							is_copying = !is_copying;
							if (opening_copying == 0)
								opening_copying = 2;
							creds_status = "copied password! clearing clipboard in 10s...";
							std::thread clear_clipboard_thread(helpers::clear_clipboard);
							clear_clipboard_thread.detach();
						}
						ImGui::SameLine();
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 16.0f);
						ImGui::Text("F"); //back
						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
							is_copying = false;
							opening_copying = 2;
						}
					}
					//ImGui::SameLine();

					
					ImGui::PopFont();
					//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 1.0f);
				}
				else {

				}

			}
		}
		bool hovering_next = ImGui::IsMouseHoveringRect(rect_min_next, rect_max_next);
		if(i < 14) draw_list->AddLine(ImVec2(rect_min.x + 2.0f, rect_min.y + row_height + (row_height * i)), ImVec2(rect_max.x - 2.0f, rect_min.y + row_height + (row_height * i)), (ImGui::IsMouseHoveringRect(rect_min_d, rect_max_d) || hovering_next) ? IM_COL32(58, 87, 83, 255) : IM_COL32(56, 56, 56, 255), 2.0f);
		
		
		if (i == creds.size())
		{
			ImGui::SetCursorPos(ImVec2(210.0f, ImGui::GetCursorPosY() + 2.0f));
			ImGui::PushFont(icon_font_regular);
			ImGui::Text("I"); //save db
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				database::save_db(db_content, pw_hash);
				logged_in = false;
				creds.clear();
				ImGui::PopFont();
				ImGui::PopStyleVar();
				ImGui::PopStyleColor(3);
				db_loaded = false;
				helpers::clear_string(db_content);
				helpers::clear_string(pw_hash);
				strcpy_s(cred_user_buf, "");
				strcpy_s(cred_pw_buf, "");
				return;
			}
			ImGui::SameLine();
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() - 18.0f, ImGui::GetCursorPosY()));
			ImGui::Text("H"); //save db
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				database::save_db(db_content, pw_hash);
				creds_status = "database saved!";
			}
			ImGui::SameLine();
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() - 18.0f, ImGui::GetCursorPosY() + 0.1f));
			ImGui::Text("G"); //add credential
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				adding_cred = true;
			}
			ImGui::PopFont();
		} 
		//draw_list->AddLine
	}

	if (creds.size() == 0)
	{
		ImGui::SetCursorPos(ImVec2(210.0f, ImGui::GetCursorPosY() + 2.0f));
		ImGui::PushFont(icon_font_regular);
		ImGui::Text("I"); //save db
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			database::save_db(db_content, pw_hash);
			logged_in = false;
			creds.clear();
			ImGui::PopFont();
			ImGui::PopStyleVar();
			ImGui::PopStyleColor(3);
			db_loaded = false;
			db_content.clear();
			pw_hash.clear();
			strcpy_s(cred_user_buf, "");
			strcpy_s(cred_pw_buf, "");
			return;
		}
		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() - 18.0f, ImGui::GetCursorPosY()));
		ImGui::Text("H"); //save db
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			database::save_db(db_content, pw_hash);
			creds_status = "database saved!";
		}
		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() - 18.0f, ImGui::GetCursorPosY() + 0.1f));
		ImGui::Text("G"); //add credential
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			adding_cred = true;
		}
		ImGui::PopFont();
	}

	draw_list->AddLine(ImVec2(rect_min.x + 110.5f, rect_min.y), ImVec2(rect_min.x + 110.5f, rect_max.y - 2.0f), IM_COL32(78, 89, 89, 200), 2.0f);

	ImGui::PopStyleVar();
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 15.0f, 332.0f));

	ImVec2 status_rect_min = ImVec2(rect_min.x, 350.0f);
	ImVec2 status_rect_max = ImVec2(rect_max.x, 370.0f);
	//ImGui::PopFont();
	ImGui::PushFont(lexend_regular_small);
	ImGui::Text(std::string(std::string((cfg::db_name.size() > 10) ? std::string("database: " + cfg::db_name.substr(0, 10) + "...").c_str() : "database: " + cfg::db_name) + " // count: " + std::to_string(creds.size())).c_str());
	ImGui::PushFont(lexend_regular_smallest);

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 12.0f);
	ImGui::TextColored(ImColor(76, 173, 186, 255), creds_status.c_str());
	ImGui::PopFont();
	draw_list->AddRectFilled(status_rect_min, status_rect_max, IM_COL32(41, 43, 43, 255), 5.0f);
	ImGui::PopFont();
	//ImGui::Dummy(ImVec2(100, 600.0f)); 
}

void db_tab(ImVec2 drawing_area_size, ImDrawList* draw_list)
{
	if (db_loaded)
	{
		creds_table(draw_list);
	}
}

int pw_char_opt = 0;
bool all_char_opt = true;
bool lowercase_char_opt = false;
bool uppercase_char_opt = false;
bool letters_char_opt = false;
bool numbers_letters_opt = false;

void add_cred_wnd(ImDrawList* draw_list)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, ImColor(70, 148, 142).operator ImVec4());
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(44, 94, 91).operator ImVec4());
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(32, 74, 71).operator ImVec4());
	ImGui::PushFont(lexend_regular_small);
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 10.0f, ImGui::GetCursorPosY() + 7.0f));
	ImGui::Text("title:");
	ImGui::SameLine();
	ImGui::PopFont();

	ImGui::PushFont(lexend_bold_medium);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(31, 31, 31).operator ImVec4());
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(201, 201, 201).operator ImVec4());

	ImGui::SetCursorPos(ImVec2(80.0f, ImGui::GetCursorPosY() - 3.0f));
	ImGui::InputTextWithHint("##credtitleinput", "title", cred_title_buf, sizeof(cred_title_buf));

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();
	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 10.0f, ImGui::GetCursorPosY() + 10.0f));
	ImGui::PushFont(lexend_regular_small);
	ImGui::Text("user:");
	ImGui::SameLine();
	ImGui::PopFont();

	ImGui::PushFont(lexend_bold_medium);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(31, 31, 31).operator ImVec4());
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(201, 201, 201).operator ImVec4());

	ImGui::SetCursorPos(ImVec2(80.0f, ImGui::GetCursorPosY() - 3.0f));
	ImGui::InputTextWithHint("##creduserinput", "example@example", cred_user_buf, sizeof(cred_user_buf));

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();
	ImGui::PopFont();

	ImGui::PushFont(lexend_regular_small);
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 10.0f, ImGui::GetCursorPosY() + 7.0f));
	ImGui::Text("pass:");
	ImGui::SameLine();
	ImGui::PopFont();
	ImGui::PushFont(lexend_bold_medium);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(31, 31, 31).operator ImVec4());
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(201, 201, 201).operator ImVec4());

	ImGui::SetCursorPos(ImVec2(80.0f, ImGui::GetCursorPosY() - 3.0f));
	ImGui::InputTextWithHint("##credpwinput", "********", cred_pw_buf, sizeof(cred_pw_buf), ImGuiInputTextFlags_Password, NULL, NULL);

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();
	ImGui::PopFont();

	ImVec2 pw_gen_rect_min = ImVec2(205.0f, 150.0f);
	ImVec2 pw_gen_rect_max = ImVec2(pw_gen_rect_min.x + 170.0f, pw_gen_rect_min.y + 60.0f);
	draw_list->AddRectFilled(pw_gen_rect_min, pw_gen_rect_max, IM_COL32(66, 66, 66, 255), 5.0f);
	ImGui::PushFont(lexend_regular);

	ImGui::SetCursorPos(ImVec2(125.0f, 130.0f));
	ImGui::Text("password:");
	ImGui::PopFont();

	ImGui::PushFont(lexend_regular_small);
	ImGui::SetCursorPos(ImVec2(85.0f, 155.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(81, 120, 117).operator ImVec4());
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImColor(109, 163, 159).operator ImVec4());

	all_char_opt = (pw_char_opt == 0);
	letters_char_opt = (pw_char_opt == 1);
	numbers_letters_opt = (pw_char_opt == 4);

	if (ImGui::Checkbox("all", &all_char_opt))
	{
		pw_char_opt = 0;
		
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("all letters", &letters_char_opt))
	{
		pw_char_opt = 1;
	}

	ImGui::SetCursorPos(ImVec2(85.0f, 175.0f));
	if (ImGui::Checkbox("numbers+letters", &numbers_letters_opt))
	{
		pw_char_opt = 4;
	}

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();
	ImGui::PopFont();
	//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 300.0f);
	ImGui::SetCursorPos(ImVec2(80.0f, pw_gen_rect_max.y - 10.0f));
	ImGui::PushFont(lexend_regular_small);

	if (ImGui::Button("generate"))
	{
		strcpy_s(cred_pw_buf, pw::random_pw(pw_char_opt, 30).c_str());
		creds_status = "generated password!";
	}

	ImGui::SameLine();

	if (ImGui::Button("add"))
	{
		credential new_cred;
		new_cred.id = creds.size() + 50;
		new_cred.user = cred_user_buf;
		new_cred.password = cred_pw_buf;
		new_cred.title = cred_title_buf;
		db_content = database::add_credential(creds, new_cred, db_content);
		clear_strings(true);
		adding_cred = false;
	}

	ImGui::SameLine();

	if (ImGui::Button("cancel"))
	{
		clear_strings(true);

		adding_cred = false;
	}

	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::PopStyleVar();
}

void debug_tab(ImVec2 drawing_area_size, ImDrawList* draw_list)
{
	//creds_table(draw_list);
}


namespace gui {
	bool closed = false;
	std::vector<float> wnd_border_color;

	bool render_gui(GLFWwindow* window)
	{
		int width, height;
		glfwGetWindowSize(window::_window, &width, &height);
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width), static_cast<float>(height)));
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(33, 33, 33).operator ImVec4());

		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(wnd_border_color_f[0], wnd_border_color_f[1], wnd_border_color_f[2], 1.0f));

		ImGui::Begin("lightbox", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		
		/*
		if (ImGui::IsMouseHoveringRect(ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x, 35.0f))) {
			if (ImGui::IsMouseClicked(0)) {
				dragging = true;
				dragStart = ImGui::GetMousePos();
				ImVec2 mousePos = ImGui::GetMousePos();
			}
		}

		if (ImGui::IsMouseReleased(0)) {
			dragging = false;
		}

		if (dragging) {
			ImVec2 delta = ImGui::GetMouseDragDelta(0.0f);
			int win_x, win_y;
			glfwGetWindowPos(window, &win_x, &win_y);
			glfwSetWindowPos(window, static_cast<float>(win_x) + delta.x, static_cast<float>(win_y) + delta.y);
		}*/
		
		//const bool dragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
		//glfwSwapInterval(dragging ? 0 : 1);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImVec2 start_pos = ImVec2(ImGui::GetCursorPosX() + 125.0f, ImGui::GetCursorPosY() + 15.0f); 
		ImVec2 end_pos = ImVec2(start_pos.x, start_pos.y + 368.0f); 

		draw_list->AddLine(start_pos, end_pos, IM_COL32_WHITE);

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 13.0f);

		ImGui::PushFont(lexend_bold);
		ImGui::Text("lightbox");
		ImGui::PopFont();

		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
		ImGui::PushFont(lexend_regular);

		ImVec2 login_txt_size = ImGui::CalcTextSize(login_txt.c_str());
		ImVec2 logged_in_txt_size = ImGui::CalcTextSize(logged_in_txt.c_str());

		if (!logged_in)
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 47.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 80.0f);
			if (menu_opt == 0)
			{
				glow_dot();
			}
			
			ImGui::PushStyleColor(ImGuiCol_Text, (menu_opt == 0) ? IM_COL32(255, 255, 255, 255) : IM_COL32(148, 148, 148, 255));
			if (ImGui::Selectable(login_txt.c_str(), false, ImGuiSelectableFlags_None, login_txt_size))
			{
				menu_opt = 0;
			}
			ImGui::PopStyleColor();

		}
		else {
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 29.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 80.0f);
			if (menu_opt == 0)
			{
				glow_dot();
			}
			ImGui::PushStyleColor(ImGuiCol_Text, (menu_opt == 0) ? IM_COL32(255, 255, 255, 255) : IM_COL32(148, 148, 148, 255));
			if (ImGui::Selectable(logged_in_txt.c_str(), false, ImGuiSelectableFlags_None, logged_in_txt_size))
			{
				menu_opt = 0;
			}
			ImGui::PopStyleColor();
		}
		

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 35.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 35.0f);
		if (menu_opt == 1)
		{
			glow_dot();
		}
		ImVec2 settings_txt_size = ImGui::CalcTextSize(settings_txt.c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, (menu_opt == 1) ? IM_COL32(255, 255, 255, 255) : IM_COL32(148, 148, 148, 255));
		if (ImGui::Selectable(settings_txt.c_str(), false, ImGuiSelectableFlags_None, settings_txt_size))
		{
			menu_opt = 1;
		}
		ImGui::PopStyleColor();

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 35.0f);
		if (menu_opt == 2)
		{
			glow_dot();
		}
		ImVec2 debug_txt_size = ImGui::CalcTextSize(debug_txt.c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, (menu_opt == 2) ? IM_COL32(255, 255, 255, 255) : IM_COL32(148, 148, 148, 255));
		if (ImGui::Selectable(debug_txt.c_str(), false, ImGuiSelectableFlags_None, debug_txt_size))
		{
			menu_opt = 2;
		}
		ImGui::PopStyleColor();

		ImVec2 window_size = ImGui::GetWindowSize();
		ImVec2 about_txt_size = ImGui::CalcTextSize(about_txt.c_str());
		ImVec2 about_pos = ImVec2(ImGui::GetCursorPosX() + 43.0f, window_size.y - about_txt_size.y - 15.0f);
		ImGui::SetCursorPos(about_pos);
		if (menu_opt == 3)
		{
			glow_dot();
		}

		ImGui::PushStyleColor(ImGuiCol_Text, (menu_opt == 3) ? IM_COL32(255, 255, 255, 255) : IM_COL32(148, 148, 148, 255));
		if (ImGui::Selectable(about_txt.c_str(), false, ImGuiSelectableFlags_None, about_txt_size))
		{
			menu_opt = 3;
		}
		ImGui::PopStyleColor();

		ImGui::PopFont();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImVec2 drawing_area_pos = ImVec2(ImGui::GetWindowContentRegionMin().x + 125.0f, ImGui::GetWindowContentRegionMin().y + 18.0f); 
		ImVec2 drawing_area_size = ImVec2(260, 380); 

		ImGui::SetCursorPos(drawing_area_pos);
		ImGui::BeginChild("uidrawzone", drawing_area_size, false, ImGuiWindowFlags_NoScrollbar);
		ImGui::PushFont(lexend_bold_medium);

		switch (menu_opt)
		{
			case 0: {
				if (!logged_in)
				{
					ImGui::SetCursorPosX((drawing_area_size.x - login_txt_size.x) * 0.5f);
					ImGui::Text(login_txt.c_str());
					login_tab(drawing_area_size);
				}
				else {
					if (!adding_cred)
					{
						ImGui::SetCursorPosX((drawing_area_size.x - logged_in_txt_size.x) * 0.5f);
						ImGui::Text(logged_in_txt.c_str());
						db_tab(drawing_area_size, draw_list);
					}
					else {
						ImVec2 add_cred_txt_size = ImGui::CalcTextSize(add_cred_txt.c_str());
						ImGui::SetCursorPosX((drawing_area_size.x - add_cred_txt_size.x) * 0.5f);
						ImGui::Text(add_cred_txt.c_str());
						//db_tab(drawing_area_size, draw_list);
						add_cred_wnd(draw_list);
					}
				}
			}break;
			case 1: {
				ImGui::SetCursorPosX((drawing_area_size.x - settings_txt_size.x) * 0.5f);
				ImGui::Text(settings_txt.c_str());
				settings_tab(drawing_area_size);
			}break;
			case 2: {
				ImGui::SetCursorPosX((drawing_area_size.x - debug_txt_size.x) * 0.5f);
				ImGui::Text(debug_txt.c_str());
				//debug_tab(drawing_area_size, draw_list);
			}break;
			case 3: {
				ImGui::SetCursorPosX((drawing_area_size.x - about_txt_size.x) * 0.5f);
				ImGui::Text(about_txt.c_str());
				about_tab(drawing_area_size);
			}
		}
		ImGui::PopFont();

		
		ImGui::EndChild();

		wnd_controls(draw_list);

		ImGui::End();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
		return true;
	}

	void init()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window::_window, true);
		ImGui_ImplOpenGL3_Init("#version 130");

		glfwMakeContextCurrent(window::_window);

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 15.0f;

		io.Fonts->AddFontDefault();

		lexend_bold = io.Fonts->AddFontFromMemoryCompressedTTF(lexend_bold_compressed_data, lexend_bold_compressed_size, 25);
		lexend_bold_medium = io.Fonts->AddFontFromMemoryCompressedTTF(lexend_bold_compressed_data, lexend_bold_compressed_size, 20);
		lexend_bold_small = io.Fonts->AddFontFromMemoryCompressedTTF(lexend_bold_compressed_data, lexend_bold_compressed_size, 18);
		lexend_regular = io.Fonts->AddFontFromMemoryCompressedTTF(lexend_regular_compressed_data, lexend_regular_compressed_size, 20);
		lexend_bold_tab = io.Fonts->AddFontFromMemoryCompressedTTF(lexend_bold_compressed_data, lexend_bold_compressed_size, 20);
		lexend_regular_small = io.Fonts->AddFontFromMemoryCompressedTTF(lexend_regular_compressed_data, lexend_regular_compressed_size, 17);
		icon_font_regular = io.Fonts->AddFontFromMemoryCompressedTTF(icon_font_compressed_data, icon_font_compressed_size, 15);
		lexend_regular_smallest = io.Fonts->AddFontFromMemoryCompressedTTF(lexend_regular_compressed_data, lexend_regular_compressed_size, 11);

		wnd_border_color_f[0] = wnd_border_color[0];
		wnd_border_color_f[1] = wnd_border_color[1];
		wnd_border_color_f[2] = wnd_border_color[2];
	}

	void newframe()
	{
		glfwMakeContextCurrent(window::_window);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void render_swap()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGui::EndFrame();

		glfwSwapBuffers(window::_window);


		glfwPollEvents();
	}
}
