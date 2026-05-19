// HECHO POR IVAN SARDON MEDINA
#ifndef CUBO_RUBICK_H_INCLUDED
#define CUBO_RUBICK_H_INCLUDED

#include "draw3D.h"
#include <iostream>
#include <vector>

using namespace std;

class RubickCube {
public:
    RubickCube(float espacio = 0.56f, float tamCubito = 0.5f, float tamSticker = 0.45f) {
        espaciado = espacio;
        tamanhoCubito = tamCubito;
        tamanhoSticker = tamSticker;
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                for (int z = -1; z <= 1; z++) {
                    ShapeGroup* cubito = crearCubito(x, y, z);
                    cubito->transform.matrix = Mat4::translate3D(x * espacio, y * espacio, z * espacio);
                    grupoGeneral.addGroup(cubito);
                    cubitos.push_back(cubito);
                }
            }
        }
    }

    ~RubickCube() {
        for (ShapeGroup* cubito : cubitos)  delete cubito;
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

private:
    float espaciado, tamanhoCubito, tamanhoSticker;
    ShapeGroup grupoGeneral;
    vector<ShapeGroup*> cubitos;
    vector<Shape*> figuras;
    ShapeGroup* crearCubito(int x, int y, int z);
    void agregarSticker(ShapeGroup& grupoCubo, const Color& color, float tx, float ty, float tz,
                        float sx, float sy, float sz);
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

#endif // CUBO_RUBICK_H_INCLUDED
