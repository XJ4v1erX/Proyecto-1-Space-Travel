#include "glm/glm.hpp"
#include <SDL.h>
#include <string>
#include "FastNoiseLite.h"
#include "GraphicsHeaders.h"

FastNoiseLite sunNoise;
static int frame = 0;
Color getSunNoise(float x, float y, float z) {
    Color fragmentColor;

    // Cambiando los colores para un efecto diferente
    glm::vec3 darkColor = glm::vec3(0.90f, 0.15f, 0.00f); // Color más rojo oscuro
    glm::vec3 brightColor = glm::vec3(1.0f, 0.8f, 0.2f); // Color amarillo brillante

    float noise;

    sunNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    // Ajustando la velocidad de la animación
    float animationSpeed = 0.02f;
    float time = frame * animationSpeed;
    // Ajuste en la frecuencia para un efecto más dinámico
    sunNoise.SetFrequency(0.04 + (sin(3 * time) + 1) * 0.025);
    int zoom = 500; // Cambio en el zoom para variar la escala del ruido
    noise = (1 + sunNoise.GetNoise(x * zoom, y * zoom, z * zoom)) / 2.5f;

    // Cambiando la probabilidad del destello blanco
    float flashProbability = 0.003;
    if (rand() / static_cast<float>(RAND_MAX) < flashProbability) {
        fragmentColor = Color(1.0f, 1.0f, 1.0f);
    } else {
        // Cambiando la probabilidad de la erupción
        float eruptionProbability = 0.002;
        if (rand() / static_cast<float>(RAND_MAX) < eruptionProbability) {
            fragmentColor = Color(1.0f, 0.6f, 0.1f); // Color de erupción más claro
        } else {
            glm::vec3 sunNoiseColor = mix(brightColor, darkColor, noise * 1.8f);
            fragmentColor = Color(sunNoiseColor.x, sunNoiseColor.y, sunNoiseColor.z);
        }
    }

    return fragmentColor;
}

// gas planet
FastNoiseLite gaseousNoise;

Color getGaseous(float x, float y, float z) {
    Color fragmentColor;

    // Nuevo esquema de colores
    glm::vec3 primaryColor = {0.7f, 0.3f, 0.0f}; // Naranja
    glm::vec3 secondaryColor = {0.2f, 0.2f, 0.7f}; // Azul oscuro

    // Cambiando el tipo de ruido a Simplex con modulación
    gaseousNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    gaseousNoise.SetFrequency(0.1);
    gaseousNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    gaseousNoise.SetFractalOctaves(4);
    gaseousNoise.SetFractalLacunarity(2.0);
    gaseousNoise.SetFractalGain(0.5);

    float zoom = 30.0f;

    float noise = gaseousNoise.GetNoise(x * zoom, y * zoom, z * zoom);

    // Creando un patrón de mezcla basado en el ruido
    float mixFactor = (noise + 1.0f) / 2.0f; // Normalizar el ruido entre 0 y 1

    // Mezcla de colores primario y secundario basado en el ruido
    glm::vec3 finalColor = mix(primaryColor, secondaryColor, mixFactor);

    fragmentColor = Color(finalColor.x, finalColor.y, finalColor.z);

    return fragmentColor;
}
FastNoiseLite rocosoNoise;

Color getRocoso(float x, float y, float z) {
    Color fragmentColor;

    // Colores para superficie rocosa
    glm::vec3 colorRoca = {0.54f, 0.42f, 0.30f}; // Marrón
    glm::vec3 colorRocaClara = {0.72f, 0.67f, 0.60f}; // Beige

    // Cambiando el tipo de ruido para un efecto más rocoso
    rocosoNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    rocosoNoise.SetFrequency(0.1);
    rocosoNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);

    int zoom = 300;

    float noise = rocosoNoise.GetNoise(x * zoom, y * zoom, z * zoom);

    // Aplicando una modulación de color más suave y rocosa
    float mixFactor = (noise + 1.0f) / 2.0f; // Normalizar el ruido entre 0 y 1

    // Mezcla de colores para un efecto rocoso
    glm::vec3 finalColor = mix(colorRoca, colorRocaClara, mixFactor);

    // Asegurando que los valores de color estén dentro del rango [0, 1]
    finalColor = glm::clamp(finalColor, 0.0f, 1.0f);

    fragmentColor = Color(finalColor.x, finalColor.y, finalColor.z);

    return fragmentColor;
}

