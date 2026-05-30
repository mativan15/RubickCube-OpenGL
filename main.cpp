#define GLAD_GL_IMPLEMENTATION
#include "animations.h"
#include "camera.h"
#include "draw2D.h"
#include "draw3D.h"
#include "cuboRubick.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <min2phase/min2phase.h>


using namespace std;
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

static bool keyPressed(GLFWwindow* window, int key) {
    static bool prev[GLFW_KEY_LAST + 1] = {};
    const bool down = glfwGetKey(window, key) == GLFW_PRESS;
    const bool pressed = down && !prev[key];
    prev[key] = down;
    return pressed;
}

static void processInput( GLFWwindow* window, Anim& animRotateCuboX, Anim& animRotateCuboY, Anim& animRotateCuboZ,
                          int& dirRotateCuboX, int& dirRotateCuboY, int& dirRotateCuboZ) {
    static bool prevLeft = false;
    static bool prevRight = false;
    static bool prevUp = false;
    static bool prevDown = false;
    static bool prevK = false;
    static bool prevL = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const bool left = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    const bool right = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
    const bool up = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    const bool down = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
    const bool k = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;
    const bool l = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;

    if (left && !prevLeft && !animRotateCuboY.isPlaying()) {
        dirRotateCuboY = 1;
        animRotateCuboY.ignite( glfwGetTime());
    }
    if (right && !prevRight && !animRotateCuboY.isPlaying()) {
        dirRotateCuboY = -1;
        animRotateCuboY.ignite( glfwGetTime());
    }
    if (up && !prevUp && !animRotateCuboX.isPlaying()) {
        dirRotateCuboX = 1;
        animRotateCuboX.ignite( glfwGetTime());
    }
    if (down && !prevDown && !animRotateCuboX.isPlaying()) {
        dirRotateCuboX = -1;
        animRotateCuboX.ignite( glfwGetTime());
    }
    if (k && !prevK && !animRotateCuboZ.isPlaying()) {
        dirRotateCuboZ = 1;
        animRotateCuboZ.ignite( glfwGetTime());
    }
    if (l && !prevL && !animRotateCuboZ.isPlaying()) {
        dirRotateCuboZ = -1;
        animRotateCuboZ.ignite( glfwGetTime());
    }

    prevLeft = left;
    prevRight = right;
    prevUp = up;
    prevDown = down;
    prevK = k;
    prevL = l;
    
}

