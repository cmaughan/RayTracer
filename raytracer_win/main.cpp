#define _CRTDBG_MAP_ALLOC
#define NOMINMAX

#include <windows.h>
#include <windowsx.h>
#include <algorithm>

using namespace std;
#include <gdiplus.h>
#include <memory>
#include <cassert>

#include "common.h"
#pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;

#include "sceneobjects.h"
#include "camera.h"
#include "manipulator.h"

#include <thread>
#include <chrono>

#include "cmdparser\cmdparser.hpp"

#include "glm/glm/gtc/random.hpp"

int ImageWidth = 1024;
int ImageHeight = 768;
const float FieldOfView = 60.0f;
HWND hWnd;

#define MAX_DEPTH 5

std::shared_ptr<Bitmap> spBitmap;
std::vector<glm::vec4> buffer;
std::vector<std::shared_ptr<SceneObject>> sceneObjects;
std::shared_ptr<Camera> pCamera;
std::shared_ptr<Manipulator> pManipulator;

float cameraAngle = 0.0f;
float cameraDistance = 8.0f;

void DrawScene(int partitions, bool antialias);
void CopyTargetToBitmap()
{
    if (spBitmap)
    {
        auto pData = &buffer[0];

        BitmapData writeData;
        Rect lockRect(0, 0, ImageWidth, ImageHeight);
        spBitmap->LockBits(&lockRect, ImageLockModeWrite, PixelFormat32bppARGB, &writeData);

        for (int y = 0; y < int(writeData.Height); y++)
        {
            for (auto x = 0; x < int(writeData.Width); x++)
            {
                glm::u8vec4* pTarget = (glm::u8vec4*)((uint8_t*)writeData.Scan0 + (y * writeData.Stride) + (x * 4));
                glm::vec4 source = buffer[(y * ImageWidth) + x];
                source = glm::clamp(source, glm::vec4(0.0f), glm::vec4(1.0f));

                source = glm::u8vec4(source * 255.0f);
                *pTarget = source;
            }
        }

        spBitmap->UnlockBits(&writeData);
    }
}

bool pause = false;
bool step = true;
bool sizeChanged = true;
int currentSample = 0;

VOID OnPaint(HDC hdc)
{
    Graphics graphics(hdc);

    graphics.SetSmoothingMode(SmoothingMode::SmoothingModeDefault);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

    RectF dest;
    dest.X = 0;
    dest.Y = 0;
    dest.Width = float(ImageWidth);
    dest.Height = float(ImageHeight);
    if (spBitmap)
    {
        graphics.DrawImage(spBitmap.get(), dest, 0.0f, 0.0f, float(ImageWidth), float(ImageHeight), Unit(UnitPixel));
    }
    else
    {
        graphics.Clear(Color::Beige);
    }
}

void InitMaps()
{
    spBitmap = std::make_shared<Bitmap>(ImageWidth, ImageHeight, PixelFormat32bppPARGB);
    buffer.resize(ImageWidth * ImageHeight, glm::vec4(0));
    currentSample = 0;
}


