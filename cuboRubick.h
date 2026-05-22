// HECHO POR IVAN SARDON MEDINA
#ifndef CUBO_RUBICK_H_INCLUDED
#define CUBO_RUBICK_H_INCLUDED

#include "draw3D.h"
#include <iostream>
#include <vector>
#include <array>

using namespace std;

enum Camada {Front, Back, Up, Down, Right, Left, X, Y, Z};

struct CubitoRubick {
    ShapeGroup* grupo = nullptr;
    int x = 0, y = 0, z = 0;
    Mat4 orientacion = Mat4::identity();
};

class RubickCube {
public:
    RubickCube(float espacio = 0.56f, float tamCubito = 0.5f, float tamSticker = 0.45f) {
        espaciado = espacio;
        tamanhoCubito = tamCubito;
        tamanhoSticker = tamSticker;
        cubitos.reserve(27);
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                for (int z = -1; z <= 1; z++) {
                    ShapeGroup* cubito = crearCubito(x, y, z);
                    grupoGeneral.addGroup(cubito);
                    cubitos.push_back({cubito, x, y, z, Mat4::identity()});
                }
            }
        }
        actualizarCamadas();
        aplicarTransformaciones();
    }

    ~RubickCube() {
        for (CubitoRubick& cubito : cubitos) delete cubito.grupo;
        for (Shape* figura : figuras) delete figura;
    }

    RubickCube(const RubickCube&) = delete;
    RubickCube& operator=(const RubickCube&) = delete;

    void transformMatGlobal(const Mat4& matrix) {
        grupoGeneral.transform.matrix = matrix;
    }

    void draw() const {
        grupoGeneral.draw();
    }

    void moverCamada(Camada camada, float angulo) {
        aplicarTransformaciones(camada, angulo);
    }

    void confirmarGiro(Camada camada, int dir) {
        const Mat4 rot = matrizRotacion(camada, dir * PI / 2.0f);
        vector<CubitoRubick*> cubitosGirados = camadas[camada];
        for (CubitoRubick* cubito : cubitosGirados) {
            rotarCoordenadas(*cubito, camada, dir);
            cubito->orientacion = rot * cubito->orientacion;
        }
        actualizarCamadas();
        aplicarTransformaciones();
    }
    

private:
    float espaciado, tamanhoCubito, tamanhoSticker;
    ShapeGroup grupoGeneral; //global
    array<vector<CubitoRubick*>, 9> camadas;
    vector<CubitoRubick> cubitos;
    vector<Shape*> figuras;
    ShapeGroup* crearCubito(int x, int y, int z);
    void agregarSticker(ShapeGroup& grupoCubo, const Color& color, float tx, float ty, float tz,
                        float sx, float sy, float sz);
    void actualizarCamadas();
    void aplicarTransformaciones();
    void aplicarTransformaciones(Camada camada, float angulo);
    Mat4 matrizBase(const CubitoRubick& cubito) const;
    Mat4 matrizRotacion(Camada camada, float angulo) const;
    void rotarCoordenadas(CubitoRubick& cubito, Camada camada, int dir);
    const Color Black{0.015f, 0.014f, 0.012f, 1.0f};
    const Color White{0.96f, 0.96f, 0.90f, 1.0f};
    const Color Yellow{1.00f, 0.84f, 0.05f, 1.0f};
    const Color Red{0.78f, 0.02f, 0.03f, 1.0f};
    const Color Orange{1.00f, 0.34f, 0.02f, 1.0f};
    const Color Blue{0.02f, 0.16f, 0.86f, 1.0f};
    const Color Green{0.00f, 0.58f, 0.18f, 1.0f};
};

inline void RubickCube::agregarSticker(ShapeGroup& grupoCubo, const Color& color,float tx, float ty, float tz,
                           float sx, float sy, float sz) {
    Cube* sticker = new Cube(sx, sy, sz, DrawPrim::Filled);
    sticker->color = color;
    sticker->transform.matrix = Mat4::translate3D(tx, ty, tz);
    grupoCubo.add(sticker);
    figuras.push_back(sticker);
}

