#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace window {
	extern GLFWwindow* _window;
	extern int _w, _h;
	void create_window(unsigned int window_w, unsigned int window_h);

}