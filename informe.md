# Informe Técnico: Implementación de `getEstado()` y el puente hacia min2phase

---

## 1. Contexto: ¿Qué problema resuelve `getEstado()`?

El cubo de Rubik en este proyecto es una **aplicación gráfica**: su estado interno está
representado mediante coordenadas enteras y matrices de transformación OpenGL. Eso es perfecto
para dibujar el cubo en pantalla, pero es completamente ilegible para cualquier algoritmo
externo de solución.

El algoritmo **Kociemba de dos fases** (implementado en la librería `min2phase`) espera recibir
el estado del cubo como una cadena de 54 caracteres con el siguiente formato:

```
UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB
```

Cada letra representa el color de un sticker visto desde fuera de la cara correspondiente:
- `U` = Up (Arriba)
- `R` = Right (Derecha)
- `F` = Front (Frente)
- `D` = Down (Abajo)
- `L` = Left (Izquierda)
- `B` = Back (Atrás)

Son 6 caras × 9 stickers = **54 caracteres** en total.

**El problema central:** el proyecto no guarda colores como datos lógicos. Los colores
existen únicamente como objetos `Cube` de OpenGL dentro de cada `ShapeGroup`. No hay ningún
array `color[cara][fila][columna]`. Por eso necesitábamos implementar una función que
reconstruya el estado de colores leyendo la geometría matemática del cubo.

`getEstado()` es esa función. Es el puente entre la representación gráfica y el solver.

---

## 2. Cómo está estructurado el cubo internamente

### 2.1 Los 27 cubitos y sus coordenadas

El cubo se construye con tres bucles `for` anidados de `-1` a `1`:

```cpp
for (int x = -1; x <= 1; x++)
  for (int y = -1; y <= 1; y++)
    for (int z = -1; z <= 1; z++)
      cubitos.push_back({cubito, x, y, z, Mat4::identity()});
```

Esto genera los **27 cubitos** (piezas pequeñas) en una grilla tridimensional:

```
Eje X: -1 = izquierda,  0 = centro,  +1 = derecha
Eje Y: -1 = abajo,      0 = centro,  +1 = arriba
Eje Z: -1 = atrás,      0 = centro,  +1 = frente
```

Ejemplo de cubitos:
- `(1, 1, 1)`  → esquina derecha-arriba-frente (tiene 3 stickers: verde, rojo, blanco)
- `(0, 1, 0)`  → centro de la cara Up (solo 1 sticker: rojo)
- `(0, 0, 0)`  → cubito central (invisible, no tiene stickers)

### 2.2 La estructura `CubitoRubick`

```cpp
struct CubitoRubick {
    ShapeGroup* grupo  = nullptr;        // figura OpenGL a dibujar
    int x = 0, y = 0, z = 0;            // posición en la grilla
    Mat4 orientacion = Mat4::identity(); // acumula todas las rotaciones sufridas
};
```

El campo más importante para el solver es `orientacion`. Es una **matriz 4×4 de rotación**
que acumula todos los giros que ha sufrido ese cubito a lo largo del tiempo. Al inicio es la
identidad (sin rotación). Cada vez que se confirma un giro de capa, se actualiza así:

```cpp
cubito->orientacion = rot * cubito->orientacion;
```

Esta matriz es la "memoria" del cubito: sabe hacia dónde apuntan sus stickers ahora mismo,
sin importar cuántos giros haya sufrido.

### 2.3 Colores asignados en la construcción

En `crearCubito()`, los stickers se asignan según la posición **inicial** del cubito:

```cpp
if (z ==  1) sticker = Blanco   // cara Front
if (y ==  1) sticker = Rojo     // cara Up
if (x ==  1) sticker = Verde    // cara Right
if (z == -1) sticker = Amarillo // cara Back
if (y == -1) sticker = Naranja  // cara Down
if (x == -1) sticker = Azul     // cara Left
```

Un sticker blanco siempre nació mirando hacia `+Z`. Un sticker rojo siempre nació mirando
hacia `+Y`. Esta correspondencia entre color y **normal original** es la clave de todo.

