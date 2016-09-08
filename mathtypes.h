#pragma once
#include <cmath>
#include "glm/glm/glm.hpp"
#include "glm/glm/gtx/intersect.hpp"

using vec3 = glm::vec3;
using vec4 = glm::vec4;

struct Material
{
    vec3 albedo;        // Base color of the surface
    vec3 specular;      // Specular reflection color
    float opacity;      // How opaque the surface is
};

enum class SceneObjectType
{
    Sphere,
    Plane
};

struct SceneObject
{
    virtual const Material& GetMaterial(const vec3& pos) const = 0;
    virtual SceneObjectType GetSceneObjectType() const = 0;
    virtual vec3 GetSurfaceNormal(const vec3& pos) const = 0;
};

struct Sphere : SceneObject
{
    vec3 center;
    float radius;
    Material material;

    Sphere(const Material& mat, const vec3& c, const float r)
    {
        material = mat;
        center = c;
        radius = r;
    }

    virtual const Material& GetMaterial(const vec3& pos) const override
    {
        return material;
    }

    virtual SceneObjectType GetSceneObjectType() const override
    {
        return SceneObjectType::Sphere;
    }

    virtual vec3 GetSurfaceNormal(const vec3& pos) const
    {
        return normalize(pos - center);
    }
};

struct Plane : SceneObject
{
    vec3 normal;
    vec3 origin;
    virtual SceneObjectType GetSceneObjectType() const override 
    {
        return SceneObjectType::Plane;
    }
};

struct TiledPlane : Plane
{
    mutable Material mat;
    TiledPlane(const vec3& o, const vec3& n)
    {
        normal = n;
        origin = o;
        mat.opacity = 1.0f;
        mat.specular = vec3(1.0f, 1.0f, 1.0f);
    }

    virtual const Material& GetMaterial(const vec3& pos) const override
    {
        bool white = ((int(floor(pos.x / .5f) + floor(pos.y / .5f)) & 1) == 0);

        if (white)
        {
            mat.albedo = vec3(1.0f, 1.0f, 1.0f);
        }
        else
        {
            mat.albedo = vec3(0.0f, 0.0f, 0.0f);
        }
        return mat;
    }
    
    virtual vec3 GetSurfaceNormal(const vec3& pos) const
    {
        return normal;
    }
};