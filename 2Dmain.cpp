#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>

const float rs = 60.0f;

void drawCircle(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 100; i++) {
        float a = 2.0f * M_PI * i / 100;
        float x = r * cosf(a);
        float y = r * sinf(a);
        glVertex2f(cx + x, cy + y);
    }
    glEnd();
}

void store_ray(float b, std::vector<float>& path){
    float x = -400, y = b;
    float vx = 1, vy = 0;
    float dt = 0.05f;

    for (int i = 0; i < 40000; i++){
        path.push_back(x);
        path.push_back(y);

        float r = sqrt(x*x + y*y);
        if (r < rs) break; // stop if inside the black hole
        if (x > 400 || y > 300 || y < -300) break;

        float h = x*vy - y*vx; // angular momentum (cross product)
        float factor = -1.5f * rs * h*h / (r*r*r*r*r); // gravitational lensing factor
        vx += factor * x * dt;
        vy += factor * y * dt;

        float v = sqrt(vx*vx + vy*vy);
        vx /= v; // normalize velocity
        vy /= v;

        x+= vx*dt;
        y+= vy*dt;
    }
}

void trace_ray(const std::vector<float>& p, int n){
    int total = p.size()/2;
    int count = (n<total)? n: total;

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < count; i++){
        glVertex2f(p[2*i], p[2*i+1]);
    }
    glEnd();

    if (count > 0 && count < total){
        glPointSize(5);
        glBegin(GL_POINTS);
        glVertex2f(p[2*(count - 1)], p[2*(count - 1)+1]);
        glEnd();
    }
}

int main() {
    glfwInit();
    std::vector<std::vector<float>> paths;
    for (float b = -280; b <= 280; b += 20) {
        paths.emplace_back();
        store_ray(b, paths.back());
    }
    int frame = 0;

    GLFWwindow* window = glfwCreateWindow(1600, 1200, "Lensing 2D", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-400, 400, -300, 300, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // black hole
        glColor3f(1.0f, 0.3f, 0.3f);
        drawCircle(0, 0, rs);

        // trace rays
        frame++;
        int n = frame * 15;
        if (n > 40100) frame = 0;

        glColor3f(1.0f, 1.0f, 1.0f);
        for (auto& p : paths){
            trace_ray(p, n);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

    }
    glfwTerminate();
}