void InitScene()
{
    sceneObjects.clear();

    // Red ball
    Material mat;
    mat.albedo = glm::vec3(.7f, .1f, .1f);
    mat.specular = glm::vec3(.9f, .1f, .1f);
    mat.reflectance = 0.5f;
    sceneObjects.push_back(std::make_shared<Sphere>(mat, glm::vec3(0.0f, 2.0f, 0.f), 2.0f));

    // Purple ball
    mat.albedo = glm::vec3(0.7f, 0.0f, 0.7f);
    mat.specular = glm::vec3(0.9f, 0.9f, 0.8f);
    mat.reflectance = 0.5f;
    sceneObjects.push_back(std::make_shared<Sphere>(mat, glm::vec3(-2.5f, 1.0f, 2.f), 1.0f));

    // Blue ball
    mat.albedo = glm::vec3(0.0f, 0.3f, 1.0f);
    mat.specular = glm::vec3(0.0f, 0.0f, 1.0f);
    mat.reflectance = 0.0f;
    mat.emissive = glm::vec3(0.0f, 0.0f, 0.0f);
    sceneObjects.push_back(std::make_shared<Sphere>(mat, glm::vec3(0.0f, 0.5f, 3.f), 0.5f));

    // White ball
    mat.albedo = glm::vec3(1.0f, 1.0f, 1.0f);
    mat.specular = glm::vec3(0.0f, 0.0f, 0.0f);
    mat.reflectance = .0f;
    mat.emissive = glm::vec3(0.0f, 0.8f, 0.8f);
    sceneObjects.push_back(std::make_shared<Sphere>(mat, glm::vec3(2.8f, 0.8f, 2.0f), 0.8f));

    // White light
    mat.albedo = glm::vec3(0.0f, 0.8f, 0.0f);
    mat.specular = glm::vec3(0.0f, 0.0f, 0.0f);
    mat.reflectance = 0.0f;
    mat.emissive = glm::vec3(1.0f, 1.0f, 1.0f);
    sceneObjects.push_back(std::make_shared<Sphere>(mat, glm::vec3(-10.8f, 6.4f, 10.0f), 0.4f));

    sceneObjects.push_back(std::make_shared<TiledPlane>(glm::vec3(0.0f, 0.0f, 0.0f), normalize(glm::vec3(0.0f, 1.0f, 0.0f))));

    pCamera = std::make_shared<Camera>();
    pCamera->SetPositionAndFocalPoint(glm::vec3(0.0f, 5.0f, cameraDistance), glm::vec3(0.0f));

    pManipulator = std::make_shared<Manipulator>(pCamera);
}

SceneObject* FindNearestObject(glm::vec3 rayorig, glm::vec3 raydir, float& nearestDistance)
{
    SceneObject* nearestObject = nullptr;
    nearestDistance = std::numeric_limits<float>::max();

    // find intersection of this ray with the sphere in the scene
    for (auto pObject : sceneObjects)
    {
        float distance;
        if (pObject->Intersects(rayorig, raydir, distance) &&
            nearestDistance > distance)
        {
            nearestObject = pObject.get();
            nearestDistance = distance;
        }
    }
    return nearestObject;
}