| Color    | Normal original | Cara de origen | Letra Kociemba |
|----------|----------------|----------------|----------------|
| Blanco   | `(0, 0, +1)`   | Front          | `F`            |
| Rojo     | `(0, +1, 0)`   | Up             | `U`            |
| Verde    | `(+1, 0, 0)`   | Right          | `R`            |
| Amarillo | `(0, 0, -1)`   | Back           | `B`            |
| Naranja  | `(0, -1, 0)`   | Down           | `D`            |
| Azul     | `(-1, 0, 0)`   | Left           | `L`            |

---

## 3. Las camadas (capas del cubo)

### 3.1 El enum `Camada`

```cpp
enum Camada {Front, Back, Up, Down, Right, Left, X, Y, Z};
```

Son 9 capas en total: 6 caras visibles y 3 capas del medio (invisibles desde afuera).

Cada valor corresponde a un **plano de coordenadas**:

| Camada  | Coordenada fija | Valor |
|---------|----------------|-------|
| `Front` | z              | +1    |
| `Back`  | z              | -1    |
| `Up`    | y              | +1    |
| `Down`  | y              | -1    |
| `Right` | x              | +1    |
| `Left`  | x              | -1    |
| `Z`     | z              | 0     |
| `Y`     | y              | 0     |
| `X`     | x              | 0     |

### 3.2 Cómo se construyen las listas de cubitos por capa

La función `actualizarCamadas()` recorre los 27 cubitos y los clasifica por posición:

```cpp
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
```

Nota que un cubito **puede pertenecer a múltiples capas** simultáneamente. Por ejemplo, el
cubito en `(1, 1, 1)` pertenece a `Front`, `Up` y `Right` al mismo tiempo. Esto es correcto:
cuando se gira la cara Front, ese cubito se mueve junto con todos los demás de `z == 1`.

Esta función se llama **después de cada giro** (`confirmarGiro`) para reconstruir las listas
con las nuevas posiciones.

### 3.3 Cómo funciona un giro de capa

Un giro tiene dos fases:

**Fase 1 — Animación visual** (`moverCamada`):  
Mientras la animación avanza de `0` a `π/2` radianes, solo se mueve visualmente
la figura OpenGL. Las coordenadas `x, y, z` del cubito **no cambian todavía**.

**Fase 2 — Confirmación** (`confirmarGiro`):  
Cuando la animación termina, se actualizan las coordenadas reales con una permutación
cíclica de 90 grados y se acumula la rotación en `orientacion`:

```cpp
void confirmarGiro(Camada camada, int dir) {
    const Mat4 rot = matrizRotacion(camada, dir * PI / 2.0f);
    for (CubitoRubick* cubito : camadas[camada]) {
        rotarCoordenadas(*cubito, camada, dir);      // actualiza x, y, z
        cubito->orientacion = rot * cubito->orientacion; // acumula rotación
    }
    actualizarCamadas();      // reconstruye listas por capa
    aplicarTransformaciones(); // actualiza visuales
}
```

`rotarCoordenadas` aplica la permutación matemática exacta de 90°:

```cpp
// Eje X (Right, Left, capa-X):
y' = -signo * z_viejo
z' =  signo * y_viejo

// Eje Y (Up, Down, capa-Y):
x' =  signo * z_viejo
z' = -signo * x_viejo

// Eje Z (Front, Back, capa-Z):
x' = -signo * y_viejo
y' =  signo * x_viejo
```

Donde `signo = +1` para horario y `-1` para antihorario.

**Ejemplo concreto:** Si se gira `Up` con `dir=+1`, el cubito en `(1, 1, 0)` se mueve a:
- `x' = signo * z = 1 * 0 = 0`
- `z' = -signo * x = -1 * 1 = -1`
- Resultado: `(0, 1, -1)` ✓ (se movió hacia la cara Back, correcto para un giro horario)

---

## 4. La matriz `orientacion` y el problema de leer colores

### 4.1 Qué guarda `orientacion`

La matriz `orientacion` es una **matriz de rotación pura** 4×4 en formato column-major
(convención de OpenGL). Al inicio es la identidad. Después de cada giro de capa que afecta
al cubito, se pre-multiplica por la nueva rotación:

```
orientacion_nueva = rot_nueva × orientacion_anterior
```

