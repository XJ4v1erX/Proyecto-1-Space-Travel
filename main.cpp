#include <SDL.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>
#include "FrameBuffer.h"
#include "GraphicsHeaders.h"
#include "ObjLoader.h"
#include "Shaders.h"

#include <string>


SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
Color currentColor;

const std::string modelPathSphere = "../OBJS/planeta.obj";
const std::string modelPathSpaceShip = "../OBJS/lab3.obj";
Color clearColor(0, 0, 0);  // Color del fondo

std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<Face> faces;
std::vector<Vertex> verticesArray;

std::vector<glm::vec3> spaceShipVertices;
std::vector<glm::vec3> spaceShipNormals;
std::vector<Face> spaceShipFaces;
std::vector<Vertex> spaceShipVerticesArray;


Uniforms uniforms;

glm::mat4 model = glm::mat4(1);
glm::mat4 view = glm::mat4(1);
glm::mat4 projection = glm::mat4(1);

enum class Primitive {
    TRIANGLES,
};

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error: No se puedo inicializar SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Proyecto 1: Space Travel", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Error: No se pudo crear una ventana SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Error: No se pudo crear SDL_Renderer: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

std::vector<std::vector<Vertex>> primitiveAssembly(
        Primitive polygon,
        const std::vector<Vertex>& transformedVertices
) {
    std::vector<std::vector<Vertex>> assembledVertices;

    switch (polygon) {
        case Primitive::TRIANGLES: {
            assert(transformedVertices.size() % 3 == 0 && "El número de vértices debe ser un múltiplo de 3 para triángulos.");

            for (size_t i = 0; i < transformedVertices.size(); i += 3) {
                std::vector<Vertex> triangle = {
                        transformedVertices[i],
                        transformedVertices[i + 1],
                        transformedVertices[i + 2]
                };
                assembledVertices.push_back(triangle);
            }
            break;
        }
        default:
            std::cerr << "Error: No se reconoce el tipo primitivo." << std::endl;
            break;
    }

    return assembledVertices;
}

std::vector<Fragment> rasterize(Primitive primitive, const std::vector<std::vector<Vertex>>& assembledVertices, std::string name, glm::vec3 worldPos) {
    std::vector<Fragment> fragments;

    switch (primitive) {
        case Primitive::TRIANGLES: {
            for (const std::vector<Vertex>& triangleVertices : assembledVertices) {
                assert(triangleVertices.size() == 3 && "Triangle vertices must contain exactly 3 vertices.");
                std::vector<Fragment> triangleFragments = triangle(triangleVertices[0], triangleVertices[1],
                                                                   triangleVertices[2], name, worldPos);
                fragments.insert(fragments.end(), triangleFragments.begin(), triangleFragments.end());
            }
            break;
        }
        default:
            std::cerr << "Error: No se reconoce el tipo primitivo para rasterización." << std::endl;
            break;
    }

    return fragments;
}

