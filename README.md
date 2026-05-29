# Cubo Rubik - Computación Gráfica

Proyecto de cubo Rubik desarrollado en C++ con OpenGL.  
Muchachos, aquí están los cambios. Son bastantes teclas, así que se las dejo aquí para que todos puedan usar el cubo correctamente.

---

## Controles de cámara

| Tecla | Acción |
|---|---|
| `F` | Vista frontal |
| `B` | Vista trasera |
| `L` | Vista izquierda |
| `R` | Vista derecha |
| `U` | Vista superior |
| `Q` | Vista inferior |
| `O` | Modo órbita global |
| `W` | Orbitar hacia arriba |
| `S` | Orbitar hacia abajo |
| `A` | Orbitar hacia la izquierda |
| `D` | Orbitar hacia la derecha |
| Click derecho + mouse | Orbitar con el mouse |

---

## Controles de camadas por vista

Primero se debe elegir una vista con:

```txt
F, B, L, R, U o Q
```

Luego se selecciona una camada visible usando las teclas del `1` al `6`.

La selección depende de la vista actual de la cámara.

---

## Selección de camadas visibles

### Vista frontal `F`

| Tecla | Camada visible |
|---|---|
| `1` | Columna izquierda |
| `2` | Columna central |
| `3` | Columna derecha |
| `4` | Fila superior |
| `5` | Fila central |
| `6` | Fila inferior |

---

### Vista trasera `B`

| Tecla | Camada visible |
|---|---|
| `1` | Columna izquierda vista desde atrás |
| `2` | Columna central vista desde atrás |
| `3` | Columna derecha vista desde atrás |
| `4` | Fila superior |
| `5` | Fila central |
| `6` | Fila inferior |

---

### Vista izquierda `L`

| Tecla | Camada visible |
|---|---|
| `1` | Columna izquierda de la vista |
| `2` | Columna central de la vista |
| `3` | Columna derecha de la vista |
| `4` | Fila superior |
| `5` | Fila central |
| `6` | Fila inferior |

---

### Vista derecha `R`

| Tecla | Camada visible |
|---|---|
| `1` | Columna izquierda de la vista |
| `2` | Columna central de la vista |
| `3` | Columna derecha de la vista |
| `4` | Fila superior |
| `5` | Fila central |
| `6` | Fila inferior |

---

### Vista superior `U`

| Tecla | Camada visible |
|---|---|
| `1` | Columna izquierda vista desde arriba |
| `2` | Columna central vista desde arriba |
| `3` | Columna derecha vista desde arriba |
| `4` | Fila superior vista desde arriba |
| `5` | Fila central vista desde arriba |
| `6` | Fila inferior vista desde arriba |

---

### Vista inferior `Q`

| Tecla | Camada visible |
|---|---|
| `1` | Columna izquierda vista desde abajo |
| `2` | Columna central vista desde abajo |
| `3` | Columna derecha vista desde abajo |
| `4` | Fila superior vista desde abajo |
| `5` | Fila central vista desde abajo |
| `6` | Fila inferior vista desde abajo |

---

## Giro de camadas en modo por vista

Después de seleccionar una camada del `1` al `6`, se gira usando las flechas.

| Camada seleccionada | Flechas válidas | Acción |
|---|---|---|
| `1`, `2`, `3` | `↑` / `↓` | Giran columnas |
| `4`, `5`, `6` | `→` / `←` | Giran filas |

Ejemplo:

```txt
F  -> Vista frontal
1  -> Selecciona la columna izquierda
↑  -> Gira esa columna hacia arriba
```

Otro ejemplo:

```txt
R  -> Vista derecha
4  -> Selecciona la fila superior visible
→  -> Gira esa fila hacia la derecha
```

---

## Modo global de 9 camadas

También existe un modo global presionando:

```txt
P
```

En este modo no se seleccionan camadas según la vista, sino directamente las 9 camadas reales del cubo.

| Tecla | Camada global |
|---|---|
| `1` | Camada izquierda |
| `2` | Camada central en eje X |
| `3` | Camada derecha |
| `4` | Camada superior |
| `5` | Camada central en eje Y |
| `6` | Camada inferior |
| `7` | Camada frontal |
| `8` | Camada central en eje Z |
| `9` | Camada trasera |

Para girar una camada en modo global:

| Flecha | Acción |
|---|---|
| `→` | Giro positivo |
| `↑` | Giro positivo |
| `←` | Giro negativo |
| `↓` | Giro negativo |

Ejemplo:

```txt
P  -> Activa el modo global
7  -> Selecciona la camada frontal
→  -> Gira la camada frontal
```

Otro ejemplo:

```txt
P  -> Activa el modo global
5  -> Selecciona la camada central en eje Y
←  -> Gira la camada en sentido contrario
```

---

## Salir del programa

Para cerrar la ventana:

```txt
ESC
```

---

## Notas importantes

- Las teclas `F`, `B`, `L`, `R`, `U` y `Q` cambian la vista de la cámara.
- Después de elegir una vista, también se puede orbitar usando `W`, `A`, `S` y `D`.
- El mouse solo orbita si se mantiene presionado el click derecho.
- En modo por vista se usan las teclas `1` a `6`.
- En modo global `P` se usan las teclas `1` a `9`.
- Las flechas ya no rotan todo el cubo, solo giran la camada seleccionada.
- Las rotaciones se animan antes de actualizar la estructura interna del cubo.