// =====================================================================
// FUNCIONES PARA MIN2PHASE (KOCIEMBA)
// =====================================================================
// Convierte el string de solucion de Kociemba a movimientos del cubo.
static void parsearSolucion(const string& solucion, queue<pair<Camada,int>>& colaMoves) {
    while(!colaMoves.empty()) colaMoves.pop();
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
        int dir = baseDir;
        int reps = 1;
        if (mov.size() > 1) {
            if (mov[1] == '\'') dir = -baseDir; // Invierte si es un movimiento prima (')
            else if (mov[1] == '2') reps = 2;   // Repite si es un movimiento doble (2)
        }
        for (int i = 0; i < reps; ++i)
            colaMoves.push({cam, dir});
    }
}
// =====================================================================

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

    RubickCube rubickCube(0.55f, 0.53f, 0.45f); //creacion cubo

    std::atomic<bool> solverListo(false);

    std::thread([&solverListo]() {
        min2phase::init();
        solverListo = true;
    }).detach();

    const double solverStartTime = glfwGetTime();

    AnimSpec RotateCubo{};
    RotateCubo.durationSec = 1.0f;
    RotateCubo.speed = 0.8f;
    RotateCubo.amplitude = 2*PI/6;
    RotateCubo.bounce = false;
    RotateCubo.infinite = false;

    Anim animRotateCuboX(RotateCubo);
    Anim animRotateCuboY(RotateCubo);
    Anim animRotateCuboZ(RotateCubo);

    AnimSpec RotateCamada{};
    RotateCamada.durationSec = 0.35f;
    RotateCamada.speed = 1.0f;
    RotateCamada.amplitude = PI/2;
    RotateCamada.bounce = false;
    RotateCamada.infinite = false;
    Anim animRotateCamada(RotateCamada);
    bool girandoCamada = false;
    Camada camadaActual = Front;
    int dirCamada = 1;

    vector<pair<Camada,int>> historial;  // movimientos manuales registrados
    queue<pair<Camada,int>>  colaMoves;  // movimientos pendientes de ejecutar
    bool autoModo = false;               // true mientras se ejecuta cola automatica
    srand(static_cast<unsigned>(time(nullptr)));

    Camera camera({0.0f, 0.0f, 4.0f});
    camera.moveSpeed = 0.6f;

    
    double prevNow = glfwGetTime();
    float baseRotateCuboX = 0.0f;
    float baseRotateCuboY = 0.0f;
    float baseRotateCuboZ = 0.0f;
    int dirRotateCuboX = 1;
    int dirRotateCuboY = 1;
    int dirRotateCuboZ = 1;
    
    while (!glfwWindowShouldClose(window)) {
        processInput(window, animRotateCuboX, animRotateCuboY, animRotateCuboZ,
                     dirRotateCuboX, dirRotateCuboY, dirRotateCuboZ);

        const double now = glfwGetTime();

        const bool inverso = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        if (!girandoCamada) {
            if (keyPressed(window, GLFW_KEY_Q)) { camadaActual = Up; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_W)) { camadaActual = Y; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_E)) { camadaActual = Down; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_A)) { camadaActual = Left; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_S)) { camadaActual = X; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_D)) { camadaActual = Right; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_Z)) { camadaActual = Front; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_X)) { camadaActual = Z; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_C)) { camadaActual = Back; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }

            // P: resuelve con Kociemba (solo si el solver cargo y el estado es valido)
            if (keyPressed(window, GLFW_KEY_P) && solverListo) {
                string estado = rubickCube.getEstado();
                bool valido = (estado.size() == 54);
                for (char c : estado)
                    if (c!='U'&&c!='R'&&c!='F'&&c!='D'&&c!='L'&&c!='B') { valido=false; break; }
                if (valido) {
                    string sol = min2phase::solve(estado, 20, 1000000, 0, 0);
                    parsearSolucion(sol, colaMoves);
                    if (!colaMoves.empty()) {
                        autoModo = true;
                        auto [cam, dir] = colaMoves.front(); colaMoves.pop();
                        camadaActual = cam; dirCamada = dir;
                        girandoCamada = true;
                        animRotateCamada.ignite(now);
                    }
                }
            }

            // Space: resuelve invirtiendo el historial de movimientos manuales
            if (keyPressed(window, GLFW_KEY_SPACE) && !historial.empty()) {
                for (int i = (int)historial.size() - 1; i >= 0; --i)
                    colaMoves.push({historial[i].first, -historial[i].second});
                historial.clear();
                auto [cam, dir] = colaMoves.front(); colaMoves.pop();
                camadaActual = cam; dirCamada = dir;
                girandoCamada = true; autoModo = true;
                animRotateCamada.ignite(now);
            }

            // R: mezcla aleatoriamente 20 movimientos
            if (keyPressed(window, GLFW_KEY_R)) {
                historial.clear();
                static const Camada allCams[] = {Front, Back, Up, Down, Right, Left};
                for (int i = 0; i < 20; ++i) {
                    Camada c = allCams[rand() % 6];
                    int    d = (rand() % 2) * 2 - 1;
                    historial.push_back({c, d});
                    colaMoves.push({c, d});
                }
                auto [cam, dir] = colaMoves.front(); colaMoves.pop();
                camadaActual = cam; dirCamada = dir;
                girandoCamada = true; autoModo = true;
                animRotateCamada.ignite(now);
            }
        }

        const bool wasRotX = animRotateCuboX.isPlaying();
        const bool wasRotY = animRotateCuboY.isPlaying();
        const bool wasRotZ = animRotateCuboZ.isPlaying();
        const bool wasRotCamada = animRotateCamada.isPlaying();
        float sRotateCuboX = animRotateCuboX.scalar(now);
        float sRotateCuboY = animRotateCuboY.scalar(now);
        float sRotateCuboZ = animRotateCuboZ.scalar(now);
        float sRotateCamada = animRotateCamada.scalar(now);

        if (wasRotX && !animRotateCuboX.isPlaying()) {
            baseRotateCuboX += RotateCubo.amplitude * float(dirRotateCuboX);
            sRotateCuboX = 0.0f;
        }
        if (wasRotY && !animRotateCuboY.isPlaying()) {
            baseRotateCuboY += RotateCubo.amplitude * float(dirRotateCuboY);
            sRotateCuboY = 0.0f;        }
        if (wasRotZ && !animRotateCuboZ.isPlaying()) {
            baseRotateCuboZ += RotateCubo.amplitude * float(dirRotateCuboZ);
            sRotateCuboZ = 0.0f; }
        
        if (girandoCamada) {
            if (wasRotCamada && !animRotateCamada.isPlaying()) {
                rubickCube.confirmarGiro(camadaActual, dirCamada);
                if (!autoModo)
                    historial.push_back({camadaActual, dirCamada});
                girandoCamada = false;
                sRotateCamada = 0.0f;
                if (!colaMoves.empty()) {
                    auto [cam, dir] = colaMoves.front(); colaMoves.pop();
                    camadaActual = cam; dirCamada = dir;
                    girandoCamada = true;
                    animRotateCamada.ignite(now);
                } else {
                    autoModo = false;
                }
            } else {
                rubickCube.moverCamada(camadaActual, sRotateCamada * float(dirCamada));
            }
        }

        float dt = static_cast<float>(now - prevNow);
        if (dt > 0.1f) dt = 0.1f;
        prevNow = now;

        const Mat4 view = camera.viewMatrix();
        const Mat4 projection = perspective(camera.zoomFovDeg,static_cast<float>(SCR_W) / static_cast<float>(SCR_H), 0.1f, 100.0f);
        setViewMatrix(view);
        setProjectionMatrix(projection);
        
        Mat4 rotX = Mat4::rotateX(baseRotateCuboX + sRotateCuboX * float(dirRotateCuboX));
        Mat4 rotY = Mat4::rotateY(baseRotateCuboY + sRotateCuboY * float(dirRotateCuboY));
        Mat4 rotZ = Mat4::rotateZ(baseRotateCuboZ + sRotateCuboZ * float(dirRotateCuboZ));
        rubickCube.transformMatGlobal(rotY * rotX * rotZ);
        
        glClearColor(0.3f, 0.3,0.3, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rubickCube.draw();

        // Actualiza el titulo con progreso del solver (calculado en render loop, sin atomics)
        {
            static bool tituloFinalPuesto = false;
            static int  ultimoSegMostrado = -1;
            if (!solverListo) {
                int seg = (int)(glfwGetTime() - solverStartTime);
                if (seg != ultimoSegMostrado) {
                    ultimoSegMostrado = seg;
                    std::string t = "Cargando solver... " + std::to_string(seg) + "s";
                    glfwSetWindowTitle(window, t.c_str());
                }
            } else if (!tituloFinalPuesto) {
                tituloFinalPuesto = true;
                glfwSetWindowTitle(window, "Cubo Rubik  |  P = Resolver  |  R = Mezclar");
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