Esto significa que si un sticker tenía la normal `n0` en su estado original, **ahora mismo**
esa normal apunta en la dirección `orientacion × n0`.

**Ejemplo:**  
Un sticker blanco tiene normal original `n0 = (0, 0, 1)` (apuntaba hacia `+Z`, cara Front).  
Si su cubito sufrió `rotateY(π/2)`:
```
orientacion × (0, 0, 1) = (1, 0, 0)
```
Ahora ese sticker blanco apunta hacia `+X` (cara Right). Si miramos la cara Right de ese
cubito, veremos color blanco.

### 4.2 El problema inverso: dado que estoy mirando una cara, ¿qué color veo?

Para construir `getEstado()` necesitamos el problema **inverso**:

> Dado un cubito en posición `(cx, cy, cz)` y yo mirando la cara cuya normal es `d = (dx, dy, dz)`,
> ¿qué sticker (color) está apuntando en esa dirección?

Buscamos el sticker cuya normal original `n0` satisface:
```
orientacion × n0 = d
```

Despejando `n0`:
```
n0 = orientacion⁻¹ × d
```

Y como `orientacion` es una **matriz de rotación pura**, su inversa es su transpuesta:
```
orientacion⁻¹ = orientacionᵀ
```

Por lo tanto:
```
n0 = orientaciónᵀ × d
```

Una vez obtenemos `n0` (redondeado al entero más cercano), lo comparamos con las normales
originales de la tabla de colores y obtenemos la letra Kociemba.

---

## 5. La función auxiliar `cubitoEn()`

```cpp
inline const CubitoRubick* RubickCube::cubitoEn(int x, int y, int z) const {
    for (const CubitoRubick& c : cubitos)
        if (c.x == x && c.y == y && c.z == z) return &c;
    return nullptr;
}
```

**Propósito:** dado un triplete de coordenadas enteras `(x, y, z)`, devuelve un puntero
al `CubitoRubick` que actualmente ocupa esa posición en la grilla.

**Por qué es necesaria:**  
`getEstado()` necesita recorrer las caras en un orden específico (por ejemplo, la cara Up
se lee de izquierda a derecha, de atrás hacia adelante). Eso significa que necesita acceder
a los cubitos por posición: "dame el cubito que está en `(-1, 1, -1)`".

El vector `cubitos` está en orden de creación (el orden de los tres bucles for del constructor),
no en orden de posición. Además, las posiciones cambian con cada giro. Por eso se necesita
esta búsqueda lineal.

**Línea a línea:**

```cpp
for (const CubitoRubick& c : cubitos)
```
Itera los 27 cubitos del vector.

```cpp
    if (c.x == x && c.y == y && c.z == z) return &c;
```
Si las coordenadas actuales del cubito coinciden con las buscadas, devuelve su dirección.

```cpp
return nullptr;
```
Si no se encontró (no debería ocurrir si el cubo está en estado válido).

---

## 6. La función auxiliar `colorEnDireccion()`

Esta es la función matemáticamente más importante de toda la implementación.

```cpp
inline char RubickCube::colorEnDireccion(const CubitoRubick& cubito,
                                          int dx, int dy, int dz) const {
    const float* m = cubito.orientacion.m;
    const int ix = static_cast<int>(std::round(m[0]*dx + m[1]*dy + m[2]*dz));
    const int iy = static_cast<int>(std::round(m[4]*dx + m[5]*dy + m[6]*dz));
    const int iz = static_cast<int>(std::round(m[8]*dx + m[9]*dy + m[10]*dz));
    if (iz ==  1) return 'F';
    if (iy ==  1) return 'U';
    if (ix ==  1) return 'R';
    if (iz == -1) return 'B';
    if (iy == -1) return 'D';
    if (ix == -1) return 'L';
    return '?';
}
```

**Propósito:** dado un cubito y una dirección de cara `(dx, dy, dz)`, devuelve la letra
Kociemba del color que está mirando hacia esa dirección.

### 6.1 La matemática de la transpuesta

La matriz `orientacion` está en formato **column-major** (convención OpenGL). Esto significa:

```
m[col * 4 + fila]
```

En memoria el arreglo `m[16]` se organiza así:

