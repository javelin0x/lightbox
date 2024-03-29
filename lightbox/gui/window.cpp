#include "Window.h"

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

namespace window {
    GLFWwindow* _window;
    int _w, _h;

    void create_window(unsigned int window_w, unsigned int window_h)
    {
        GLFWwindow* window;

        if (!glfwInit())
            _window = NULL;

        int count;
        int monitor_x, monitor_y;

        GLFWmonitor** monitors = glfwGetMonitors(&count);
        const GLFWvidmode* video_mode = glfwGetVideoMode(monitors[0]);

        glfwGetMonitorPos(monitors[0], &monitor_x, &monitor_y);

        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, false);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

        window = glfwCreateWindow(window_w, window_h, "lightbox", NULL, NULL);
        if (!window)
        {
            glfwTerminate();
            _window = NULL;
        }

        glfwSetWindowPos(window, monitor_x + (video_mode->width - window_w) / 2, monitor_y + (video_mode->height - window_h) / 2);
        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            _window = NULL;
        }

        glViewport(0, 0, window_w, window_h);

        //GLFWimage images[1];
        //images[0].pixels = stbi_load("LOGO", &images[0].width, &images[0].height, 0, 4);
        //glfwSetWindowIcon(window, 1, images);
        //stbi_image_free(images[0].pixels);

       // _w = window_w;
        //_h = window_h;

        _window = window;
	}

    void destroy_window()
    {

    }
}