glm::mat4 createProjectionMatrix() {
    float fovInDegrees = 45.0f;
    float aspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
    float nearClip = 0.1f;
    float farClip = 100.0f;

    return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

glm::mat4 createViewportMatrix(size_t screenWidth, size_t screenHeight) {
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale
    viewport = glm::scale(viewport, glm::vec3(screenWidth / 2.0f, screenHeight / 2.0f, 0.5f));

    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

void render(Primitive polygon, std::string name, std::vector<Vertex>& modelVertices, glm::vec3 worldPos){

    // 1. Vertex Shader
    std::vector<Vertex> transformedVertices;

    for (const Vertex& vertex : modelVertices) {
        transformedVertices.push_back(vertexShader(vertex, uniforms));
    }

    // 2. Primitive Assembly
    std::vector<std::vector<Vertex>> assembledVertices = primitiveAssembly(polygon, transformedVertices);

    // 3. Rasterization
    std::vector<Fragment> fragments = rasterize(polygon, assembledVertices, name, worldPos);

    // 4. Fragment Shader
    for (Fragment& fragment : fragments) {
        // Apply the fragment shader to compute the final color
        fragment = fragmentShader(fragment, name);
        point(fragment);
    }

}

void setUpRender(Planet& model) {
    float rotationAngle = model.rotationAngle;
    float translationAngle = model.translationAngle;

    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);

    if (model.name != "sun") {
        model.rotationAngle += 1;
        model.translationAngle += model.translationSpeed;
    }
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), rotationAxis);

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), model.scaleFactor);

    model.worldPos.x = model.translationAxis.x + (model.translationRadius * cos(glm::radians(translationAngle)));
    model.worldPos.z = model.translationAxis.z + (model.translationRadius * sin(glm::radians(translationAngle)));

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), model.worldPos);

    // Calcular la matriz de modelo
    uniforms.model = translation * rotation * scale;
}
void drawLine(Vertex start, Vertex end) {
    int x0 = static_cast<int>(start.position.x);
    int y0 = static_cast<int>(start.position.y);
    int x1 = static_cast<int>(end.position.x);
    int y1 = static_cast<int>(end.position.y);

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        if (x0 >= 0 && y0 >= 0 && x0 < SCREEN_WIDTH && y0 < SCREEN_HEIGHT) {
            Fragment fragment = {
                    glm::ivec2(x0, y0),
                    Color(200, 200, 200), // Color de la línea
                    0,
                    1.0f,
                    glm::vec3(x0, y0, 0)
            };
            point(fragment);
        }

        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}


void setUpOrbit(Planet& planet) {
    std::vector<Vertex> orbitsVertices;

    // Agregar puntos para 360 grados
    for (float i = 0.0f; i <= 360.0f; i += 1.0f) {
        glm::vec3 position = glm::vec3(
                planet.translationAxis.x + planet.translationRadius * cos(glm::radians(i)),
                0.0f,
                planet.translationAxis.z + planet.translationRadius * sin(glm::radians(i))
        );
        orbitsVertices.push_back({position, glm::vec3(1.0f)});
    }

    // Transformar y dibujar líneas entre cada par de vértices consecutivos
    for (size_t i = 0; i < orbitsVertices.size() - 1; i++) {
        Vertex transformedStart = vertexShader(orbitsVertices[i], uniforms);
        Vertex transformedEnd = vertexShader(orbitsVertices[i + 1], uniforms);
        drawLine(transformedStart, transformedEnd);
    }
}





