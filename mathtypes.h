#pragma once
#include <cmath>
#include "glm/glm/glm.hpp"
#include "glm/glm/gtx/intersect.hpp"

using namespace glm;

struct Sphere
{
    vec3 center;
    float radius;
    vec3 color;
    float reflection;
    vec3 specularColor;

};