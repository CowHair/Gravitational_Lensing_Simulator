#include <glad/gl.h>
#include <GLFW/glfw3.h>

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(800, 600, "Lensing", nullptr, nullptr);
    glfwMakeContextCurrent(win);
    gladLoadGL(glfwGetProcAddress);

    while (!glfwWindowShouldClose(win)) {
        glClearColor(0.05f, 0.05f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(win);
        glfwPollEvents();

    }
    glfwTerminate();
}