void fastTravel(SDL_Renderer* renderer, const Camera& camera) {
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    Uint32 startTime = SDL_GetTicks();
    const Uint32 animationDuration = 750;

    while (true) {
        Uint32 currentTime = SDL_GetTicks();

        Uint32 elapsedTime = currentTime - startTime;

        if (elapsedTime >= animationDuration) {
            break;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < SCREEN_HEIGHT; y += 2) {
            for (int x = 0; x < SCREEN_WIDTH; x += 2) {
                float noiseValue = noise.GetNoise((x + camera.cameraPosition.x) * 30.0f, (y + camera.cameraPosition.y) * 30.0f);

                if (noiseValue > 0.75f) {
                    SDL_SetRenderDrawColor(renderer, 225, 225, 225, 255);

                    if (x < SCREEN_WIDTH / 2 && y < SCREEN_HEIGHT / 2)
                        SDL_RenderDrawLine(renderer, x - elapsedTime / 100, y - elapsedTime / 100, x, y );

                    else if (x > SCREEN_WIDTH / 2 && y < SCREEN_HEIGHT / 2)
                        SDL_RenderDrawLine(renderer, x + elapsedTime / 100, y - elapsedTime / 100, x, y);

                    else if (x < SCREEN_WIDTH / 2 && y > SCREEN_HEIGHT / 2)
                        SDL_RenderDrawLine(renderer, x - elapsedTime / 100, y + elapsedTime / 100, x, y);

                    else if (x > SCREEN_WIDTH / 2 && y > SCREEN_HEIGHT / 2)
                        SDL_RenderDrawLine(renderer, x + elapsedTime / 100, y + elapsedTime / 100, x, y);
                }
            }
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(10);
    }
}

int main(int argv, char** args) {
    if (!init()) {
        return 1;
    }

    clear(10, 10);

    // Inicializar cámara
    float cameraSpeed = 0.1f;

    Camera camera{};
    camera.cameraPosition = glm::vec3(0.0f, 15.0f, 15.0f);
    camera.targetPosition = glm::vec3(0.0f, 5.0f, 0.0f);
    camera.upVector = glm::vec3(0.0f, 5.0f, 0.0f);

    // Matriz de proyección
    uniforms.projection = createProjectionMatrix();

    // Matriz de viewport
    uniforms.viewport = createViewportMatrix(SCREEN_WIDTH, SCREEN_HEIGHT);

    /*PREPARAR UNIFORMS*/

    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f); // Rotar alrededor del eje Y


    Planet sol;
    sol.name = "sol";
    sol.worldPos = {0.0f, 0.0f, 0.0f};
    sol.rotationAngle = 0.0f;
    sol.scaleFactor = {1.5f, 1.5f, 1.5f};
    sol.translationRadius = 0.0f;
    sol.translationAxis = {0.0f, 0.0f, 0.0f};


    Planet rocoso;
    rocoso.name = "rocoso";
    rocoso.translationRadius = 2.0f;
    rocoso.rotationAngle = 1.0f;
    rocoso.scaleFactor = {1.0f, 1.0f, 1.0f};
    rocoso.translationAngle = 170.0f;
    rocoso.translationSpeed = 1.0f;
    rocoso.worldPos = {rocoso.translationRadius * cos(glm::radians(rocoso.translationAngle)), 0.0f,
                       rocoso.translationRadius * sin(glm::radians(rocoso.translationAngle))};
    rocoso.translationAxis = sol.worldPos;


    Planet tierra;
    tierra.name = "tierra";
    tierra.translationRadius = 4.0f;
    tierra.rotationAngle = 1.0f;
    tierra.scaleFactor = {1.0f, 1.0f, 1.0f};
    tierra.translationAngle = 100.0f;
    tierra.worldPos = {tierra.translationRadius * cos(glm::radians(tierra.translationAngle)), 0.0f,
                       tierra.translationRadius * sin(glm::radians(tierra.translationAngle))};
    tierra.translationSpeed = 0.5f;
    tierra.translationAxis = sol.worldPos;


    Planet gaseoso;
    gaseoso.name = "gaseoso";
    gaseoso.translationRadius = 7.0f;
    gaseoso.rotationAngle = 1.0f;
    gaseoso.scaleFactor = {0.8f, 0.8f, 0.8f};
    gaseoso.translationAngle = 0.0f;
    gaseoso.translationSpeed = 0.3f;
    gaseoso.worldPos = {gaseoso.translationRadius * cos(glm::radians(gaseoso.translationAngle)), 0.0f,
                        gaseoso.translationRadius * sin(glm::radians(gaseoso.translationAngle))};
    gaseoso.translationAxis = sol.worldPos;


    Planet portal;
    portal.name = "portal";
    portal.translationRadius = 12.0f;
    portal.rotationAngle = 1.0f;
    portal.scaleFactor = {1.3f, 1.3f, 1.3f};
    portal.translationAngle = 0.0f;
    portal.translationSpeed = 0.5f;
    portal.worldPos = {portal.translationRadius * cos(glm::radians(portal.translationAngle)), 0.0f,
                       portal.translationRadius * sin(glm::radians(portal.translationAngle))};
    portal.translationAxis = sol.worldPos;


    Planet shine;
    shine.name = "shine";
    shine.translationRadius = 15.0f;
    shine.rotationAngle = 0.0f;
    shine.scaleFactor = {1.1f, 1.1f, 1.1f};
    shine.translationAngle = 300.0f;
    shine.translationSpeed = 0.3f;
    shine.worldPos = {shine.translationRadius * cos(glm::radians(shine.translationAngle)), 0.0f,
                      shine.translationRadius * sin(glm::radians(shine.translationAngle))};
    shine.translationAxis = sol.worldPos;

    Planet luna;
    luna.name = "luna";
    luna.translationRadius = 1.0f;
    luna.rotationAngle = 0.0f;
    luna.scaleFactor = {0.2f, 0.2f, 0.2f};
    luna.translationAngle = 1.0f;
    luna.translationSpeed = 2.0f;
    luna.translationAxis = tierra.worldPos;
    luna.worldPos = {luna.translationAxis.x + (luna.translationRadius * cos(glm::radians(luna.translationAngle))),
                     0.3f,
                     luna.translationAxis.z + (luna.translationRadius * sin(glm::radians(luna.translationAngle)))};


    //Agregar satelites alrededor de sus correspondientes planetas
    tierra.satelites.push_back(luna);


    // Agregar planetas alrededor del Sol
    sol.satelites.push_back(rocoso);
    sol.satelites.push_back(tierra);
    sol.satelites.push_back(gaseoso);
    sol.satelites.push_back(portal);
    sol.satelites.push_back(shine);

    // Preparar uniforms de la nave
    Spaceship spaceship;

    spaceship.worldPos = {0.0f, 0.5f, 30.0f};
    spaceship.scaleFactor = {0.09f, 0.09f, 0.1f};
    spaceship.rotationAngle = 90.0f;
    spaceship.rotationSpeed = 2.0f;
    spaceship.movementSpeed = 1.0f;

    // Vertices de modelo spaceship
    loadOBJ(modelPathSpaceShip, spaceShipVertices, spaceShipNormals, spaceShipFaces);
    spaceShipVerticesArray = setupVertexArray(spaceShipVertices, spaceShipNormals, spaceShipFaces);

    // Vertices de modelo esfera
    loadOBJ(modelPathSphere, vertices, normals, faces);
    verticesArray = setupVertexArray(vertices, normals, faces);

    //Matriz de vista

    bool isRunning = true;
    while (isRunning) {
        frame += 1;
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {

            if (event.type == SDL_QUIT) {
                isRunning = false;
            }

            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_RIGHT:
                        if (spaceship.rotationAngle - spaceship.rotationSpeed == 0.0f) {
                            spaceship.rotationAngle = 360.0f;
                        }
                        else {
                            spaceship.rotationAngle -= spaceship.rotationSpeed;
                        }
                        break;

                    case SDLK_LEFT:
                        if (spaceship.rotationAngle + spaceship.rotationSpeed > 360.0f) {
                            spaceship.rotationAngle = spaceship.rotationSpeed;
                        }
                        else {
                            spaceship.rotationAngle += spaceship.rotationSpeed;
                        }
                        break;

                    case SDLK_UP:
                        spaceship.worldPos.z -= spaceship.movementSpeed * sin(glm::radians(spaceship.rotationAngle));
                        spaceship.worldPos.x += spaceship.movementSpeed * cos(glm::radians(spaceship.rotationAngle));
                        break;

                    case SDLK_DOWN:
                        spaceship.worldPos.z += spaceship.movementSpeed * sin(glm::radians(spaceship.rotationAngle));
                        spaceship.worldPos.x -= spaceship.movementSpeed * cos(glm::radians(spaceship.rotationAngle));
                        break;

                    case SDLK_r:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {0.0f, 0.0f, 20.0f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    case SDLK_1:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {sol.satelites[0].worldPos.x, 0.0f, sol.satelites[0].worldPos.z + 1.0f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    case SDLK_2:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {sol.satelites[1].worldPos.x, 0.0f, sol.satelites[1].worldPos.z + 1.0f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    case SDLK_3:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {sol.satelites[2].worldPos.x, 0.0f, sol.satelites[2].worldPos.z + 1.0f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    case SDLK_4:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {sol.satelites[3].worldPos.x, 0.0f, sol.satelites[3].worldPos.z + 1.5f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    case SDLK_5:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {sol.satelites[4].worldPos.x, 0.0f, sol.satelites[4].worldPos.z + 1.5f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    default:
                        break;
                }
            }
        }

        clear(10, 10);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Actualizar cámara

        // Crear la matriz de vista usando el objeto cámara
        uniforms.view = glm::lookAt(camera.cameraPosition, camera.targetPosition, camera.upVector);


        // Renderizar el Sol
        setUpRender(sol);
        render(Primitive::TRIANGLES, sol.name, verticesArray, sol.worldPos);

        // Renderizar orbitas

        // Renderizar nave
        glm::mat4 spaceshipTranslation = glm::translate(glm::mat4(5.0f), spaceship.worldPos);
        glm::mat4 spaceshipScale = glm::scale(glm::mat4(1.5f), spaceship.scaleFactor);
        glm::mat4 spaceshipRotation = glm::rotate(glm::mat4(1.0f), glm::radians(spaceship.rotationAngle), rotationAxis);
        uniforms.model = spaceshipTranslation * spaceshipRotation *  spaceshipScale;

        float d = 3.0f; //Distancia de camara a nave

        // Determinar posición de la cámara
        float cameraAngle;
        float dx;
        float dz;

        if (spaceship.rotationAngle > 0 && spaceship.rotationAngle <= 90) {
            // I cuadrante: de 1 a 90
            cameraAngle = spaceship.rotationAngle;
            dz = d * sin(glm::radians(cameraAngle));
            dx = d * cos(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x - dx, 1.0f, spaceship.worldPos.z + dz};
        }

        else if (spaceship.rotationAngle > 90 && spaceship.rotationAngle <= 180) {
            // II cuadrante: de 91 a 180
            cameraAngle = spaceship.rotationAngle - 90.0f;
            dz = d * cos(glm::radians(cameraAngle));
            dx = d * sin(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x + dx, 1.0f, spaceship.worldPos.z + dz};
        }

        else if (spaceship.rotationAngle > 180 && spaceship.rotationAngle <= 270) {
            // III cuadrante: de 181 a 270
            cameraAngle = spaceship.rotationAngle - 180.0f;
            dz = d * sin(glm::radians(cameraAngle));
            dx = d * cos(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x + dx, 1.0f, spaceship.worldPos.z - dz};
        }

        else if (spaceship.rotationAngle > 270 && spaceship.rotationAngle <= 360) {
            // IV cuadrante: de 271 a 360
            cameraAngle = spaceship.rotationAngle - 270.0f;
            dz = d * cos(glm::radians(cameraAngle));
            dx = d * sin(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x - dx, 1.0f, spaceship.worldPos.z - dz};
        }



        //Actualizar lookAt de camera
        camera.targetPosition = spaceship.worldPos;

        render(Primitive::TRIANGLES, "nave", spaceShipVerticesArray, spaceship.worldPos);


        for (Planet& planet : sol.satelites) {
            // Renderizar satelites
            for (Planet& satellite : planet.satelites) {
                satellite.translationAxis = planet.worldPos;
                setUpRender(satellite);
                render(Primitive::TRIANGLES, satellite.name, verticesArray, satellite.worldPos);
                renderBuffer(renderer);
            }

            // Renderizar planetas
            setUpRender(planet);
            render(Primitive::TRIANGLES, planet.name, verticesArray, planet.worldPos);
            renderBuffer(renderer);

        }

        SDL_RenderPresent(renderer);

    }

    return 0;
}