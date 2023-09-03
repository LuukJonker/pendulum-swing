#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <deque>
#include <tuple>
#include <thread>

int width = 1024;
int height = 1024;
bool playing = true;
int max_trace_length = 100;
bool lines = true;
int calc_per_frame = 1;

#define PI 3.142857

class Part {
    public:
        double l1;
        double a1;
        double v1;
        double m1;

        double l2;
        double a2;
        double v2;
        double m2;

        std::deque<std::tuple<double, double>> traces;

        double r, g, b, a;

    Part(double an1, double an2, double red, double green, double blue) {
        l1 = 250;
        a1 = an1;
        v1 = 0;
        m1 = l1 / 100;

        l2 = 250;
        a2 = an2;
        v2 = 0;
        m2 = l2 / 100;

        r = red;
        g = green;
        b = blue;
        a = 1;
    }

    void update() {
        if (max_trace_length) {
            if (traces.size() >= max_trace_length) {
                traces.pop_front();
            }

            traces.push_back(std::make_tuple(width / 2 + sin(a1) * l1 + sin(a2) * l2, height / 2 + cos(a1) * l1 + cos(a2) * l2));
        }

        double g = 9.81;
        double temp_v = v1;
        double step_size = 0.15;
        v1 += (-g * (2 * m1 + m2) * sin(a1) - m2 * g * sin(a1 - 2 * a2) - 2 * sin(a1 - a2) * m2 * (v2 * v2 * l2 + v1 * v1 * l1 * cos(a1 - a2))) / (l1 * (2 * m1 + m2 - m2 * cos(2 * a1 - 2 * a2))) * step_size;
        v2 += (2 * sin(a1 - a2) * (temp_v * temp_v * l1 * (m1 + m2) + g * (m1 + m2) * cos(a1) + v2 * v2 * l2 * m2 * cos(a1 - a2))) / (l1 * (2 * m1 + m2 - m2 * cos(2 * a1 - 2 * a2))) * step_size;

        a1 += v1 * step_size;
        a2 += v2 * step_size;
    }

    /**
     * @brief Display the pendulum on the screen
     */
    void display() {
        glColor4d(r, g, b, a);
        glPointSize(10.0);
        glBegin(GL_LINES);
        glVertex2d(width / 2, height / 2);
        glVertex2d(width / 2 + sin(a1) * l1, height / 2 + cos(a1) * l1);
        glVertex2d(width / 2 + sin(a1) * l1, height / 2 + cos(a1) * l1);
        glVertex2d(width / 2 + sin(a1) * l1 + sin(a2) * l2, height / 2 + cos(a1) * l1 + cos(a2) * l2);
        glEnd();
    }

    /**
     * @brief Display the traces of the pendulum on the screen
     */
    void display_traces() {
        glColor4d(r / 2, g / 2, b / 2, a);

        if (lines) {
            glBegin(GL_LINES);
            bool first = true;

            int last_x, last_y;

            for (auto[x, y] : traces) {

                if (first) {
                    last_x = x;
                    last_y = y;
                    first = false;
                    continue;
                }

                glVertex2d(last_x, last_y);
                glVertex2d(x, y);

                last_x = x;
                last_y = y;
            }
        } else {
            glPointSize(2.0);
            glBegin(GL_POINTS);
            for (auto[x, y] : traces) {
                glVertex2d(x, y);
            }
        }
        glEnd();
    }
};


std::vector<Part *> parts;

void draw_fps_counter() {
    static int frame_count = 0;
    static int last_time = 0;
    static char fps[16];

    frame_count++;
    int time = glutGet(GLUT_ELAPSED_TIME);

    if (time - last_time > 1000) {
        sprintf(fps, "FPS: %4.2f", frame_count * 1000.0 / (time - last_time));
        last_time = time;
        frame_count = 0;
    }

    glColor3f(0, 1, 0);

    glRasterPos2f(10, 10);
    for (char *c = fps; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
    }
}

void draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (Part *p : parts) {
        p->display();
        if (max_trace_length) {
            p->display_traces();
        }
    }

    draw_fps_counter();

    glutSwapBuffers();
}



void update() {
    if (!playing) { return; }

    const int num_threads = 6;
    // Multithreaded the for loop
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int i = 0; i < num_threads; i++) {
        threads.push_back(std::thread([i, num_threads]() {
            for (int j = i; j < parts.size(); j += num_threads) {
                for (int k = 0; k < calc_per_frame; k++) {
                    parts[j]->update();
                }
            }
        }));
    }

    for (std::thread &t : threads) {
        t.join();
    }

    glutPostRedisplay();
}

void buttons(unsigned char key, int x, int y) {
    switch (key) {
        case ' ':
            playing = !playing;
            break;
        case 't':
            if (max_trace_length) { max_trace_length = 0; }
            else { max_trace_length = 100; }
        case 'l':
            lines = !lines;
            glutPostRedisplay();
            break;
        case '.':
            calc_per_frame++;
            break;
        case ',':
            if (calc_per_frame > 1)
            {
                calc_per_frame--;
            }
            break;
    }
}

void glut_init(int *argc, char **argv) {
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

    glutInitWindowSize(width, height);
    glutCreateWindow("Pendulum swing");

    glClearColor(0, 0, 0, 0);
    gluOrtho2D(0, width, height, -10);

    glutIdleFunc(update);
    glutDisplayFunc(draw);
    glutKeyboardFunc(buttons);
}

void gen_rainbow(double a1, double a2, int i, int max) {
    double angle = (double) i / max * 360;
    double red, green, blue;

    if (angle<60) {red = 255; green = round(angle*4.25-0.01); blue = 0;} else
    if (angle<120) {red = round((120-angle)*4.25-0.01); green = 255; blue = 0;} else
    if (angle<180) {red = 0, green = 255; blue = round((angle-120)*4.25-0.01);} else
    if (angle<240) {red = 0, green = round((240-angle)*4.25-0.01); blue = 255;} else
    if (angle<300) {red = round((angle-240)*4.25-0.01), green = 0; blue = 255;} else
                    {red = 255, green = 0; blue = round((360-angle)*4.25-0.01);}

    red /= 255;
    green /= 255;
    blue /= 255;

    Part *p = new Part(a1, a2, red, green, blue);
    parts.push_back(p);
}

int main(int argc, char **argv) {
    glut_init(&argc, argv);

    int num = 10000;

    for (int i = 0; i < num; i++) {
        gen_rainbow(3* PI/ 2, PI + i * 1E-10, i, num);
    }

    glutMainLoop();
}
