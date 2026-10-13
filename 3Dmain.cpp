#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

float yaw = 0.0f, pitch = 0.0f, dist = 15.0f;
double lastX = 0.0, lastY = 0.0;
int skyMode = 1;
bool diskOn = true;
bool rk4On = false;
bool objOn = true;

std::string loadshader(const char* path){
    std::ifstream f(path);
    if (!f){printf("could not open %s\n", path); return "";}
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

GLuint compile(GLenum type, const char* src){
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint status;
    glGetShaderiv(s, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE){
        char buffer[1024];
        glGetShaderInfoLog(s, 1024, nullptr, buffer);
        printf("Shader compilation error: %s\n", buffer);
    }
    return s;
}

void cursorCallback(GLFWwindow* window, double xpos, double ypos){
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
        yaw -= (float)(xpos - lastX) * 0.005f;
        pitch += (float)(ypos - lastY) * 0.005f;
        if (pitch > 1.5f) pitch = 1.5f;
        if (pitch < -1.5f) pitch = -1.5f;
    }
    lastX = xpos;
    lastY = ypos;
}

void scrollCallback(GLFWwindow* window, double xoff, double yoff){
    dist -= (float)yoff * 1.0f;
    if (dist < 2.0f) dist = 2.0f;
    if (dist > 35.0f) dist = 35.0f;
}

void keyCallback(GLFWwindow* window, int key, int sc, int action, int mods){
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_1) skyMode = 1;
    if (key == GLFW_KEY_2) skyMode = 0;
    if (key == GLFW_KEY_D) diskOn = !diskOn;
    if (key == GLFW_KEY_R) rk4On = !rk4On;
    if (key == GLFW_KEY_O) objOn = !objOn;
}

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1200, 900, "Leninsg 3D", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);
    glewExperimental = GL_TRUE;
    glewInit();

    GLuint prog = glCreateProgram();
    std::string vs = loadshader("shaders/fullscreen.vert");
    std::string fs = loadshader("shaders/lensing.frag");

    glAttachShader(prog, compile(GL_VERTEX_SHADER, vs.c_str()));
    glAttachShader(prog, compile(GL_FRAGMENT_SHADER, fs.c_str()));
    glLinkProgram(prog);

    GLuint vao;
    glGenVertexArrays(1, &vao);

    while (!glfwWindowShouldClose(window)){
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        glUseProgram(prog);

        glUniform1i(glGetUniformLocation(prog, "uSky"), skyMode);
        glUniform1i(glGetUniformLocation(prog, "uDisk"), diskOn ? 1:0);
        glUniform1i(glGetUniformLocation(prog, "uRK4"), rk4On ? 1:0);
        glUniform1i(glGetUniformLocation(prog, "uObj"), objOn ? 1:0);

        float camX = dist * cos(pitch) * sin(yaw);
        float camY = dist * sin(pitch);
        float camZ = dist * cos(pitch) * cos(yaw);
        glUniform3f(glGetUniformLocation(prog, "uCamPos"), camX, camY, camZ);
        GLint loc = glGetUniformLocation(prog, "uResolution");
        glUniform2f(loc, (float)w, (float)h);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}