```
m[0]  m[4]  m[8]  m[12]      columna 0  columna 1  columna 2  columna 3
m[1]  m[5]  m[9]  m[13]
m[2]  m[6]  m[10] m[14]
m[3]  m[7]  m[11] m[15]
```

La multiplicación `M × v` en column-major es:
```
(M × v).x = m[0]*vx + m[4]*vy + m[8]*vz
(M × v).y = m[1]*vx + m[5]*vy + m[9]*vz
(M × v).z = m[2]*vx + m[6]*vy + m[10]*vz
```

La multiplicación `Mᵀ × v` (transpuesta) en column-major es:
```
(Mᵀ × v).x = m[0]*vx + m[1]*vy + m[2]*vz   ← usa la FILA 0 de M, no la columna 0
(Mᵀ × v).y = m[4]*vx + m[5]*vy + m[6]*vz
(Mᵀ × v).z = m[8]*vx + m[9]*vy + m[10]*vz
```

**Verificación con ejemplo:**  
Sea `M = rotateY(π/2)`:  
`m[0]=0, m[2]=-1, m[8]=1, m[10]=0` (todos los demás son identidad)

Aplicamos `M × (1,0,0)`:  
`x = m[0]*1 = 0`, `y = m[1]*1 = 0`, `z = m[2]*1 = -1`  
→ El vector `+X` se convierte en `-Z`. ✓ (rotación horaria alrededor de Y)

Aplicamos `Mᵀ × (0,0,-1)` para recuperar el original:  
`x = m[0]*0 + m[1]*0 + m[2]*(-1) = 0 + 0 + 1 = 1`  
`y = m[4]*0 + m[5]*0 + m[6]*(-1) = 0`  
`z = m[8]*0 + m[9]*0 + m[10]*(-1) = 0`  
→ Resultado: `(1, 0, 0)` = `+X`. ✓ Recuperamos la normal original correctamente.

### 6.2 Línea a línea

```cpp
const float* m = cubito.orientacion.m;
```
Obtiene el puntero al arreglo de 16 floats de la matriz `orientacion`.

```cpp
const int ix = static_cast<int>(std::round(m[0]*dx + m[1]*dy + m[2]*dz));
const int iy = static_cast<int>(std::round(m[4]*dx + m[5]*dy + m[6]*dz));
const int iz = static_cast<int>(std::round(m[8]*dx + m[9]*dy + m[10]*dz));
```
Calcula `Mᵀ × (dx, dy, dz)` usando las tres primeras filas de `M` (que son las columnas de
`Mᵀ`). El resultado es la **normal original** del sticker que ahora apunta en la dirección
`(dx, dy, dz)`. Se redondea al entero más cercano porque la matriz puede tener pequeños
errores de punto flotante acumulados.

```cpp
if (iz ==  1) return 'F';  // White  — Front (+Z)
if (iy ==  1) return 'U';  // Red    — Up    (+Y)
if (ix ==  1) return 'R';  // Green  — Right (+X)
if (iz == -1) return 'B';  // Yellow — Back  (-Z)
if (iy == -1) return 'D';  // Orange — Down  (-Y)
if (ix == -1) return 'L';  // Blue   — Left  (-X)
```
Compara la normal original recuperada con la tabla de colores iniciales (sección 2.3).
Cada caso corresponde a un color específico que se asignó en `crearCubito()`:
- Si la normal original era `+Z` → el sticker era blanco → cara Front → letra `'F'`
- Si la normal original era `+Y` → el sticker era rojo → cara Up → letra `'U'`
- etc.

---

## 7. La función principal `getEstado()`

