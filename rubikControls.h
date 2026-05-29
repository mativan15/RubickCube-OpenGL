#ifndef RUBIK_CONTROLS_H_INCLUDED
#define RUBIK_CONTROLS_H_INCLUDED

#include "animations.h"
#include "camera.h"
#include "cuboRubick.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

enum class ViewMode {
    Front,
    Back,
    Left,
    Right,
    Up,
    Down,
    OrbitalGlobal
};

enum class LayerMode {
    FaceView,
    Global9Layers
};

class RubikControls {
public:
    RubikControls()
        : layerRotationAnim(createLayerRotationAnim())
    {}

    void update(GLFWwindow* window, Camera& camera, RubickCube& cube, float dt, double now) {
        handleExit(window);

        handleViewKeys(window, camera);
        handleOrbitWASD(window, camera, dt);
        handleMouseOrbit(window, camera);

        handleLayerSelection(window);
        handleLayerRotationStart(window, now);
        updateLayerAnimation(cube, now);
    }

private:
    ViewMode currentView = ViewMode::Front;
    LayerMode layerMode = LayerMode::FaceView;

    int selectedLayer = 0;

    bool rotatingLayer = false;
    Camada currentLayer = Front;
    int layerDirection = 1;

    Anim layerRotationAnim;

    bool mouseInitialized = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

private:
    static AnimSpec createLayerRotationAnim() {
        AnimSpec spec{};
        spec.durationSec = 0.35f;
        spec.speed = 1.0f;
        spec.amplitude = PI / 2.0f;
        spec.bounce = false;
        spec.infinite = false;
        return spec;
    }

    bool keyPressedOnce(GLFWwindow* window, int key) {
        static bool previous[GLFW_KEY_LAST + 1] = {};

        const bool isDown = glfwGetKey(window, key) == GLFW_PRESS;
        const bool pressedOnce = isDown && !previous[key];

        previous[key] = isDown;

        return pressedOnce;
    }