FastNoiseLite worldNoise;
FastNoiseLite surfaceNoise;
FastNoiseLite atmosphereNoise;

Color getWorldNoise(float x, float y, float z) {
    // Configuraciones del ruido para la generación de paisajes terrestres y marinos
    worldNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    worldNoise.SetFrequency(0.004); // Bajando la frecuencia para patrones más grandes
    worldNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    worldNoise.SetFractalOctaves(5);

    surfaceNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    surfaceNoise.SetFrequency(0.008); // Ajuste para la variación de la superficie
    surfaceNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    surfaceNoise.SetFractalOctaves(4);

    atmosphereNoise.SetNoiseType(FastNoiseLite::NoiseType_Value);
    atmosphereNoise.SetFrequency(0.012); // Ajuste para nubes
    atmosphereNoise.SetFractalOctaves(6);

    // Colores
    glm::vec3 landColor = {0.6f, 0.5f, 0.4f}; // Marrón claro para la tierra
    glm::vec3 vegetationColor = {0.3f, 0.6f, 0.2f}; // Verde para la vegetación
    glm::vec3 oceanColor = {0.1f, 0.2f, 0.5f}; // Azul marino para el océano
    glm::vec3 seaColor = {0.2f, 0.4f, 0.7f}; // Azul claro para el mar
    glm::vec3 skyColor = {0.8f, 0.8f, 1.0f}; // Azul cielo para las nubes
    Color fragmentColor;

    float zoom = 375.0f;
    float firstLayer = worldNoise.GetNoise(x * zoom, y * zoom, z * zoom);
    glm::vec3 tempColor;

    // Ajustando la lógica para una menor frecuencia y mayor tamaño de los continentes
    if (firstLayer > 0.1) { // Umbral más alto para tierra
        float landNoise = surfaceNoise.GetNoise(x * zoom, y * zoom, z * zoom);
        glm::vec3 baseLandColor = mix(vegetationColor, vegetationColor, landNoise);
        glm::vec3 vegetationLayer = mix(baseLandColor, landColor, landNoise);
        tempColor = mix(baseLandColor, vegetationLayer, landNoise);
    } else {
        // Océano
        float oceanNoise = surfaceNoise.GetNoise(x * zoom, y * zoom, z * zoom);
        tempColor = mix(oceanColor, seaColor, oceanNoise);
    }

    // Nubes y atmósfera
    float cloudNoise = atmosphereNoise.GetNoise(x * zoom, y * zoom, z * zoom);
    if (cloudNoise > 0.3) {
        tempColor += skyColor * (cloudNoise - 0.3f);
    }

    fragmentColor = {tempColor.x, tempColor.y, tempColor.z};
    return fragmentColor;
}


//MOON
FastNoiseLite moonNoise;

Color getMoonNoise(float x, float y, float z) {

    Color fragmentColor;

    glm::vec3 darkColor = {93.0f/255, 100.0f/255, 115.0f/255};
    glm::vec3 lightColor = {208.0f/255, 193.0f/255, 180.0f/255};

    moonNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    moonNoise.SetFrequency(0.2);
    moonNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);

    float zoom = 55.0f;

    float noise;

    noise = moonNoise.GetNoise(x*zoom, y*zoom, z*zoom);

    glm::vec3 moonNoiseColor = mix(lightColor, darkColor, noise);

    fragmentColor = Color(moonNoiseColor.x, moonNoiseColor.y, moonNoiseColor.z);

    return fragmentColor;
}

