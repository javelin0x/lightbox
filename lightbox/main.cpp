#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <vector>
#include <cctype>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "gui/window.h"
#include "gui/gui.h"

#include "cfg/cfg.h"
#include "ui/ui.h"
#include "lang/lang.h"

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
) {
    //ui::init();
    cfg::init();
    lang::set_english();

    window::create_window(400, 400);

    gui::init();
    bool closing = false;

    while (!gui::closed)
    {
        gui::newframe();

        gui::render_gui(window::_window);

        gui::render_swap();
    }

    glfwTerminate();


	return 0;
}