inline ShapeGroup* RubickCube::crearCubito(int x, int y, int z) {
    ShapeGroup* grupoCubo = new ShapeGroup();

    Cube* cuerpo = new Cube(tamanhoCubito, tamanhoCubito, tamanhoCubito, DrawPrim::Filled);
    cuerpo->color = Black;
    grupoCubo->add(cuerpo);
    figuras.push_back(cuerpo);
    float posSticker = tamanhoCubito/2.f;
    
    if (z ==  1) agregarSticker(*grupoCubo, White,  0, 0, posSticker, tamanhoSticker, tamanhoSticker, 0.02f); // frente
    if (y ==  1) agregarSticker(*grupoCubo, Red,   0,  posSticker, 0, tamanhoSticker, 0.02f, tamanhoSticker); // arriba
    if (x ==  1) agregarSticker(*grupoCubo, Green, posSticker, 0,  0, 0.02f, tamanhoSticker, tamanhoSticker); // der
    if (z == -1) agregarSticker(*grupoCubo, Yellow, 0,  0,-posSticker, tamanhoSticker, tamanhoSticker, 0.02f); // atras
    if (y == -1) agregarSticker(*grupoCubo, Orange, 0,-posSticker, 0, tamanhoSticker, 0.02f, tamanhoSticker); // abajo
    if (x == -1) agregarSticker(*grupoCubo, Blue, -posSticker, 0, 0,  0.02f, tamanhoSticker, tamanhoSticker); // izq

    return grupoCubo;
}

inline void RubickCube::actualizarCamadas() {
    for (vector<CubitoRubick*>& camada : camadas) {
        camada.clear();
    }

    for (CubitoRubick& cubito : cubitos) {
        if (cubito.z ==  1) camadas[Front].push_back(&cubito);
        if (cubito.z ==  0) camadas[Z].push_back(&cubito);
        if (cubito.z == -1) camadas[Back].push_back(&cubito);
        if (cubito.y ==  1) camadas[Up].push_back(&cubito);
        if (cubito.y ==  0) camadas[Y].push_back(&cubito);
        if (cubito.y == -1) camadas[Down].push_back(&cubito);
        if (cubito.x ==  1) camadas[Right].push_back(&cubito);
        if (cubito.x ==  0) camadas[X].push_back(&cubito);
        if (cubito.x == -1) camadas[Left].push_back(&cubito);
    }
}

inline Mat4 RubickCube::matrizBase(const CubitoRubick& cubito) const {
    return Mat4::translate3D(cubito.x * espaciado, cubito.y * espaciado, cubito.z * espaciado) *
           cubito.orientacion;
}

inline Mat4 RubickCube::matrizRotacion(Camada camada, float angulo) const {
    if (camada == Right || camada == Left || camada == X) return Mat4::rotateX(angulo);
    if (camada == Up || camada == Down || camada == Y) return Mat4::rotateY(angulo);
    return Mat4::rotateZ(angulo);
}

inline void RubickCube::aplicarTransformaciones() {
    for (CubitoRubick& cubito : cubitos) {
        cubito.grupo->transform.matrix = matrizBase(cubito);
    }
}

inline void RubickCube::aplicarTransformaciones(Camada camada, float angulo) {
    const Mat4 rot = matrizRotacion(camada, angulo);
    for (CubitoRubick& cubito : cubitos) {
        cubito.grupo->transform.matrix = matrizBase(cubito);
    }
    for (CubitoRubick* cubito : camadas[camada]) {
        cubito->grupo->transform.matrix = rot * matrizBase(*cubito);
    }
}

inline void RubickCube::rotarCoordenadas(CubitoRubick& cubito, Camada camada, int dir) {
    const int signo = dir >= 0 ? 1 : -1;
    const int oldX = cubito.x;
    const int oldY = cubito.y;
    const int oldZ = cubito.z;

    if (camada == Right || camada == Left || camada == X) {
        cubito.y = signo > 0 ? -oldZ : oldZ;
        cubito.z = signo > 0 ? oldY : -oldY;
    } else if (camada == Up || camada == Down || camada == Y) {
        cubito.x = signo > 0 ? oldZ : -oldZ;
        cubito.z = signo > 0 ? -oldX : oldX;
    } else {
        cubito.x = signo > 0 ? -oldY : oldY;
        cubito.y = signo > 0 ? oldX : -oldX;
    }
}

#endif //
