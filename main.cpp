#include <cstdio>
#include <chrono>
#include <algorithm>
#include "writebitmap.h"
#include "mathtypes.h"
#include <vector>

const int ImageWidth = 512;
const int ImageHeight = 512;
 
#define MAX_DEPTH 1

std::vector<Sphere> spheres;

vec3 CameraPosition = vec3{ 0.0f, -3.0f, 0.0f };
vec3 CameraLookDirection = vec3{ 0.0f, 0.0f, 1.0f };

float FieldOfView = 30.0f;

void InitScene()
{
    spheres.push_back(Sphere{ vec3{ -1.0f, 0.0f, -7.5f }, 1.0f, vec3{0.5f, 0.0f, 0.0f},0.3f, vec3{.9f, .1f, .1f} });
    spheres.push_back(Sphere{ vec3{ 1.0f, 0.0f, -9.f }, 1.0f, vec3{0.5f, 0.0f, 0.5f}, 1.f, vec3{.9f, .1f, .8f} });
    spheres.push_back(Sphere{ vec3{ 0.0f, 1.2f, -7.5f }, 0.5f, vec3{0.0f, 0.5f, 0.0f}, .7f, vec3{.1f, .9f, .0f } });
}

vec3 TraceRay(const vec3& rayorig, const vec3 &raydir, const int &depth)
{
    const Sphere* sphere = nullptr;
    float lastDistance = std::numeric_limits<float>::max();

    // find intersection of this ray with the sphere in the scene
    for (unsigned i = 0; i < spheres.size(); ++i)
    {
        float distance;
        if (glm::intersectRaySphere(rayorig, glm::normalize(raydir), spheres[i].center, spheres[i].radius * spheres[i].radius, distance))
        {
            if (distance < lastDistance)
            {
                lastDistance = distance;
                sphere = &spheres[i];
            }
        }
    }

    if (!sphere)
    {
        return vec3{ 0.0f, 0.0f, 0.0f };
    }

    vec3 pos = rayorig + (raydir * lastDistance);
    vec3 normal = glm::normalize(pos - sphere->center);
    vec3 reflect = glm::normalize(glm::reflect(raydir, normal));

    vec3 outputColor{ 0.0f, 0.0f, 0.0f };

    // If the object is transparent, get the reflection color
    if (depth < MAX_DEPTH && sphere->reflection > 0.0f)
    {
        outputColor = TraceRay(pos + reflect, reflect, depth + 1) * sphere->reflection;
    }

    // Otherwise, sample the lighting
    // A fixed, directional light
    vec3 lightDir = vec3{ 1.f, .0f, 2.0f };
    lightDir = glm::normalize(lightDir);

    vec3 specColor{ 1.0f, 1.0f, 1.0f };
    vec3 diffuseColor{ 0.0f, 0.0f, 0.0f };

    float diffuseI = dot(normal, lightDir);
    float specI = 0.0f;
    
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
    outputColor = outputColor + ((sphere->color * diffuseI) + (sphere->specularColor * specI));
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
