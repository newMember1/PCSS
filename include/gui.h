#ifndef GUI_H
#define GUI_H

#include <GLFW/glfw3.h>

class GUI
{
public:
    static void init(GLFWwindow *window);
    static void render();
    static int getLightSize();

private:
    static int lightSize;
};

#endif