    void handleExit(GLFWwindow* window) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
    }

    void handleViewKeys(GLFWwindow* window, Camera& camera) {
        if (keyPressedOnce(window, GLFW_KEY_F)) {
            currentView = ViewMode::Front;
            layerMode = LayerMode::FaceView;
            selectedLayer = 0;

            camera.setDirectView(
                {0.0f, 0.0f, 4.0f},
                {0.0f, 0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f}
            );
        }

        if (keyPressedOnce(window, GLFW_KEY_B)) {
            currentView = ViewMode::Back;
            layerMode = LayerMode::FaceView;
            selectedLayer = 0;

            camera.setDirectView(
                {0.0f, 0.0f, -4.0f},
                {0.0f, 0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f}
            );
        }

        if (keyPressedOnce(window, GLFW_KEY_L)) {
            currentView = ViewMode::Left;
            layerMode = LayerMode::FaceView;
            selectedLayer = 0;

            camera.setDirectView(
                {-4.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f}
            );
        }

        if (keyPressedOnce(window, GLFW_KEY_R)) {
            currentView = ViewMode::Right;
            layerMode = LayerMode::FaceView;
            selectedLayer = 0;

            camera.setDirectView(
                {4.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f}
            );
        }

        if (keyPressedOnce(window, GLFW_KEY_U)) {
            currentView = ViewMode::Up;
            layerMode = LayerMode::FaceView;
            selectedLayer = 0;

            camera.setDirectView(
                {0.0f, 4.0f, 0.0f},
                {0.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, -1.0f}
            );
        }

        if (keyPressedOnce(window, GLFW_KEY_Q)) {
            currentView = ViewMode::Down;
            layerMode = LayerMode::FaceView;
            selectedLayer = 0;

            camera.setDirectView(
                {0.0f, -4.0f, 0.0f},
                {0.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 1.0f}
            );
        }

        if (keyPressedOnce(window, GLFW_KEY_O)) {
            currentView = ViewMode::OrbitalGlobal;
            layerMode = LayerMode::FaceView;
            selectedLayer = 0;

            camera.useDirectView = false;
        }

        if (keyPressedOnce(window, GLFW_KEY_P)) {
            layerMode = LayerMode::Global9Layers;
            selectedLayer = 0;
        }
    }

    void handleOrbitWASD(GLFWwindow* window, Camera& camera, float dt) {
        const float speed = camera.orbitSpeedDeg * dt;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera.orbit(-speed, 0.0f);
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera.orbit(speed, 0.0f);
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera.orbit(0.0f, speed);
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera.orbit(0.0f, -speed);
        }
    }

    void handleMouseOrbit(GLFWwindow* window, Camera& camera) {
        const bool rightClick =
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        double x, y;
        glfwGetCursorPos(window, &x, &y);

        if (!rightClick) {
            mouseInitialized = false;
            return;
        }

        if (!mouseInitialized) {
            lastMouseX = x;
            lastMouseY = y;
            mouseInitialized = true;
            return;
        }

        const float dx = static_cast<float>(x - lastMouseX);
        const float dy = static_cast<float>(lastMouseY - y);

        lastMouseX = x;
        lastMouseY = y;

        const float sensitivity = camera.mouseSensitivity * 2.0f;

        camera.orbit(dx * sensitivity, dy * sensitivity);
    }

    void handleLayerSelection(GLFWwindow* window) {
        if (layerMode == LayerMode::FaceView) {
            if (currentView == ViewMode::OrbitalGlobal) {
                return;
            }

            if (keyPressedOnce(window, GLFW_KEY_1)) {
                selectedLayer = 1;
            }

            if (keyPressedOnce(window, GLFW_KEY_2)) {
                selectedLayer = 2;
            }

            if (keyPressedOnce(window, GLFW_KEY_3)) {
                selectedLayer = 3;
            }

            if (keyPressedOnce(window, GLFW_KEY_4)) {
                selectedLayer = 4;
            }

            if (keyPressedOnce(window, GLFW_KEY_5)) {
                selectedLayer = 5;
            }

            if (keyPressedOnce(window, GLFW_KEY_6)) {
                selectedLayer = 6;
            }
        }

        else if (layerMode == LayerMode::Global9Layers) {
            if (keyPressedOnce(window, GLFW_KEY_1)) {
                selectedLayer = 1;
            }

            if (keyPressedOnce(window, GLFW_KEY_2)) {
                selectedLayer = 2;
            }

            if (keyPressedOnce(window, GLFW_KEY_3)) {
                selectedLayer = 3;
            }

            if (keyPressedOnce(window, GLFW_KEY_4)) {
                selectedLayer = 4;
            }

            if (keyPressedOnce(window, GLFW_KEY_5)) {
                selectedLayer = 5;
            }

            if (keyPressedOnce(window, GLFW_KEY_6)) {
                selectedLayer = 6;
            }

            if (keyPressedOnce(window, GLFW_KEY_7)) {
                selectedLayer = 7;
            }

            if (keyPressedOnce(window, GLFW_KEY_8)) {
                selectedLayer = 8;
            }

            if (keyPressedOnce(window, GLFW_KEY_9)) {
                selectedLayer = 9;
            }
        }
    }

    void handleLayerRotationStart(GLFWwindow* window, double now) {
        if (rotatingLayer) {
            return;
        }

        if (selectedLayer == 0) {
            return;
        }

        int visualDirection = 0;

        if (layerMode == LayerMode::FaceView) {
            if (currentView == ViewMode::OrbitalGlobal) {
                return;
            }

            if (selectedLayer >= 1 && selectedLayer <= 3) {
                if (keyPressedOnce(window, GLFW_KEY_UP)) {
                    visualDirection = 1;
                }

                if (keyPressedOnce(window, GLFW_KEY_DOWN)) {
                    visualDirection = -1;
                }
            }

            if (selectedLayer >= 4 && selectedLayer <= 6) {
                if (keyPressedOnce(window, GLFW_KEY_RIGHT)) {
                    visualDirection = 1;
                }

                if (keyPressedOnce(window, GLFW_KEY_LEFT)) {
                    visualDirection = -1;
                }
            }

            if (visualDirection == 0) {
                return;
            }

            currentLayer = getLayerFromView(currentView, selectedLayer);
            layerDirection = visualDirection * getViewDirectionSign(currentView, selectedLayer);
        }

        else if (layerMode == LayerMode::Global9Layers) {
            if (keyPressedOnce(window, GLFW_KEY_RIGHT) ||
                keyPressedOnce(window, GLFW_KEY_UP)) {
                visualDirection = 1;
            }

            if (keyPressedOnce(window, GLFW_KEY_LEFT) ||
                keyPressedOnce(window, GLFW_KEY_DOWN)) {
                visualDirection = -1;
            }

            if (visualDirection == 0) {
                return;
            }

            currentLayer = getGlobalLayer(selectedLayer);
            layerDirection = visualDirection;
        }

        rotatingLayer = true;
        layerRotationAnim.ignite(now);
    }

    void updateLayerAnimation(RubickCube& cube, double now) {
        if (!rotatingLayer) {
            return;
        }

        const bool wasPlaying = layerRotationAnim.isPlaying();
        float angle = layerRotationAnim.scalar(now);

        if (wasPlaying && !layerRotationAnim.isPlaying()) {
            cube.confirmarGiro(currentLayer, layerDirection);
            rotatingLayer = false;
            angle = 0.0f;
        }
        else {
            cube.moverCamada(
                currentLayer,
                angle * static_cast<float>(layerDirection)
            );
        }
    }

    Camada getLayerFromView(ViewMode view, int selection) {
        if (view == ViewMode::Front) {
            if (selection == 1) return Left;
            if (selection == 2) return X;
            if (selection == 3) return Right;
            if (selection == 4) return Up;
            if (selection == 5) return Y;
            if (selection == 6) return Down;
        }

        if (view == ViewMode::Back) {
            if (selection == 1) return Right;
            if (selection == 2) return X;
            if (selection == 3) return Left;
            if (selection == 4) return Up;
            if (selection == 5) return Y;
            if (selection == 6) return Down;
        }

        if (view == ViewMode::Right) {
            if (selection == 1) return Front;
            if (selection == 2) return Z;
            if (selection == 3) return Back;
            if (selection == 4) return Up;
            if (selection == 5) return Y;
            if (selection == 6) return Down;
        }

        if (view == ViewMode::Left) {
            if (selection == 1) return Back;
            if (selection == 2) return Z;
            if (selection == 3) return Front;
            if (selection == 4) return Up;
            if (selection == 5) return Y;
            if (selection == 6) return Down;
        }

        if (view == ViewMode::Up) {
            if (selection == 1) return Left;
            if (selection == 2) return X;
            if (selection == 3) return Right;
            if (selection == 4) return Back;
            if (selection == 5) return Z;
            if (selection == 6) return Front;
        }

        if (view == ViewMode::Down) {
            if (selection == 1) return Left;
            if (selection == 2) return X;
            if (selection == 3) return Right;
            if (selection == 4) return Front;
            if (selection == 5) return Z;
            if (selection == 6) return Back;
        }

        return Front;
    }

    Camada getGlobalLayer(int selection) {
        if (selection == 1) return Left;
        if (selection == 2) return X;
        if (selection == 3) return Right;

        if (selection == 4) return Up;
        if (selection == 5) return Y;
        if (selection == 6) return Down;

        if (selection == 7) return Front;
        if (selection == 8) return Z;
        if (selection == 9) return Back;

        return Front;
    }

    int getViewDirectionSign(ViewMode view, int selection) {
        (void)selection;

        if (view == ViewMode::Back) {
            return -1;
        }

        if (view == ViewMode::Left) {
            return -1;
        }

        if (view == ViewMode::Down) {
            return -1;
        }

        return 1;
    }
};

#endif