#include "glm/glm.hpp"
#include <SDL.h>
#include <algorithm>
#include <iostream>
#pragma once

const int SCREEN_WIDTH = 1300;
const int SCREEN_HEIGHT = 800;


struct Planet {
    float rotationAngle;
    float translationAngle;
    float translationRadius;
    float translationSpeed;
    glm::vec3 translationAxis;
    glm::vec3 worldPos;
    glm::vec3 scaleFactor;
    std::string name;
    std::vector<Planet> satelites;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 originalPos;
};

struct Camera {
    glm::vec3 cameraPosition;
    glm::vec3 targetPosition;
    glm::vec3 upVector;
};

struct Uniforms {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 viewport;
};

struct Spaceship {
    glm::vec3 worldPos;
    glm::vec3 scaleFactor;
    float rotationAngle;
    float movementSpeed;
    float rotationSpeed;
};

struct Color {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;

    Color() : r(0), g(0), b(0), a(255) {}

    Color(int red, int green, int blue, int alpha = 255) {
        r = static_cast<Uint8>(std::min(std::max(red, 0), 255));
        g = static_cast<Uint8>(std::min(std::max(green, 0), 255));
        b = static_cast<Uint8>(std::min(std::max(blue, 0), 255));
        a = static_cast<Uint8>(std::min(std::max(alpha, 0), 255));
    }

    Color(float red, float green, float blue, float alpha = 1.0f) {
        r = static_cast<Uint8>(std::min(std::max(red * 255, 0.0f), 255.0f));
        g = static_cast<Uint8>(std::min(std::max(green * 255, 0.0f), 255.0f));
        b = static_cast<Uint8>(std::min(std::max(blue * 255, 0.0f), 255.0f));
        a = static_cast<Uint8>(std::min(std::max(alpha * 255, 0.0f), 255.0f));
    }

    // Overload the + operator to add colors
    Color operator+(const Color& other) const {
        return Color{
                std::min(255, int(r) + int(other.r)),
                std::min(255, int(g) + int(other.g)),
                std::min(255, int(b) + int(other.b)),
                std::min(255, int(a) + int(other.a))
        };
    }

    // Overload the * operator to scale colors by a factor
    Color operator*(float factor) const {
        return Color{
                std::clamp(static_cast<Uint8>(r * factor), Uint8(0), Uint8(255)),
                std::clamp(static_cast<Uint8>(g * factor), Uint8(0), Uint8(255)),
                std::clamp(static_cast<Uint8>(b * factor), Uint8(0), Uint8(255)),
                std::clamp(static_cast<Uint8>(a * factor), Uint8(0), Uint8(255))
        };
    }

    // Friend function to allow float * Color
    friend Color operator*(float factor, const Color& color);
};




struct Fragment {
    glm::ivec2 position; // X and Y coordinates of the pixel (in screen space)
    Color color; // r, g, b values for color
    double z;  // zbuffer
    float intensity;  // light intensity
    glm::vec3 originalPos; // Posición original
};

std::vector<Fragment> line(const glm::vec3& v1, const glm::vec3& v2) {
    glm::ivec2 p1(static_cast<int>(v1.x), static_cast<int>(v1.y));
    glm::ivec2 p2(static_cast<int>(v2.x), static_cast<int>(v2.y));

    std::vector<Fragment> fragments;

    int dx = std::abs(p2.x - p1.x);
    int dy = std::abs(p2.y - p1.y);
    int sx = (p1.x < p2.x) ? 1 : -1;
    int sy = (p1.y < p2.y) ? 1 : -1;

    int err = dx - dy;

    glm::ivec2 current = p1;

    while (true) {
        Fragment fragment;
        fragment.position = current;

        fragments.push_back(fragment);

        if (current == p2) {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            current.x += sx;
        }
        if (e2 < dx) {
            err += dx;
            current.y += sy;
        }
    }

    return fragments;
}


glm::vec3 L = glm::vec3(0.0f, 0.0f, 1.0f);

std::pair<float, float> barycentricCoordinates(const glm::ivec2& P, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    glm::vec3 bary = glm::cross(
            glm::vec3(C.x - A.x, B.x - A.x, A.x - P.x),
            glm::vec3(C.y - A.y, B.y - A.y, A.y - P.y)
    );

    if (abs(bary.z) < 1) {
        return std::make_pair(-1, -1);
    }

    return std::make_pair(
            bary.y / bary.z,
            bary.x / bary.z
    );
}

std::vector<Fragment> triangle(const Vertex &a, const Vertex &b, const Vertex &c, std::string string, glm::vec3 vec) {

    std::vector<Fragment> fragments;
    glm::vec3 A = a.position;
    glm::vec3 B = b.position;
    glm::vec3 C = c.position;

    float minX = std::min(std::min(A.x, B.x), C.x);
    float minY = std::min(std::min(A.y, B.y), C.y);
    float maxX = std::max(std::max(A.x, B.x), C.x);
    float maxY = std::max(std::max(A.y, B.y), C.y);

    // Iterate over each point in the bounding box
    for (int y = static_cast<int>(std::ceil(minY)); y <= static_cast<int>(std::floor(maxY)); ++y) {
        for (int x = static_cast<int>(std::ceil(minX)); x <= static_cast<int>(std::floor(maxX)); ++x) {
            if (x < 0 || y < 0 || y > SCREEN_HEIGHT || x > SCREEN_WIDTH)
                continue;

            glm::ivec2 P(x, y);
            auto barycentric = barycentricCoordinates(P, A, B, C);
            float w = 1 - barycentric.first - barycentric.second;
            float v = barycentric.first;
            float u = barycentric.second;
            float epsilon = 1e-10;

            if (w < epsilon || v < epsilon || u < epsilon)
                continue;

            double z = A.z * w + B.z * v + C.z * u;

            glm::vec3 normal = glm::normalize(
                    a.normal * w + b.normal * v + c.normal * u
            );

            //normal = a.normal; // assume flatness
            float intensity = glm::dot(normal, L);

            if (intensity < 0)
                continue;

            glm::vec3 originalPos = a.originalPos * w + b.originalPos * v + c.originalPos * u;

            fragments.push_back(
                    Fragment{
                            glm::vec3(static_cast<uint16_t>(P.x), static_cast<uint16_t>(P.y), 0),
                            Color(205, 205, 205), //color blanco hardcodeado
                            z,
                            intensity,
                            originalPos}
            );
        }
    }
    return fragments;
}