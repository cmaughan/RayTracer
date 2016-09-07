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
    //spheres.push_back(Sphere{ vec3{ 1.0f, .5f, -2.5f }, 0.5f, vec3{0.5f, 0.5f, 0.0f}, .5f });
}

vec3 TraceRay(const vec3& rayorig, const vec3 &raydir, const int &depth, bool& hit)
{
    const Sphere* sphere = NULL;
    float lastDistanceSquared = INFINITY;
    vec3 pos, normal;

    // find intersection of this ray with the sphere in the scene
    for (unsigned i = 0; i < spheres.size(); ++i)
    {
        vec3 intersectPos;
        vec3 intersectNormal;
        if (intersectRaySphere(rayorig, normalize(raydir), spheres[i].center, spheres[i].radius, intersectPos, intersectNormal))
        {
            auto d = fabs(distance(intersectPos, rayorig));
            if (d < lastDistanceSquared)
            {
                lastDistanceSquared = d;
                sphere = &spheres[i];
                pos = intersectPos;
                normal = normalize(intersectNormal);
            }
        }
    }

    if (!sphere)
    {
        hit = false;
        return vec3{ 0.0f, 0.0f, 0.0f };
    }

    hit = true;
    vec3 reflect = normalize(glm::reflect(raydir, normal));
    vec3 result{ 0.0f, 0.0f, 0.0f };

    float tiny = std::numeric_limits<float>::min();

    vec3 outputColor{ 0.0f, 0.0f, 0.0f };

    // If the object is transparent, return a reflection color
    if (depth < MAX_DEPTH && sphere->reflection > 0.0f)
    {
        outputColor = TraceRay(pos, reflect, depth + 1, hit) * sphere->reflection;
    }

    // Otherwise, sample the lighting
    vec3 lightDir = vec3{ 1.f, .0f, 2.0f };
    lightDir = normalize(lightDir);

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
    outputColor = outputColor + ( ((sphere->color * diffuseI) + (sphere->specularColor * specI)) /* (1.0f - sphere->transparency)*/) ;
    return outputColor;
}

void DrawScene(Bitmap* pBitmap)
{
    float invWidth = 1.0f / float(ImageWidth);
    float invHeight = 1.0f / float(ImageHeight);

    float aspectRatio = float(ImageWidth) / float(ImageHeight);

    float halfAngle = tan(glm::radians(FieldOfView) * 0.5f);

    Color col{ 255, 0, 255 };
    for (int y = 0; y < ImageHeight; y++)
    {
        for (int x = 0; x < ImageWidth; x++)
        {
            float xx = (2.0f * (((float)x + 0.5f) * invWidth) - 1.f) * halfAngle * aspectRatio;
            float yy = (1.0f - 2.0f * (((float)y + 0.5f) * invHeight)) * halfAngle;

            vec3 rayDir{ xx, yy, -1.0f };
            rayDir = normalize(rayDir);

            bool hit = false;
            vec3 color = TraceRay(vec3{ 0.0f, 0.0f, 0.0f }, rayDir, 0, hit);
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
