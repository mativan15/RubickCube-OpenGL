// HECHO POR IVAN SARDON MEDINA
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
    static bool prevQ = false;
    static bool prevE = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const bool left = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    const bool right = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
    const bool up = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    const bool down = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
    const bool q = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
    const bool e = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;

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
    if (q && !prevQ && !animRotateCuboZ.isPlaying()) {
        dirRotateCuboZ = 1;
        animRotateCuboZ.ignite( glfwGetTime());
    }
    if (e && !prevE && !animRotateCuboZ.isPlaying()) {
        dirRotateCuboZ = -1;
        animRotateCuboZ.ignite( glfwGetTime());
    }

    prevLeft = left;
    prevRight = right;
    prevUp = up;
    prevDown = down;
    prevQ = q;
    prevE = e;
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

    RubickCube rubickCube(0.55f, 0.53f, 0.45f); //creacion cubo

    AnimSpec RotateCubo{};
    RotateCubo.durationSec = 1.0f;
    RotateCubo.speed = 0.8f;
    RotateCubo.amplitude = PI/4;
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

        const bool inverso = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                             glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        if (!girandoCamada) {
            if (keyPressed(window, GLFW_KEY_F)) { camadaActual = Front; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_B)) { camadaActual = Back; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_U)) { camadaActual = Up; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_D)) { camadaActual = Down; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_R)) { camadaActual = Right; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
            else if (keyPressed(window, GLFW_KEY_L)) { camadaActual = Left; dirCamada = inverso ? -1 : 1; girandoCamada = true; animRotateCamada.ignite(now); }
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
                girandoCamada = false;
                sRotateCamada = 0.0f;
            } else {
                rubickCube.moverCamada(camadaActual, sRotateCamada * float(dirCamada));
            }
        }

        float dt = static_cast<float>(now - prevNow);
        if (dt > 0.1f) dt = 0.1f;
        prevNow = now;

        const Mat4 view = camera.viewMatrix();
        const Mat4 projection = perspective(camera.zoomFovDeg,static_cast<float>(SCR_W) / static_cast<float>(SCR_H),
                                            0.1f, 100.0f);
        setViewMatrix(view);
        setProjectionMatrix(projection);
        
        Mat4 rotX = Mat4::rotateX(baseRotateCuboX + sRotateCuboX * float(dirRotateCuboX));
        Mat4 rotY = Mat4::rotateY(baseRotateCuboY + sRotateCuboY * float(dirRotateCuboY));
        Mat4 rotZ = Mat4::rotateZ(baseRotateCuboZ + sRotateCuboZ * float(dirRotateCuboZ));
		rubickCube.transformMatGlobal(rotY * rotX * rotZ);
		
        glClearColor(0.4f, 0.4,0.4, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rubickCube.draw();


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