// PORTAL CON PATRONES DE CÍRCULOS CONCÉNTRICOS
FastNoiseLite portalNoise;

Color getPortalNoise(float x, float y, float z) {
    Color fragmentColor;

    // Colores para el portal
    glm::vec3 greenColor = {0.0f, 1.0f, 0.0f}; // Verde intenso
    glm::vec3 whiteColor = {1.0f, 1.0f, 1.0f}; // Blanco

    // Configurar el ruido para generar un patrón base
    portalNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    portalNoise.SetFrequency(0.1);
    portalNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    portalNoise.SetFractalOctaves(4);
    portalNoise.SetFractalLacunarity(2.0);
    portalNoise.SetFractalGain(0.5);

    float zoom = 50.0f;
    float noise = portalNoise.GetNoise(x * zoom, y * zoom, z * zoom);

    // Crear patrones circulares concéntricos
    float circlePattern = 0.5f * (1.0f + sin(sqrt(x*x + y*y) * 15.0f - noise * 5.0f));

    // Mezcla de colores basada en el patrón circular
    glm::vec3 finalColor = mix(greenColor, whiteColor, circlePattern);

    // Asegurar que los valores de color están en el rango [0, 1]
    finalColor = glm::clamp(finalColor, 0.0f, 1.0f);

    fragmentColor = Color(finalColor.x, finalColor.y, finalColor.z);

    return fragmentColor;
}

// DISCOBALL CON CUADRADOS METÁLICOS
FastNoiseLite shineNoise;

Color getshine(float x, float y, float z) {
    // Colores metálicos
    glm::vec3 silverColor = {0.8f, 0.8f, 0.8f}; // Plata
    glm::vec3 goldColor = {0.83f, 0.69f, 0.22f}; // Oro
    glm::vec3 bronzeColor = {0.8f, 0.5f, 0.2f}; // Bronce

    shineNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    shineNoise.SetFrequency(0.5); // Mayor frecuencia para cuadrados más pequeños
    shineNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Div);

    float zoom = 500.0f;
    float noise = shineNoise.GetNoise(x * zoom, y * zoom, z * zoom);

    // Patrón de cuadrícula
    float gridPattern = fmod(fabs(x * 10.0f) + fabs(y * 10.0f), 2.0f);
    gridPattern = (gridPattern < 1.0f) ? 0.1f : 1.0f;

    glm::vec3 colorMix;
    // Seleccionar color según el patrón y el ruido
    if (gridPattern > 0.5f) {
        colorMix = mix(silverColor, goldColor, noise);
    } else {
        colorMix = mix(goldColor, bronzeColor, noise);
    }

    // Asegurar que los valores de color están en el rango [0, 1]
    glm::vec3 finalColor = glm::clamp(colorMix, 0.0f, 1.0f);

    // Asegúrate de que tu estructura Color puede aceptar glm::vec3 o convierte los valores adecuadamente
    return Color(finalColor.x, finalColor.y, finalColor.z);
}


// URANO-LIKE PLANET
FastNoiseLite uranoNoise;

Color getUranoLikePlanetNoise(float x, float y, float z) {
    Color fragmentColor;

    // Colores característicos de Urano: azules y turquesas
    glm::vec3 lightBlueColor = {0.68f, 0.85f, 0.9f};  // Azul claro
    glm::vec3 darkBlueColor = {0.05f, 0.2f, 0.4f};    // Azul oscuro

    uranoNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    uranoNoise.SetFrequency(0.1);
    uranoNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    uranoNoise.SetFractalOctaves(6);

    float zoom = 30.0f;

    float noise = uranoNoise.GetNoise(x * zoom, y * zoom, z * zoom);

    // Crear un efecto de bandas como las de Urano
    float bandPattern = abs(sin(y * 5.0f + noise * 0.5f));

    // Mezcla de colores basada en el patrón de bandas
    glm::vec3 colorMix = mix(lightBlueColor, darkBlueColor, bandPattern);

    // Asegurar que los valores de color están en el rango [0, 1]
    glm::vec3 finalColor = glm::clamp(colorMix, 0.0f, 1.0f);

    fragmentColor = Color(finalColor.x, finalColor.y, finalColor.z);

    return fragmentColor;
}
FastNoiseLite spaceshipNoise;

