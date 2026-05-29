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

using namespace std;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

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

        if (dt > 0.1f) {
            dt = 0.1f;
        }

        prevNow = now;

        controls.update(window, camera, rubickCube, dt, now);

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