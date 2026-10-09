#include <GLFW/glfw3.h>
#include <cmath>

const float PI = 3.14159265f;
const float rs = 60.0f;

void drawCircle(float cx, float cy, float r) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++) {
        float a = 2.0f * PI * i / 100;
        glVertex2f(cx + r * cos(a), cy + r * sin(a));
    }
    glEnd();
}

void trac_ray(float b){
    float x = -400, y = b;
    float vx = 1, vy = 0;
    float dt = 0.05f;

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 40000; i++){
        glVertex2f(x, y);

        float r = sqrt(x*x + y*y);
        if (r < rs) break; // stop if inside the black hole
        if (x > 400 || y > 300 || y < -300) break;

        float h = x*vy - y*vx; // angular momentum (cross product)
        float factor = -1.5f * rs * h*h / (r*r*r*r*r); // gravitational lensing factor
        float ax = factor * x;
        float ay = factor * y;

        vx += ax * dt;
        vy += ay * dt;

        float v = sqrt(vx*vx + vy*vy);
        vx /= v; // normalize velocity
        vy /= v;

        x += vx * dt;
        y += vy * dt;
    }
    glEnd();
}

int main() {
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(1600, 1200, "Lensing 2D", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // world coordinates: x from -400..400, y from -300..300, center at (0,0)
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

        glColor3f(1.0f, 1.0f, 1.0f);
        for (float b = -280; b <= 280; b += 20) {
            trac_ray(b);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

    }
    glfwTerminate();
}