Color getSpaceshipColor(float x, float y, float z) {
    Color fragmentColor;

    // Colores primarios para la nave: Plata y acentos azules
    glm::vec3 primaryColor = {0.75f, 0.75f, 0.75f}; // Plata
    glm::vec3 accentColor = {0.1f, 0.2f, 0.6f};    // Azul oscuro

    spaceshipNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    spaceshipNoise.SetFrequency(0.2);
    spaceshipNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    spaceshipNoise.SetFractalOctaves(4);

    float zoom = 20.0f;

    float noise = spaceshipNoise.GetNoise(x * zoom, y * zoom, z * zoom);

    // Crear un patrón que alterne entre el color primario y los acentos
    float pattern = (sin(x * 10.0f + noise) + sin(y * 10.0f + noise) + sin(z * 10.0f + noise)) / 3.0f;
    pattern = abs(pattern); // Asegurarse de que el patrón sea positivo

    // Mezcla de colores basada en el patrón
    glm::vec3 colorMix = mix(primaryColor, accentColor, pattern);

    // Asegurar que los valores de color estén en el rango [0, 1]
    glm::vec3 finalColor = glm::clamp(colorMix, 0.0f, 1.0f);

    fragmentColor = Color(finalColor.x, finalColor.y, finalColor.z);

    return fragmentColor;
}



Vertex vertexShader(const Vertex& vertex, const Uniforms& uniforms) {
    // Aplicar las transformaciones al vértice utilizando las matrices de uniforms
    glm::vec4 clipSpaceVertex = uniforms.projection * uniforms.view * uniforms.model * glm::vec4(vertex.position, 1.0f);

    // Perspectiva
    glm::vec3 ndcVertex = glm::vec3(clipSpaceVertex) / clipSpaceVertex.w;

    // Aplicar transformación del viewport
    glm::vec4 screenVertex = uniforms.viewport * glm::vec4(ndcVertex, 1.0f);

    // Transformar la normal
    glm::vec3 transformedNormal = glm::mat3(uniforms.model) * vertex.normal;
    transformedNormal = glm::normalize(transformedNormal);

    return Vertex{
            glm::vec3(screenVertex),
            transformedNormal,
            vertex.position
    };
}

Fragment fragmentShader(Fragment& fragment, const std::string name) {

    float x = fragment.originalPos.x;
    float y = fragment.originalPos.y;
    float z = fragment.originalPos.z;

    if (name == "sol") {
        fragment.color = getSunNoise(x, y, z);

    } else if (name == "gaseoso") {
        fragment.color = getGaseous(x, y, z) * fragment.intensity;
    } else if (name == "rocoso") {
        fragment.color = getRocoso(x, y, z) * fragment.intensity;
    } else if (name == "tierra") {
        fragment.color = getWorldNoise(x, y, z) * fragment.intensity;

    } else if (name == "luna") {

        fragment.color = getMoonNoise(x, y, z) * fragment.intensity;
    } else if (name == "portal") {
        fragment.color = getPortalNoise(x, y, z) * fragment.intensity;
    } else if (name == "shine") {
        fragment.color = getshine(x, y, z) * fragment.intensity;
    }
    else if (name == "urano") {
        fragment.color = getUranoLikePlanetNoise(x, y, z) * fragment.intensity;
    }
    else if (name == "nave") {
        fragment.color = getSpaceshipColor(x, y, z) * fragment.intensity;
    }


    return fragment;
}