```cpp
inline std::string RubickCube::getEstado() const {
    static const int normals[6][3] = {
        { 0, 1, 0},  // U
        { 1, 0, 0},  // R
        { 0, 0, 1},  // F
        { 0,-1, 0},  // D
        {-1, 0, 0},  // L
        { 0, 0,-1}   // B
    };
    static const int pos[6][9][3] = {
        {{-1,1,-1},{0,1,-1},{1,1,-1},{-1,1,0},{0,1,0},{1,1,0},{-1,1,1},{0,1,1},{1,1,1}},
        {{1,1,1},{1,1,0},{1,1,-1},{1,0,1},{1,0,0},{1,0,-1},{1,-1,1},{1,-1,0},{1,-1,-1}},
        {{-1,1,1},{0,1,1},{1,1,1},{-1,0,1},{0,0,1},{1,0,1},{-1,-1,1},{0,-1,1},{1,-1,1}},
        {{-1,-1,1},{0,-1,1},{1,-1,1},{-1,-1,0},{0,-1,0},{1,-1,0},{-1,-1,-1},{0,-1,-1},{1,-1,-1}},
        {{-1,1,-1},{-1,1,0},{-1,1,1},{-1,0,-1},{-1,0,0},{-1,0,1},{-1,-1,-1},{-1,-1,0},{-1,-1,1}},
        {{1,1,-1},{0,1,-1},{-1,1,-1},{1,0,-1},{0,0,-1},{-1,0,-1},{1,-1,-1},{0,-1,-1},{-1,-1,-1}}
    };
    std::string estado;
    estado.reserve(54);
    for (int f = 0; f < 6; ++f)
        for (int i = 0; i < 9; ++i)
            estado += colorEnDireccion(*cubitoEn(pos[f][i][0], pos[f][i][1], pos[f][i][2]),
                                       normals[f][0], normals[f][1], normals[f][2]);
    return estado;
}
```

### 7.1 La tabla de normales por cara

```cpp
static const int normals[6][3] = {
    { 0, 1, 0},  // U — mirando hacia +Y (arriba)
    { 1, 0, 0},  // R — mirando hacia +X (derecha)
    { 0, 0, 1},  // F — mirando hacia +Z (frente)
    { 0,-1, 0},  // D — mirando hacia -Y (abajo)
    {-1, 0, 0},  // L — mirando hacia -X (izquierda)
    { 0, 0,-1}   // B — mirando hacia -Z (atrás)
};
```

Para leer la cara Up, la dirección que preguntamos a cada cubito es `(0, +1, 0)`: "¿qué
sticker tuyo está mirando hacia arriba ahora mismo?". El vector normal es la dirección desde
el interior del cubo hacia afuera de esa cara.

### 7.2 La tabla de posiciones por cara — explicación visual

El arreglo `pos[6][9][3]` tiene tres dimensiones con este significado:

```
pos [ cara ] [ número de sticker ] [ coordenada ]
      0..5       0..8                 0=x, 1=y, 2=z
```

- **6** = 6 caras del cubo (U, R, F, D, L, B)
- **9** = 9 stickers por cara (grilla 3×3)
- **3** = las coordenadas `(x, y, z)` del cubito que tiene ese sticker

Para acceder a las coordenadas del sticker 3 de la cara R:
```cpp
pos[1][2][0]  →  x del cubito
pos[1][2][1]  →  y del cubito
pos[1][2][2]  →  z del cubito
```

El orden de los 9 stickers no es aleatorio: sigue el orden de **lectura de texto**
(izquierda a derecha, arriba a abajo) visto desde fuera de esa cara. Este orden debe
coincidir exactamente con la convención que usa min2phase.

---

#### Cara U — `y = 1` (mirando desde arriba hacia abajo)

```
           [atrás, z=-1]
        ┌───┬───┬───┐
        │ 1 │ 2 │ 3 │   y=1, z=-1,  x: -1 → 0 → +1
        ├───┼───┼───┤
        │ 4 │ 5 │ 6 │   y=1, z= 0,  x: -1 → 0 → +1
        ├───┼───┼───┤
        │ 7 │ 8 │ 9 │   y=1, z=+1,  x: -1 → 0 → +1
        └───┴───┴───┘
           [frente, z=+1]
```

Mirando desde arriba, la fila "de atrás" (`z=-1`) queda en la parte superior de lo que ves.
El eje x va de izquierda a derecha normalmente.

```cpp
// Cara U en el arreglo pos:
{{-1,1,-1},{0,1,-1},{1,1,-1},   // stickers 1,2,3  (z=-1, fila de atrás)
 {-1,1, 0},{0,1, 0},{1,1, 0},   // stickers 4,5,6  (z= 0, fila del medio)
 {-1,1,+1},{0,1,+1},{1,1,+1}}   // stickers 7,8,9  (z=+1, fila del frente)
```

