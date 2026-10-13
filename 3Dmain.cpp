#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>
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
uniform int uSky;
uniform int uDisk;
uniform int uRK4;
uniform int uObj;

float hash(vec2 p){
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

vec3 stars(vec3 d){
    float lon = atan(d.x, -d.z);
    float lat = asin(clamp(d.y, -1.0, 1.0));

    float cells = 300.0;
    vec2 g = vec2(lon, lat) * cells / (2.0 * PI);
    vec2 id = floor(g);
    vec2 f = fract(g);

    vec2 starPos = 0.2 + 0.6 * vec2(hash(id), hash(id + 1.0));
    float dist = length(f - starPos);
    float brightness = 1.0 - smoothstep(0.0, 0.15, dist);

    float has = step(0.8, hash(id+3.7));
    brightness *= has;

    float tint = hash(id + 7.0);
    vec3 starColor = mix(vec3(0.7, 0.8, 1.0), vec3(1.0, 0.9, 0.7), tint);

    return vec3(0.01, 0.01, 0.02) + starColor * brightness;
}

vec3 accel(vec3 p, float h2){
    float r = length(p);
    return -1.5 * rs * h2/(r*r*r*r*r)*p;
}

void main() {
    float fov = 1.0;
    vec3 forward = normalize(-uCamPos);
    vec3 right = normalize(cross(forward,vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(right, forward);

    bool diskHit = false;
    vec3 diskCol = vec3(0.0);

    vec2 uv = gl_FragCoord.xy / uResolution;
    vec2 p = uv * 2.0 - 1.0;
    float t = tan(fov/2.0);

    p.x *= uResolution.x / uResolution.y;

    vec3 direction = normalize(forward + right * p.x*t + up* p.y * t);

    vec3 pos = uCamPos;
    vec3 vel = direction;

    vec3 c = cross(pos, vel);
    float h2 = dot(c, c);

    // vec3 objC = vec3(0.0, 0.0, -25.0);
    vec3 objC = vec3(4.0, 0.0, -25.0);
    float objR = 2.0;
    bool objHit = false;
    vec3 objCol = vec3(0.0);

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
        float dt = 0.04 * r;
        vec3 oldPos = pos;

        if (uRK4 == 1){
            vec3 k1x = vel;
            vec3 k1v = accel(pos, h2);

            vec3 k2x = vel + k1v * dt *0.5;
            vec3 k2v = accel(pos + k1x * dt * 0.5, h2);

            vec3 k3x = vel + k2v * dt *0.5;
            vec3 k3v = accel(pos + k2x * dt * 0.5, h2);

            vec3 k4x = vel + k3v * dt;
            vec3 k4v = accel(pos + k3x * dt, h2);

            pos += (dt/6.0) * (k1x + 2.0*k2x + 2.0*k3x + k4x);
            vel += (dt/6.0) * (k1v + 2.0*k2v + 2.0*k3v + k4v);
        } else{
            vec3 acc = accel(pos,h2);
            vel += acc*dt;
            pos += vel*dt;
         }

        if (uDisk == 1 && oldPos.y * pos.y < 0.0){
            float s = oldPos.y/(oldPos.y - pos.y);
            vec3 hit = mix(oldPos, pos, s);
            float rh = length(hit);

            if (rh < 8.0 * rs && rh > 3.0 * rs){
                diskHit = true;
                float k = (rh - (3*rs)) / (rs * 5.0);

                float angle = atan(hit.z, hit.x);
                float streak = 0.75 + 0.25 * sin(angle*12.0 + rh*3.0);

                float exposure = 2.0;
                vec3 hot = vec3(1.0,0.85,0.55);
                vec3 cool = vec3(0.7, 0.15, 0.03);
                float brightness = pow(1.0 - k, 2.0);

                float beta = sqrt(rs/(2.0*rh));
                vec3 gasDir = normalize(cross(vec3(0.0,1.0,0.0), hit));
                vec3 lightDir = -normalize(vel);

                float D = 1.0/(1.0 - beta*dot(gasDir, lightDir));
                float beam = pow(D, 3.0);

                diskCol = mix(hot, cool, k)*brightness*streak*beam*exposure;
                break;
            }
        }
        
        if (uObj == 1 && length(pos - objC) < objR){
            objHit = true;
            vec3 n = normalize(pos - objC);
            float facing = dot(n, -normalize(vel));

            vec3 core = vec3(0.9, 0.95, 1.0);
            vec3 rim = vec3(0.25, 0.45, 1.0);
            objCol = mix(rim, core, pow(facing, 2.0)) * 1.0;

            float knots = 0.7 + 0.5 * hash(floor(n.xy*3.0));
            objCol *= knots;
            
            break;
        } 
    }

    vec3 bg = (uSky == 1) ? stars(normalize(vel)) : sky(normalize(vel));
    vec3 col = fellIn ? vec3(0.0) : (diskHit ? diskCol:(objHit ? objCol : bg));
    color = vec4(col, 1.0);
}
)";

float yaw = 0.0f, pitch = 0.0f, dist = 15.0f;
double lastX = 0.0, lastY = 0.0;
int skyMode = 1;
bool diskOn = true;
bool rk4On = false;
bool objOn = true;

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