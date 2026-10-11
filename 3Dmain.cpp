#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>

const char* vertSrc = R"(
#version 330 core
const vec2 pos[6] = vec2[6](
    vec2(-1,-1), vec2( 1,-1), vec2( 1, 1),
    vec2(-1,-1), vec2( 1, 1), vec2(-1, 1)
);
void main() {
    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
}
)";

const char* fragSrc = R"(
#version 330 core
uniform vec2 uResolution;
out vec4 color;
const float PI = 3.1415926535897932384626433832795;
const float rs = 1.0;

vec3 sky(vec3 d){
    float lon = atan(d.x, -d.z);
    float lat = asin(d.y);

    float cell = PI/12.0;
    float cx = floor(lon/cell);
    float cy = floor(lat/cell);

    float checker = mod(cx + cy, 2.0);
    vec3 dark = vec3(0.05, 0.05, 0.12);
    vec3 light = vec3(0.35, 0.40, 0.65);

    return mix(dark, light, checker);


}


uniform vec3 uCamPos;

void main() {
    float fov = 1.0;
    vec3 forward = normalize(-uCamPos);
    vec3 right = normalize(cross(forward,vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(right, forward);

    vec2 uv = gl_FragCoord.xy / uResolution;
    vec2 p = uv * 2.0 - 1.0;
    float t = tan(fov/2.0);

    p.x *= uResolution.x / uResolution.y;

    vec3 direction = normalize(forward + right * p.x*t + up* p.y * t);

    vec3 pos = uCamPos;
    vec3 vel = direction;

    vec3 c = cross(pos, vel);
    float h2 = dot(c, c);

    bool fellIn  = false;
    for (int i = 0; i < 800; i++){
        float r = length(pos);

        if (r < rs){
            fellIn = true;
            break;
        }
        if (r > 40.0 && dot(pos, vel) > 0.0){
            break;
        }
        vec3 acc = -1.5 * rs * h2 / (r*r*r*r*r) * pos;
        float dt = 0.04 * r;
        vel += acc * dt;
        pos += vel * dt;
    }
    vec3 col = fellIn ? vec3(0.0) : sky(normalize(vel));
    color = vec4(col, 1.0);
}
)";


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
int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1200, 900, "Leninsg 3D", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glewExperimental = GL_TRUE;
    glewInit();

    GLuint prog = glCreateProgram();
    glAttachShader(prog, compile(GL_VERTEX_SHADER, vertSrc));
    glAttachShader(prog, compile(GL_FRAGMENT_SHADER, fragSrc));
    glLinkProgram(prog);

    
    GLuint vao;
    glGenVertexArrays(1, &vao);

    while (!glfwWindowShouldClose(window)){
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        glUseProgram(prog);
        glUniform3f(glGetUniformLocation(prog, "uCamPos"), 0.0f, 0.0f, 15.0f);
        GLint loc = glGetUniformLocation(prog, "uResolution");
        glUniform2f(loc, (float)w, (float)h);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}