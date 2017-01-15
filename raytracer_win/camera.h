#pragma once

#include "glm/glm/gtx/rotate_vector.hpp"

// A simple camera
class Camera
{
private:
    glm::vec3 position = glm::vec3(0.0f);                          // Position of the camera in world space
    glm::vec3 focalPoint = glm::vec3(0.0f);                        // Look at point

    float filmWidth = 1.0f;                              // Width/height of the film
    float filmHeight = 1.0f;

    glm::vec3 viewDirection = glm::vec3(0.0f, 0.0f, 1.0f);         // The direction the camera is looking in
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);                 // The vector to the right
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);                    // The vector up

    float fieldOfView = 60.0f;                          // Field of view
    float halfAngle = 30.0f;                            // Half angle of the view frustum
    float aspectRatio = 1.0f;                           // Ratio of x to y of the viewport

public:
    Camera()
    {
    }

    virtual ~Camera()
    {

    }

    const glm::vec3& GetPosition() const
    {
        return position;
    }

    void SetPosition(const glm::vec3& pos)
    {
        position = pos;
    }

    void SetFocalPoint(const glm::vec3& pos)
    {
        focalPoint = pos;
    }

    void SetFilmSize(float width, float height)
    {
        filmWidth = width;
        filmHeight = height;
        aspectRatio = width / height;
    }

    void PreRender()
    {
        UpdateRays();

        // The half-width of the viewport, in world space
        halfAngle = float(tan(glm::radians(fieldOfView) / 2.0));
    }

    // Given a screen coordinate, return a ray leaving the camera and entering the world at that 'pixel'
    glm::vec3 GetWorldRay(const glm::vec2& imageSample)
    {
        // Could move some of this maths out of here for speed, but this isn't time critical
        glm::vec3 dir(viewDirection);
        float x = ((imageSample.x * 2.0f) / filmWidth) - 1.0f;
        float y = ((imageSample.y * 2.0f) / filmHeight) - 1.0f;

        // Take the view direction and adjust it to point at the given sample, based on the 
        // the frustum 
        dir += (right * (halfAngle * aspectRatio * x));
        dir -= (up * (halfAngle * y));
        dir = normalize(dir);

        return dir;
    }

    void Orbit(const glm::vec2& angle)
    {
        UpdateRays();
        glm::vec3 yUp(0.0f, 1.0f, 0.0f);

        glm::vec3 xRotate = glm::rotate(-viewDirection, glm::radians(-angle.x), yUp);
        glm::vec3 yRotate = glm::rotate(xRotate, glm::radians(-angle.y), right);

        position = (yRotate * glm::length(focalPoint - position)) + focalPoint;
        //position += (yRotate * glm::length(focalPoint - position)) + focalPoint;

        //viewDirection = -yRotate;
        UpdateRays();
    }
    
    void Dolly(float distance)
    {
        UpdateRays();
        position += viewDirection * distance;
    }

private:
    void UpdateRays()
    {
        // Work out direction
        viewDirection = focalPoint - position;
        viewDirection = glm::normalize(viewDirection);

        // The vector to the right of the camera - assume the 'Up' Vector is vertically up
        right = glm::cross(viewDirection, glm::vec3(0.0f, 1.0, 0.0));

        // The 'up' vector pointing above the camera
        // We recalculate it from the existing right and view direction.
        // Imaging a line pointing up from your head, based on how far forward you are leaning
        up = glm::cross(right, viewDirection);

        // Ensure our vectors are normalized
        up = normalize(up);
        right = normalize(right);
        viewDirection = normalize(viewDirection);
    }
};