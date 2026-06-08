// HECHO POR IVAN SARDON MEDINA
#define GLAD_GL_IMPLEMENTATION

#include "animations.h"
#include "camera.h"
#include "draw2D.h"
#include "draw3D.h"
#include "cuboRubick.h"
#include "rubikControls.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <queue>
#include <sstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <min2phase/min2phase.h>

using namespace std;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Deteccion de flanco (una sola pulsacion por tecla)
static bool keyPressed(GLFWwindow* window, int key) {
    static bool prev[GLFW_KEY_LAST + 1] = {};
    const bool down    = glfwGetKey(window, key) == GLFW_PRESS;
    const bool pressed = down && !prev[key];
    prev[key] = down;
    return pressed;
}

// Convierte el string de solucion Kociemba a movimientos del cubo
static void parsearSolucion(const string& solucion, queue<pair<Camada,int>>& out) {
    while (!out.empty()) out.pop();
    stringstream ss(solucion);
    string mov;
    while (ss >> mov) {
        Camada cam;
        int baseDir;
        switch (mov[0]) {
            case 'U': cam = Up;    baseDir = -1; break;
            case 'R': cam = Right; baseDir = -1; break;
            case 'F': cam = Front; baseDir = -1; break;
            case 'D': cam = Down;  baseDir =  1; break;
            case 'L': cam = Left;  baseDir =  1; break;
            case 'B': cam = Back;  baseDir =  1; break;
            default: continue;
        }
        int dir  = baseDir;
        int reps = 1;
        if (mov.size() > 1) {
            if      (mov[1] == '\'') dir  = -baseDir;
            else if (mov[1] == '2')  reps = 2;
        }
        for (int i = 0; i < reps; ++i)
            out.push({cam, dir});
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    const unsigned int SCR_W = 900;
    const unsigned int SCR_H = 900;

    GLFWwindow* window = glfwCreateWindow(SCR_W, SCR_H, "Project_11", nullptr, nullptr);
    if (!window) {
        cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    RubickCube rubickCube(0.55f, 0.53f, 0.45f);

    // Solver Kociemba: init en segundo plano
    atomic<bool> solverListo(false);
    thread([&solverListo]() {
        min2phase::init();
        solverListo = true;
    }).detach();
    const double solverStartTime = glfwGetTime();

    // Estado del solve async: solve() tambien corre en thread para no bloquear el render
    atomic<bool> resolviendoEnCurso(false);
    atomic<bool> solucionLista(false);
    string       solucionBuffer;
    mutex        solucionMutex;

    srand(static_cast<unsigned>(time(nullptr)));

    // Camara orbital + controles de capas (rubikControls lo maneja todo)
    Camera camera({0.0f, 0.0f, 4.0f});
    camera.moveSpeed = 0.6f;
    camera.setDirectView(
        {0.0f, 0.0f, 4.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}
    );

    RubikControls controls;

    double prevNow = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        float dt = static_cast<float>(now - prevNow);
        if (dt > 0.1f) dt = 0.1f;
        prevNow = now;

        // Camara + rotacion de capas (1-9 + flechas) + vistas (F/B/L/R/U/T/O)
        controls.update(window, camera, rubickCube, dt, now);

        // Recoger solucion cuando el thread la termina
        if (solucionLista.exchange(false)) {
            string sol;
            { lock_guard<mutex> lk(solucionMutex); sol = solucionBuffer; }
            queue<pair<Camada,int>> movs;
            parsearSolucion(sol, movs);
            if (!movs.empty())
                controls.iniciarMovimientos(move(movs), now);
        }

        // Enter: lanza el solve en un thread (no bloquea el render)
        if (keyPressed(window, GLFW_KEY_ENTER) && solverListo
                && !controls.isRotating() && !resolviendoEnCurso) {
            string estado = rubickCube.getEstado();
            bool valido = (estado.size() == 54);
            for (char c : estado)
                if (c!='U'&&c!='R'&&c!='F'&&c!='D'&&c!='L'&&c!='B') { valido = false; break; }
            if (valido) {
                resolviendoEnCurso = true;
                solucionLista     = false;
                thread([estado, &solucionBuffer, &solucionMutex,
                             &solucionLista, &resolviendoEnCurso]() {
                    string sol = min2phase::solve(estado, 20, 1000000, 0, 0);
                    { lock_guard<mutex> lk(solucionMutex); solucionBuffer = sol; }
                    solucionLista     = true;
                    resolviendoEnCurso = false;
                }).detach();
            }
        }

        // Space: mezclar aleatoriamente con 20 movimientos
        if (keyPressed(window, GLFW_KEY_SPACE) && !controls.isRotating()) {
            static const Camada allCams[] = {Front, Back, Up, Down, Right, Left};
            queue<pair<Camada,int>> movs;
            for (int i = 0; i < 20; ++i) {
                Camada c = allCams[rand() % 6];
                int    d = (rand() % 2) * 2 - 1;
                movs.push({c, d});
            }
            controls.iniciarMovimientos(move(movs), now);
        }

        const Mat4 view = camera.viewMatrix();
        const Mat4 projection = perspective(
            camera.zoomFovDeg,
            static_cast<float>(SCR_W) / static_cast<float>(SCR_H),
            0.1f,
            100.0f
        );

        setViewMatrix(view);
        setProjectionMatrix(projection);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rubickCube.draw();

        // Titulo: progreso del solver o instrucciones
        {
            static bool tituloFinalPuesto = false;
            static int  ultimoSegMostrado = -1;
            if (!solverListo) {
                int seg = (int)(glfwGetTime() - solverStartTime);
                if (seg != ultimoSegMostrado) {
                    ultimoSegMostrado = seg;
                    string t = "Cargando solver... " + to_string(seg) + "s";
                    glfwSetWindowTitle(window, t.c_str());
                }
            } else if (!tituloFinalPuesto) {
                tituloFinalPuesto = true;
                glfwSetWindowTitle(window, "Cubo Rubik  |  Enter=Resolver  |  Space=Mezclar  |  1-9+Flechas=Capas  |  F/B/L/R/U/T=Vista");
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}
