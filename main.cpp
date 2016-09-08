#include <cstdio>
#include <chrono>
#include <algorithm>
#include "writebitmap.h"
#include "mathtypes.h"
#include <vector>
#include <memory>

const int ImageWidth = 512;
const int ImageHeight = 512;

#define MAX_DEPTH 5

std::vector<std::shared_ptr<SceneObject>> sceneObjects;

vec3 CameraPosition = vec3{ 0.0f, -3.0f, 0.0f };
vec3 CameraLookDirection = vec3{ 0.0f, 0.0f, 1.0f };

float FieldOfView = 30.0f;

void InitScene()
{
    Material mat;
    mat.albedo = vec3(.7f, .1f, .1f);
    mat.specular = vec3(.9f, .1f, .1f);
    mat.opacity = 0.6f;

    sceneObjects.push_back(std::make_shared<Sphere>(mat, vec3(-1.0f, -0.0f, -8.5f), 0.8f));

    mat.albedo = vec3(0.7f, 0.0f, 0.7f);
    mat.specular = vec3(0.9f, 0.9f, 0.8f);
    mat.opacity = 0.9f;

    sceneObjects.push_back(std::make_shared<Sphere>(mat, vec3(1.0f, -0.0f, -8.5f), 0.8f));

    mat.albedo = vec3(0.0f, 0.8f, 0.0f);
    mat.specular = vec3(0.1f, 0.9f, 0.0f);
    mat.opacity = 1.0f;

    //sceneObjects.push_back(std::make_shared<Sphere>(mat, vec3(0.0f, -1.0f, -6.0f), 0.4f));

    sceneObjects.push_back(std::make_shared<TiledPlane>(vec3(0.0f, -0.0f, -9.9f), normalize(vec3(0.0f, 1.0f, 1.0f))));
}

SceneObject* FindNearestObject(vec3 rayorig, vec3 raydir, float& nearestDistance)
{
    SceneObject* nearestObject = nullptr;
    nearestDistance = std::numeric_limits<float>::max();

    // find intersection of this ray with the sphere in the scene
    for (auto pObject : sceneObjects)
    {
        float distance;
        if (pObject->GetSceneObjectType() == SceneObjectType::Sphere)
        {
            const Sphere* pSphere = static_cast<Sphere*>(pObject.get());
            if (glm::intersectRaySphere(rayorig, glm::normalize(raydir), pSphere->center, pSphere->radius * pSphere->radius, distance))
            {
                if (distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearestObject = pObject.get();
                }
            }
        }
        else if (pObject->GetSceneObjectType() == SceneObjectType::Plane)
        {
            const Plane* pPlane = static_cast<Plane*>(pObject.get());
            if (glm::intersectRayPlane(rayorig, glm::normalize(raydir), pPlane->origin, pPlane->normal, distance))
            {
                if (distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearestObject = pObject.get();
                }
            }
        }
    }

    return nearestObject;
}

vec3 TraceRay(const vec3& rayorig, const vec3 &raydir, const int &depth)
{
    const SceneObject* nearestObject = nullptr;
    float distance;
    nearestObject = FindNearestObject(rayorig, raydir, distance);

    if (!nearestObject)
    {
        return vec3{ 0.0f, 0.0f, 0.0f };
    }

    vec3 pos = rayorig + (raydir * distance);
    vec3 normal = nearestObject->GetSurfaceNormal(pos);
    vec3 reflect = glm::normalize(glm::reflect(raydir, normal));

    vec3 outputColor{ 0.0f, 0.0f, 0.0f };

    const Material& material = nearestObject->GetMaterial(pos);

    // If the object is transparent, get the reflection color
    if (depth < MAX_DEPTH && material.opacity < 1.0f)
    {
        outputColor = TraceRay(pos + reflect, reflect, depth + 1) * (1.0f - material.opacity);
    }

    float diffuseI = 0.0f;
    float specI = 0.0f;

    // Otherwise, sample the lighting
    // A fixed, directional light
    vec3 lightDir = vec3{ 1.f, 0.0f, 2.0f };
    lightDir = glm::normalize(lightDir);

    float occluderDistance;
    if (!FindNearestObject(pos + (lightDir * 0.001f), lightDir, occluderDistance))
    {
        vec3 diffuseColor{ 0.0f, 0.0f, 0.0f };

        diffuseI = dot(normal, lightDir);

        if (diffuseI > 0.0f)
        {
            specI = dot(reflect, lightDir);
            if (specI > 0.0f)
            {
                specI = pow(specI, 10);
                specI = std::max(0.0f, specI);
            }
            else
            {
                specI = 0.0f;
            }
        }
        else
        {
            diffuseI = 0.0f;
        }
    }
    outputColor = outputColor + ((material.albedo * diffuseI) + (material.specular * specI)) * material.opacity;
    return outputColor;
}

void DrawScene(Bitmap* pBitmap)
{
    float invWidth = 1.0f / float(ImageWidth);
    float invHeight = 1.0f / float(ImageHeight);

    float aspectRatio = float(ImageWidth) / float(ImageHeight);

    float halfAngle = tan(glm::radians(FieldOfView) * 0.5f);

    for (int y = 0; y < ImageHeight; y++)
    {
        for (int x = 0; x < ImageWidth; x++)
        {
            float xx = (2.0f * (((float)x + 0.5f) * invWidth) - 1.f) * halfAngle * aspectRatio;
            float yy = (1.0f - 2.0f * (((float)y + 0.5f) * invHeight)) * halfAngle;

            vec3 rayDir{ xx, yy, -1.0f };
            rayDir = glm::normalize(rayDir);

            vec3 color = TraceRay(vec3{ 0.0f, 0.0f, 0.0f }, rayDir, 0);

            // Color might have maxed out, so clamp.
            color = color * 255.0f;
            color = clamp(color, vec3(0.0f, 0.0f, 0.0f), vec3(255.0f, 255.0f, 255.0f));

            PutPixel(pBitmap, x, y, Color{ uint8_t(color.x), uint8_t(color.y),uint8_t(color.z) });
        }
    }
}

void main(void* arg, void** args)
{
    Bitmap* pBitmap = CreateBitmap(ImageWidth, ImageHeight);

    Color col{ 127, 127, 127 };
    ClearBitmap(pBitmap, col);

    InitScene();

    DrawScene(pBitmap);

    WriteBitmap(pBitmap, "image.bmp");

    DestroyBitmap(pBitmap);

    system("start image.bmp");
}