---

#### Cara F — `z = +1` (mirando de frente hacia ti)

```
        ┌───┬───┬───┐
        │ 1 │ 2 │ 3 │   z=+1, y=+1,  x: -1 → 0 → +1
        ├───┼───┼───┤
        │ 4 │ 5 │ 6 │   z=+1, y= 0,  x: -1 → 0 → +1
        ├───┼───┼───┤
        │ 7 │ 8 │ 9 │   z=+1, y=-1,  x: -1 → 0 → +1
        └───┴───┴───┘
```

La fila superior tiene `y=+1` (arriba) y la inferior `y=-1` (abajo).
El eje x va de izquierda a derecha normalmente.

```cpp
// Cara F en el arreglo pos:
{{-1,+1,1},{0,+1,1},{1,+1,1},   // stickers 1,2,3  (y=+1, arriba)
 {-1, 0,1},{0, 0,1},{1, 0,1},   // stickers 4,5,6  (y= 0, centro)
 {-1,-1,1},{0,-1,1},{1,-1,1}}   // stickers 7,8,9  (y=-1, abajo)
```

---

#### Cara R — `x = +1` (mirando desde la derecha)

```
   [frente z=+1]              [atrás z=-1]
        ┌───┬───┬───┐
        │ 1 │ 2 │ 3 │   x=+1, y=+1,  z: +1 → 0 → -1
        ├───┼───┼───┤
        │ 4 │ 5 │ 6 │   x=+1, y= 0,  z: +1 → 0 → -1
        ├───┼───┼───┤
        │ 7 │ 8 │ 9 │   x=+1, y=-1,  z: +1 → 0 → -1
        └───┴───┴───┘
```

**¿Por qué z va de +1 a -1?**
Cuando te paras a la derecha del cubo mirando hacia la izquierda (-X), el frente
del cubo (`z=+1`) queda a tu izquierda y el fondo (`z=-1`) a tu derecha.
Leer de izquierda a derecha = leer z de `+1` a `-1`.

```cpp
// Cara R en el arreglo pos:
{{1,+1,+1},{1,+1,0},{1,+1,-1},   // stickers 1,2,3
 {1, 0,+1},{1, 0,0},{1, 0,-1},   // stickers 4,5,6
 {1,-1,+1},{1,-1,0},{1,-1,-1}}   // stickers 7,8,9
```

---

#### Cara B — `z = -1` (mirando desde atrás)

```
   [derecha x=+1]            [izquierda x=-1]
        ┌───┬───┬───┐
        │ 1 │ 2 │ 3 │   z=-1, y=+1,  x: +1 → 0 → -1
        ├───┼───┼───┤
        │ 4 │ 5 │ 6 │   z=-1, y= 0,  x: +1 → 0 → -1
        ├───┼───┼───┤
        │ 7 │ 8 │ 9 │   z=-1, y=-1,  x: +1 → 0 → -1
        └───┴───┴───┘
```

**¿Por qué x va de +1 a -1?**
Cuando te paras atrás mirando hacia adelante (+Z), la cara Right (`x=+1`) del cubo
queda a tu izquierda. Leer de izquierda a derecha = x de `+1` a `-1`.

```cpp
// Cara B en el arreglo pos:
{{+1,+1,-1},{0,+1,-1},{-1,+1,-1},   // stickers 1,2,3
 {+1, 0,-1},{0, 0,-1},{-1, 0,-1},   // stickers 4,5,6
 {+1,-1,-1},{0,-1,-1},{-1,-1,-1}}   // stickers 7,8,9
```

---

### 7.3 El bucle que construye el string — línea a línea

```cpp
std::string estado;
estado.reserve(54);
```
Crea un string vacío y reserva espacio para 54 caracteres (6 caras × 9 stickers).
Es solo una optimización para no redimensionar la memoria mientras se llena.

```cpp
for (int f = 0; f < 6; ++f)        // f = cara (0=U, 1=R, 2=F, 3=D, 4=L, 5=B)
    for (int i = 0; i < 9; ++i)    // i = sticker (0 al 8, en orden de lectura)
```
Dos bucles anidados: recorre las 6 caras, y por cada cara sus 9 stickers en orden.

