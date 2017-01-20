#pragma once

#include "glm/glm/gtx/intersect.hpp"
struct Material
{
    glm::vec3 albedo;        // Base color of the surface
    glm::vec3 specular;      // Specular reflection color
    float reflectance;  // How reflective the surface is
    glm::vec3 emissive;      // Light that the material emits
};

enum class SceneObjectType
{
    Sphere,
    Plane
};

struct SceneObject
{
    // Given a point on the surface, return the material at that point
    virtual const Material& GetMaterial(const glm::vec3& pos) const = 0;

    // Is it a sphere or a plane?
    virtual SceneObjectType GetSceneObjectType() const = 0;

    // Given a point on the surface, return a normal
    virtual glm::vec3 GetSurfaceNormal(const glm::vec3& pos) const = 0;

    // Given a source position, return a ray to this object's center
    virtual glm::vec3 GetRayFrom(const glm::vec3& from) const = 0;

    // Intersect this object with a ray and figure out if it hits, and return the distance to the hit point 
    virtual bool Intersects(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float& distance) const = 0;
};

// A sphere, at a coordinate, with a radius and a material
struct Sphere : SceneObject
{
    glm::vec3 center;
    float radius;
    Material material;

    Sphere(const Material& mat, const glm::vec3& c, const float r)
    {
        material = mat;
        center = c;
        radius = r;
    }

    virtual const Material& GetMaterial(const glm::vec3& pos) const override
    {
        return material;
    }

    virtual SceneObjectType GetSceneObjectType() const override
    {
        return SceneObjectType::Sphere;
    }

    virtual glm::vec3 GetSurfaceNormal(const glm::vec3& pos) const
    {
        return normalize(pos - center);
    }
    
    virtual glm::vec3 GetRayFrom(const glm::vec3& from) const override
    {
        return normalize(center - from);
    }

    virtual bool Intersects(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float& distance) const
    {
        bool hit = glm::intersectRaySphere(rayOrigin, glm::normalize(rayDir), center, radius * radius, distance);
        return hit;
    }
};

// A plane, centered at origin, with a normal direction
struct Plane : SceneObject
{
    glm::vec3 normal;
    glm::vec3 origin;
    virtual SceneObjectType GetSceneObjectType() const override 
    {
        return SceneObjectType::Plane;
    }
};

// A tiled plane.  returns a different material based on the hit point to represent the grid
struct TiledPlane : Plane
{
    Material blackMat;
    Material whiteMat;

    TiledPlane(const glm::vec3& o, const glm::vec3& n)
    {
        normal = n;
        origin = o;
        blackMat.reflectance = 0.2f;
        blackMat.specular = glm::vec3(1.0f, 1.0f, 1.0f);
        blackMat.albedo = glm::vec3(0.0f, 0.0f, 0.0f);

        whiteMat.reflectance = 0.3f;
        whiteMat.specular = glm::vec3(1.0f, 1.0f, 1.0f);
        whiteMat.albedo = glm::vec3(1.0f, 1.0f, 1.0f);
    }

    virtual const Material& GetMaterial(const glm::vec3& pos) const override
    {
        bool white = ((int(floor(pos.x / 4) + /*floor(pos.y) +*/ floor(pos.z / 4)) & 1) == 0); 
        if (white)
        {
            return whiteMat;
        }
        return blackMat;
    }
    
    virtual glm::vec3 GetSurfaceNormal(const glm::vec3& pos) const
    {
        return normal;
    }
    
    virtual glm::vec3 GetRayFrom(const glm::vec3& from) const override
    {
        return normalize(origin - from);
    }
    
    virtual bool Intersects(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float& distance) const override
    {
        return glm::intersectRayPlane(rayOrigin, rayDir, origin, normal, distance);
    }
};