glm::vec3 TraceRay(const glm::vec3& rayorig, const glm::vec3 &raydir, const int depth)
{
    const SceneObject* nearestObject = nullptr;
    float distance;
    nearestObject = FindNearestObject(rayorig, raydir, distance);

    if (!nearestObject)
    {
        return glm::vec3{ 0.1f, 0.1f, 0.1f };
    }
    glm::vec3 pos = rayorig + (raydir * distance);
    glm::vec3 normal = nearestObject->GetSurfaceNormal(pos);
    glm::vec3 outputColor{ 0.0f, 0.0f, 0.0f };

    const Material& material = nearestObject->GetMaterial(pos);

    glm::vec3 reflect = glm::normalize(glm::reflect(raydir, normal));

    // If the object is transparent, get the reflection color
    if (depth < MAX_DEPTH && (material.reflectance > 0.0f))
    {
        glm::vec3 reflectColor(0.0f, 0.0f, 0.0f);
        glm::vec3 refractColor(0.0f, 0.0f, 0.0f);

        reflectColor = TraceRay(pos + (reflect * 0.001f), reflect, depth + 1);
        outputColor = (reflectColor * material.reflectance);
    }
    // For every emitter, gather the light
    for (auto& emitterObj : sceneObjects)
    {
        glm::vec3 emitterDir = emitterObj->GetRayFrom(pos);

        float bestDistance = std::numeric_limits<float>::max();
        SceneObject* pOccluder = nullptr;
        const Material* pEmissiveMat = nullptr;
        for (auto& occluder : sceneObjects)
        {
            if (occluder->Intersects(pos + (emitterDir * 0.001f), emitterDir, distance))
            {
                if (occluder == emitterObj)
                {
                    if (bestDistance > distance)
                    {
                        bestDistance = distance;

                        // If we found our emitter, and the point we hit is not emissive, then ignore
                        pEmissiveMat = &occluder->GetMaterial(pos + (emitterDir * distance));
                        if (pEmissiveMat->emissive == glm::vec3(0.0f, 0.0f, 0.0f))
                        {
                            pEmissiveMat = nullptr;
                        }
                        else
                        {
                            pOccluder = nullptr;
                        }
                    }
                }
                else
                {
                    if (bestDistance > distance)
                    {
                        pOccluder = occluder.get();
                        pEmissiveMat = nullptr;
                        bestDistance = distance;
                    }
                }
            }
        }

        // No emissive material, or occluder
        if (!pEmissiveMat || pOccluder)
        {
            continue;
        }

        float diffuseI = 0.0f;
        float specI = 0.0f;

        diffuseI = dot(normal, emitterDir);// / (bestDistance * .1f);

        if (diffuseI > 0.0f)
        {
            specI = dot(reflect, emitterDir);
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
        outputColor += (pEmissiveMat->emissive * material.albedo * diffuseI) + (material.specular * specI);
    }
    outputColor *= 1.f - material.reflectance;
    outputColor += material.emissive;
    return outputColor;
}

void DrawScene(int partitions, bool antialias)
{
    if (!spBitmap)
    {
        return;
    }

    pCamera->PreRender();

    srand(currentSample);

    std::vector<std::shared_ptr<std::thread>> threads;

    const float k1 = float(currentSample);
    const float k2 = 1.f / (k1 + 1.f);
    glm::vec2 sample = glm::linearRand(glm::vec2(0.0f), glm::vec2(1.0f));
    //glm::vec2 sample = glm::gaussRand(glm::vec2(0.5f), glm::vec2(0.5f));
    for (int i = 0; i < partitions; i++)
    {
        auto pT = std::make_shared<std::thread>([&](int offset)
        {
            for (int y = offset; y < ImageHeight; y += partitions)
            {
                for (int x = 0; x < ImageWidth; x += 1)
                {
                    glm::vec3 color{ 0.0f, 0.0f, 0.0f };
                    auto offset = sample + glm::vec2(x, y);

                    auto ray = pCamera->GetWorldRay(offset);
                    color += TraceRay(pCamera->GetPosition(), ray, 0);

                    auto index = (y * ImageWidth) + x;
                    auto& bufferVal = buffer[index];

                    bufferVal = ((bufferVal * k1) + glm::vec4(color, 1.0f)) * k2;
                }
            }
        }, i);
        threads.push_back(pT);
    }

    for (auto& t : threads)
    {
        t->join();
    }
    currentSample++;
    CopyTargetToBitmap();
    InvalidateRect(hWnd, NULL, TRUE);
}

void OnSizeChanged()
{
    RECT rc;
    GetClientRect(hWnd, &rc);

    int x = rc.right - rc.left;
    int y = rc.bottom - rc.top;

    ImageWidth = x;
    ImageHeight = y;

    InitMaps();

    pCamera->SetFilmSize(float(ImageWidth), float(ImageHeight));

    currentSample = 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    HDC          hdc;
    PAINTSTRUCT  ps;

    switch (message)
    {
    case WM_CHAR:
    {
        if (wParam == 'o')
        {
            currentSample = 0;
        }
        else if (wParam == 'p')
        {
            pause = !pause;
        }
        else if (wParam == ' ')
        {
            step = true;
        }
        /*
        else if (wParam == 'r')
        {
            //pCamera->Orbit(1.0f);
            currentSample = 0;
            step = true;
        }
        else if (wParam == 'f')
        {
            pCamera->Orbit(-1.0f);
            currentSample = 0;
            step = true;
        }
        */
        else if (wParam == 'w')
        {
            pCamera->Dolly(.5f);
            currentSample = 0;
            step = true;
        }
        else if (wParam == 's')
        {
            pCamera->Dolly(-.5f);
            currentSample = 0;
            step = true;
        }
    }
    break;

    case WM_SYSCOMMAND:
    {
        switch (LOWORD(wParam))
        {
        case SC_MAXIMIZE:
            spBitmap.reset();
            break;
        default:
            break;
        }
    }
    break;

    case WM_MOUSEMOVE:
    {
        auto xPos = GET_X_LPARAM(lParam); 
        auto yPos = GET_Y_LPARAM(lParam); 
        if (pManipulator->MouseMove(glm::vec2(xPos, yPos)))
        {
            currentSample = 0;
            step = true;
        }
    }
    break;

    case WM_LBUTTONDOWN:
    {
        auto xPos = GET_X_LPARAM(lParam); 
        auto yPos = GET_Y_LPARAM(lParam); 
        pManipulator->MouseDown(glm::vec2(xPos, yPos));
        SetCapture(hWnd);
    }
    break;

    case WM_LBUTTONUP:
    {
        auto xPos = GET_X_LPARAM(lParam); 
        auto yPos = GET_Y_LPARAM(lParam); 
        pManipulator->MouseUp(glm::vec2(xPos, yPos));
        ReleaseCapture();
    }
    break;

    case WM_SIZE:
    {
        spBitmap.reset();
    }
    break;

    case WM_ERASEBKGND:
        return TRUE;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        OnPaint(hdc);
        EndPaint(hWnd, &ps);
        return 0;

    case WM_DESTROY:
        spBitmap.reset();
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
} // WndProc

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
    MSG                 msg;
    WNDCLASS            wndClass;
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;

    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = TEXT("RayTracer");

    RegisterClass(&wndClass);

    // Make a window in the center of the screen, with an square client rect
    const int WindowSize = 500;
    int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    hWnd = CreateWindow(
        TEXT("RayTracer"),   // window class name
        TEXT("RayTracer"),  // window caption
        WS_OVERLAPPEDWINDOW,      // window style
        (nScreenWidth / 2) - (WindowSize / 2),            // initial x position
        (nScreenHeight / 2) - (WindowSize / 2),            // initial y position
        WindowSize,            // initial x size
        WindowSize + GetSystemMetrics(SM_CYCAPTION),            // initial y size
        NULL,                     // parent window handle
        NULL,                     // window menu handle
        hInstance,                // program instance handle
        NULL);                    // creation parameters

    ShowWindow(hWnd, iCmdShow);
    UpdateWindow(hWnd);

#ifdef _DEBUG
    pause = true;
    step = true;
#endif
    InitScene();

    cli::Parser parser(__argc, __argv);
    parser.set_optional<int>("p", "partitions", 2, "thread partitions 2 == 4, 3 == 9");
    parser.set_optional<int>("a", "antialiased", 0, "Antialias each pixel");
    parser.run();

    auto partitions = parser.get<int>("p");
    auto antialias = parser.get<int>("a") == 0 ? false : true;

    Color col{ 127, 127, 127 };

    msg.message = 0;
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
        {
            //Translate message
            TranslateMessage(&msg);

            //Dispatch message
            DispatchMessage(&msg);
        }
        else
        {
            if (spBitmap == nullptr)
            {
                OnSizeChanged();
                DrawScene(partitions, antialias);
            }
            else
            {
                if (!pause || step)
                {
                    DrawScene(partitions, antialias);
                }
            }
            SetWindowTextA(hWnd, std::to_string(currentSample).c_str());
            step = false;
        }
    }

    GdiplusShutdown(gdiplusToken);
    return 0;
}  // WinMain

/*void main(int argc, char** args)
{

    InitScene();
    auto start = std::chrono::high_resolution_clock::now();


    auto end = std::chrono::high_resolution_clock::now();
    auto diff = end - start;

    std::cout << "Time: " << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;

}*/
