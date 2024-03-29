#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

namespace gui {
	extern bool closed;
	void init();
	bool render_gui(GLFWwindow* window);
	void newframe();
	void render_swap();

	extern std::vector<float> wnd_border_color;
}