# Proyecto 1: Space Travel

## Descripción
Este proyecto consiste en crear una simulación del sistema solar con un software renderer diseñado por nosotros. Inspirado en [jsorrery](http://mgvez.github.io/jsorrery/), nuestro sistema no se limita a replicar el sistema solar existente; en cambio, exploramos la creación de uno imaginario.


### Demostración
[![Vista previa del video](https://i9.ytimg.com/vi_webp/umhsxiR9GHI/mq2.webp?sqp=CNTLkKsG-oaymwEmCMACELQB8quKqQMa8AEB-AH-CYAC0AWKAgwIABABGCQgZSghMA8=&rs=AOn4CLAByB22QZW1jRH8HSEOIMQTyr0poQ)](https://youtu.be/umhsxiR9GHI)



## Detalles Importantes
- Instant warping a diferentes puntos del sistema.
- Renderizado de una nave modelada por nosotros.
- Skybox con estrellas en el horizonte.
- Condiciones para evitar que la nave/cámara atraviese elementos del sistema.

## Estructura del Proyecto
- `main.cpp`: Lógica principal de renderizado.
- `FastNoiseLite.h`: Generación de ruido para shaders.
- `CMakeLists.txt`: Configuración de compilación.
- `Shaders.h`: Shaders para cuerpos celestes.
- `GraphicsHeaders.h` y `FrameBuffer.h`: Estructuras para renderizado.
- `ObjLoader.h`: Carga de modelos 3D.
- `planeta.obj`: Modelo esférico para planetas.
- `nave.obj`: Modelo de la nave.

## Uso
1. Clone el repositorio de GitHub.
2. Compile con CMake y las dependencias necesarias.
3. Ejecute el binario para iniciar la simulación.
4. Interactúe con el mouse o teclado para explorar.

## Puntos Completados

| Criterio                                                     | Puntos Posibles | Puntos Completados |
|--------------------------------------------------------------|-----------------|--------------------|
| Estética del sistema completo                                  | 30              | ?                  |
| Performance de la escena completa                             | 20              | 20                  |
| Puntos por cada planeta / estrella / luna en el sistema       | 50              | 50                  |
| Implementación de instant warping a diferentes puntos del sistema solar | 10    | 0                  |
| Efectos animados                                              | 10              | 0                  |
| Renderización de una nave modelada por ustedes que siga a su cámara | 30        | 0                  |
| Creación de un skybox que muestre estrellas en el horizonte   | 10              | 10                  |
| Implementación de condicionales que eviten que la nave/cámara atraviese elementos del sistema solar | 10 | 00  |
| Implementación de movimiento 3D para la cámara                | 40              | 40                 |
| Renderización de las órbitas de los planetas                  | 20              | 0                  |


| Total de Puntos Completados |
|-----------------------------|
| 120                         |

## Autor
- Javier Ramírez
