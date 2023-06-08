#ifndef GUI_H
#define GUI_H

#include <GLFW/glfw3.h>

class GUI
{
public:
    static void init(GLFWwindow *window);
    static void render();
    static int getLightSize(){return lightSize;};

private:
    static int lightSize;
    static int sampleSize;
};

#endif