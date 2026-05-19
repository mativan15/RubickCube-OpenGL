// HECHO POR IVAN SARDON MEDINA
#define GLAD_GL_IMPLEMENTATION
#include "animations.h"
#include "camera.h"
#include "draw2D.h"
#include "draw3D.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

const Color PlasticBlack{0.015f, 0.014f, 0.012f, 1.0f};
const Color Black{0.06f, 0.055f, 0.048f, 1.0f};
const Color White{0.96f, 0.96f, 0.90f, 1.0f};
const Color Yellow{1.00f, 0.84f, 0.05f, 1.0f};
const Color Red{0.78f, 0.02f, 0.03f, 1.0f};
const Color Orange{1.00f, 0.34f, 0.02f, 1.0f};
const Color Blue{0.02f, 0.16f, 0.86f, 1.0f};
const Color Green{0.00f, 0.58f, 0.18f, 1.0f};

ShapeGroup* crearCubito(){
    Cube* cubo1B = new Cube(0.5f, 0.5f, 0.5f, DrawPrim::Filled);
    cubo1B->color = Black;
    Cube* cubo2B = new Cube(0.45f, 0.45f, 0.02f, DrawPrim::Filled); // frente
    cubo2B->color = White;
    Cube* cubo3B = new Cube(0.45f, 0.02f, 0.45f, DrawPrim::Filled); // arriba
    cubo3B->color = Red;
    Cube* cubo4B = new Cube(0.45f, 0.45f, 0.02f, DrawPrim::Filled); // atras
    cubo4B->color = Yellow;
    Cube* cubo5B = new Cube(0.45f, 0.02f, 0.45f, DrawPrim::Filled); // abajo
    cubo5B->color = Orange;
    Cube* cubo6B = new Cube(0.02f, 0.45f, 0.45f, DrawPrim::Filled); // izq
    cubo6B->color = Blue;
    Cube* cubo7B = new Cube(0.02f, 0.45f, 0.45f, DrawPrim::Filled); // der
    cubo7B->color = Green;
    Mat4 initPos2B = Mat4::translate3D(0,0,0.26);
	Mat4 initPos3B = Mat4::translate3D(0,0.26,0);
    Mat4 initPos4B = Mat4::translate3D(0,0,-0.26);
	Mat4 initPos5B = Mat4::translate3D(0,-0.26,0);
    Mat4 initPos6B = Mat4::translate3D(-0.26,0,0);
	Mat4 initPos7B = Mat4::translate3D(0.26,0,0);
    cubo2B->transform.matrix = initPos2B;
    cubo3B->transform.matrix = initPos3B;
    cubo4B->transform.matrix = initPos4B;
    cubo5B->transform.matrix = initPos5B;
    cubo6B->transform.matrix = initPos6B;
    cubo7B->transform.matrix = initPos7B;
    //cubo2B->lineWidth = 2.0f;
    
    ShapeGroup* grupoCuboB = new ShapeGroup();
    grupoCuboB->add(cubo1B);
    grupoCuboB->add(cubo2B);
    grupoCuboB->add(cubo3B);
    grupoCuboB->add(cubo4B);
    grupoCuboB->add(cubo5B);
    grupoCuboB->add(cubo6B);
    grupoCuboB->add(cubo7B);
    return grupoCuboB;
};

using namespace std;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);


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

    
    
   
	
	//Pyramid(float baseWidth, float baseDepth, float height, const vector<Color>& faceColors,
           // DrawPrim prim = DrawPrim::Filled)
	
    ShapeGroup grupoGeneral;
    //ShapeGroup* grupoCuboA = crearCubito() ;
    //ShapeGroup* grupoCuboB = crearCubito();
    for(int i=0; i<9;i++){
        ShapeGroup* grupoCuboA = crearCubito();
        grupoGeneral.addGroup(grupoCuboA);
    }
    grupoGeneral.arrangeGrid(3, 0.5,0.5);
    //grupoGeneral.addGroup(grupoCuboA);
    //grupoGeneral.addGroup(grupoCuboB);

    AnimSpec RotateCubo{};
    RotateCubo.durationSec = 1.0f;
    RotateCubo.speed = 0.8f;
    RotateCubo.amplitude = PI/2;
    RotateCubo.bounce = false;
    RotateCubo.infinite = false;

    Anim animRotateCuboX(RotateCubo);
    Anim animRotateCuboY(RotateCubo);
    Anim animRotateCuboZ(RotateCubo);

    Camera camera({0.0f, 0.0f, 2.0f});
    camera.moveSpeed = 0.6f;

    
    double prevNow = glfwGetTime();
    float baseRotateCuboX = 0.0f;
    float baseRotateCuboY = 0.0f;
    float baseRotateCuboZ = 0.0f;
    int dirRotateCuboX = 1;
    int dirRotateCuboY = 1;
    int dirRotateCuboZ = 1;
	
	Mat4 initPosA = Mat4::translate3D(0.3,0,0);
	Mat4 initPosB = Mat4::translate3D(-0.3,0,0);
	

	//grupoCuboA->transform.matrix = initPosA;
    //grupoCuboB->transform.matrix = initPosB;

	
	
	
    while (!glfwWindowShouldClose(window)) {
        processInput(window, animRotateCuboX, animRotateCuboY, animRotateCuboZ,
                     dirRotateCuboX, dirRotateCuboY, dirRotateCuboZ);

        const double now = glfwGetTime();

        const bool wasRotX = animRotateCuboX.isPlaying();
        const bool wasRotY = animRotateCuboY.isPlaying();
        const bool wasRotZ = animRotateCuboZ.isPlaying();
        float sRotateCuboX = animRotateCuboX.scalar(now);
        float sRotateCuboY = animRotateCuboY.scalar(now);
        float sRotateCuboZ = animRotateCuboZ.scalar(now);

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
		grupoGeneral.transform.matrix = rotY * rotX * rotZ;
		
        glClearColor(0.05f, 0.1f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        grupoGeneral.draw();


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
