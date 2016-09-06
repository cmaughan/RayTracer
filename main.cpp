#include <cstdio>
#include <chrono>

#include "writebitmap.h"
#include "mathtypes.h"
#include <vector>

const int ImageWidth = 512;
const int ImageHeight = 512;

std::vector<Sphere> spheres;

Vector3 CameraPosition = Vector3{ 0.0f, -3.0f, 0.0f };
Vector3 CameraLookDirection = Vector3{ 0.0f, 0.0f, 1.0f };
float FieldOfView = 30.0f;

void InitScene()
{
	spheres.push_back(Sphere{ Vector3{ -1.0f, 0.0f, -5.f }, 0.7f, Color3{1.0f, 0.0f, 0.0f} });
	spheres.push_back(Sphere{ Vector3{ 1.0f, 0.0f, -5.f }, 0.5f, Color3{1.0f, 0.0f, 1.0f}  });
	spheres.push_back(Sphere{ Vector3{ 0.0f, 0.0f, -7.f }, 0.5f, Color3{0.0f, 1.0f, 0.0f} });
}

Color3 TraceRay(const Vector3& rayorig, const Vector3 &raydir, const int &depth)
{
	float tnear = INFINITY;
	const Sphere* sphere = NULL;
	 
	// find intersection of this ray with the sphere in the scene
	for (unsigned i = 0; i < spheres.size(); ++i)
	{
		float t0 = INFINITY, t1 = INFINITY;
		if (spheres[i].Intersect(rayorig, raydir, t0, t1)) 
		{
			if (t0 < 0)
			{
				t0 = t1;
			}

			if (t0 < tnear) 
			{
				tnear = t0;
				sphere = &spheres[i];
			}
		}
	}

	if (!sphere)
	{
		return Color3{ 0.0f, 0.0f, 0.0f };
	}

	// Where we hit
	Vector3 phit = rayorig + raydir * tnear; 

	// Normal
    Vector3 normal = phit - sphere->center; 
    normal = -normal.Normalize(); 

	if (sphere)
	{
		return sphere->color * normal.Dot(raydir);
	}

	return Color3{ 0.0f, 0.0f, 0.0f };
}

void DrawScene(Bitmap* pBitmap)
{
	float invWidth = 1.0f / float(ImageWidth);
	float invHeight = 1.0f / float(ImageHeight);

	float aspectRatio = float(ImageWidth) / float(ImageHeight);

	float halfAngle = tan(DegreesToRadians(FieldOfView) * 0.5f);

	Color col{ 255, 0, 255 };
	for (int y = 0; y < ImageHeight; y++)
	{
		for (int x = 0; x < ImageWidth; x++)
		{
			float xx = (2.0f * (((float)x + 0.5f) * invWidth) - 1.f) * halfAngle * aspectRatio;
			float yy = (1.0f - 2.0f * (((float)y + 0.5f) * invHeight)) * halfAngle;

			Vector3 rayDir{ xx, yy, -1.0f };
			rayDir = rayDir.Normalize();

			Color3 color = TraceRay(Vector3{ 0.0f, 0.0f, 0.0f }, rayDir, 0);
			color = color * 255.0f;
			color = color.Clamp(255.0f);
			
			PutPixel(pBitmap, x, y, Color{ uint8_t(color.x), uint8_t(color.y ),uint8_t(color.z) });
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