```cpp
estado += colorEnDireccion(
    *cubitoEn(pos[f][i][0], pos[f][i][1], pos[f][i][2]),
    normals[f][0], normals[f][1], normals[f][2]
);
```

Esta línea tiene **dos partes** que trabajan juntas:

**Parte 1 — `cubitoEn(pos[f][i][0], pos[f][i][1], pos[f][i][2])`**

Usa el arreglo `pos` para obtener las coordenadas `(x,y,z)` del cubito que corresponde
a ese sticker, y lo busca entre los 27 cubitos. El `*` al inicio desreferencia el puntero
para obtener el objeto.

Ejemplo para cara U (`f=0`), sticker 1 (`i=0`):
```
pos[0][0][0] = -1
pos[0][0][1] =  1    →  cubitoEn(-1, 1, -1)
pos[0][0][2] = -1
```
Busca el cubito en la esquina trasera-izquierda-arriba.

**Parte 2 — `normals[f][0], normals[f][1], normals[f][2]`**

Le dice a `colorEnDireccion` hacia qué dirección mirar ese cubito. Como es la cara U
(`f=0`), la normal es `(0, 1, 0)`: "dime qué color de este cubito apunta hacia arriba".

**El resultado**: un char (`'U'`, `'R'`, `'F'`, etc.) que se concatena al string.

Imaginando el recorrido completo:
```
f=0 (cara U), i=0..8  →  agrega 9 letras: "UUUUURUUU"  (ejemplo mezclado)
f=1 (cara R), i=0..8  →  agrega 9 letras: "RRRRRRRRR"
f=2 (cara F), i=0..8  →  agrega 9 letras: "FFFUFFFFF"
f=3 (cara D), i=0..8  →  agrega 9 letras: "DDDDDDDDD"
f=4 (cara L), i=0..8  →  agrega 9 letras: "LLLLLLLLL"
f=5 (cara B), i=0..8  →  agrega 9 letras: "BBBBBBBBB"
                                            ──────────
                                            54 chars total
```

### 7.4 Ejemplo de salida

Cubo en estado resuelto (cada cara tiene sus 9 stickers del mismo color):
```
UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB
```

Cubo mezclado (algunos stickers están en la cara equivocada):
```
UUDUUFUUURRRURRRRRFFFBFFFFFDDDRDDDDDRLLLLLLLLBBBBBBBBB
    ^  ^      ^       ^    ^      ^
    stickers fuera de lugar
```

---

## 8. División de responsabilidades: el solver piensa, el código mueve

Este es el punto conceptual más importante de toda la arquitectura. El algoritmo
**no mueve nada** por sí solo. Solo piensa y devuelve una lista de instrucciones.
Quien ejecuta los movimientos con animación es el código que ya existe en el proyecto.

```
Tu código                          min2phase (solver)
─────────────────                  ──────────────────────────────
getEstado()          ──────────►   recibe "UUDUUFUUR..."
                                   piensa con algoritmo Kociemba
colaMoves (cola)     ◄──────────   devuelve "U R2 F' B R..."
animación            ejecuta los movimientos uno por uno visualmente
```

### 8.1 Qué devuelve min2phase exactamente

Su función de entrada es:
```cpp
std::string Search::solution(std::string facelets, int maxDepth,
                             long timeOut, bool useSeparator);
```

Donde `facelets` es exactamente el string de 54 caracteres que produce `getEstado()`.

La función devuelve una cadena de texto con movimientos en notación estándar:
```
"U R2 F' B R B2 R U2 L B2 R U' D' R2 F R' L B2 U2 F2"
```

Cada token separado por espacio es un movimiento:

| Token | Significado |
|-------|-------------|
| `U`   | Cara Up, 90° horario |
| `U'`  | Cara Up, 90° antihorario |
| `U2`  | Cara Up, 180° (dos veces) |
| `R`   | Cara Right, 90° horario |
| `F'`  | Cara Front, 90° antihorario |

### 8.2 El solver no controla movimientos, solo calcula la solución

La idea clave es esta: min2phase es un **calculador**, no un **ejecutor**.
Recibe el estado del cubo, calcula la secuencia óptima de movimientos para
resolverlo (≤ 20 movimientos), y te la devuelve como texto. No sabe nada
de OpenGL, animaciones, ni de la clase `RubickCube`.

