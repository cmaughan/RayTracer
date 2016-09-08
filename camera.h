#pragma once

struct Camera
{
    vec3 position;
    vec3 viewDirection;
    vec3 right;
    vec3 up;

    float viewPlaneHalfWidth;
    float viewPlaneHalfHeight;
    float aspectRatio;

    vec3 viewPlaneTopLeft;

    vec3 xInc;
    vec3 yInc;

    float halfAngle;

    float imageWidth;
    float imageHeight;
    Camera(const vec3& cameraPosition, const vec3& dir, float fov, int width, unsigned int height)
    {
        // Remember where the camera is in space
        position = cameraPosition;

        viewDirection = dir;

        imageWidth = (float)width;
        imageHeight = (float)height;

        // The vector to the right of the camera - assume the 'Up' Vector is to the right
        right = glm::cross(viewDirection, vec3(0.0f, 1.0f, 0.0f));

        // The 'up' vector pointing above the camera
        up = glm::cross(right, viewDirection);

        // Ensure our vectors are normalized
        up = normalize(up);
        right = normalize(right);
        viewDirection = normalize(viewDirection);

        // Aspect ratio if the window is not square
        aspectRatio = float(width) / float(height);

        // The half-width of the viewport, in world space
        halfAngle = float(tan(glm::radians(fov) / 2.0));
    }

    // Given a screen coordinate, return a ray leaving the camera and entering the world at that 'pixel'
    vec3 GetWorldRay(const vec2& imageSample)
    {
        // Could move some of this maths out of here. 
        vec3 dir(viewDirection);
        float x = ((imageSample.x * 2.0f) / imageWidth) - 1.0f;
        float y = ((imageSample.y * 2.0f) / imageHeight) - 1.0f;

        dir += (right * (halfAngle * aspectRatio * x));
        dir -= (up * (halfAngle * y));
        dir = normalize(dir);

        return dir;
    }
};