El que ejecuta los movimientos eres tú, usando el sistema de cola que ya existe:

```
"U R2 F' B R'"  →  parsear  →  colaMoves  →  animación frame a frame
```

### 8.3 El traductor: de notación Kociemba a Camada + dir

Para conectar min2phase con la cola, se parsea el string token por token y
se convierte cada movimiento al par `{Camada, dir}`:

```cpp
"U"  →  colaMoves.push({Up,    +1})
"U'" →  colaMoves.push({Up,    -1})
"U2" →  colaMoves.push({Up,    +1})   // dos veces
        colaMoves.push({Up,    +1})
"R"  →  colaMoves.push({Right, +1})
"F'" →  colaMoves.push({Front, -1})
"D2" →  colaMoves.push({Down,  +1})
        colaMoves.push({Down,  +1})
```

Tabla completa de traducción:

| Notación | `Camada` | `dir` |
|----------|----------|-------|
| `U`      | `Up`     | +1    |
| `U'`     | `Up`     | -1    |
| `U2`     | `Up`     | +1 × 2|
| `R`      | `Right`  | +1    |
| `R'`     | `Right`  | -1    |
| `F`      | `Front`  | +1    |
| `F'`     | `Front`  | -1    |
| `D`      | `Down`   | +1    |
| `D'`     | `Down`   | -1    |
| `L`      | `Left`   | +1    |
| `L'`     | `Left`   | -1    |
| `B`      | `Back`   | +1    |
| `B'`     | `Back`   | -1    |

Los movimientos de 180° (`X2`) se insertan dos veces en la cola con `dir=+1`.
Una vez llenada la cola, el sistema de animación que ya existe ejecuta todo
sin necesitar ningún cambio adicional.

---

## 9. Flujo completo: del cubo mezclado a la solución

```
┌─────────────────────────────────────────────────────────────────┐
│  1. Usuario mezcla el cubo (giros manuales o tecla R)           │
│     → coordenadas x,y,z cambian                                 │
│     → orientacion de cada cubito se acumula                     │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│  2. getEstado() construye el string de 54 chars                 │
│     Para cada posición de cada cara:                            │
│       cubitoEn(x,y,z) → encuentra el cubito en esa posición    │
│       colorEnDireccion(cubito, nx,ny,nz):                       │
│         → Mᵀ × (nx,ny,nz) → normal original                    │
│         → normal original → letra Kociemba                      │
│     Resultado: "UURUUULUU..." (54 chars)                        │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│  3. min2phase recibe el string                                  │
│     Ejecuta algoritmo de Kociemba de dos fases                  │
│     Devuelve: "U R2 F B R' ..."                                 │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│  4. Parseo: "U R2 F B R' ..." → vector<pair<Camada,int>>        │
│     "U"  → {Up,    +1}                                          │
│     "R2" → {Right, +1}, {Right, +1}                             │
│     "F"  → {Front, +1}                                          │
│     "R'" → {Right, -1}                                          │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│  5. Los movimientos se meten en colaMoves                       │
│     El loop principal los ejecuta uno a uno con animación       │
│     El cubo se resuelve visualmente en pantalla                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 10. Resumen de las tres funciones implementadas

| Función           | Rol                                          | Dónde se usa          |
|-------------------|----------------------------------------------|-----------------------|
| `cubitoEn(x,y,z)` | Búsqueda de cubito por posición actual       | Llamada desde `getEstado()` |
| `colorEnDireccion(cubito, dx,dy,dz)` | Aplica Mᵀ para recuperar el color que mira en una dirección | Llamada desde `getEstado()` |
| `getEstado()`     | Construye el string Kociemba de 54 chars     | Entrada para min2phase |

**Sin `getEstado()`, el solver no puede saber en qué estado está el cubo.** Las matrices
de orientación son perfectas para dibujar, pero completamente opacas para un algoritmo
que necesita saber "¿el sticker rojo de la esquina superior derecha está mirando hacia
arriba o hacia la derecha?". `getEstado()` convierte esa información geométrica en el
lenguaje que el solver entiende.
