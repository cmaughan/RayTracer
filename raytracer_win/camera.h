#pragma once

// A simple camera
class Camera
{
private:
    vec3 position = vec3(0.0f);                          // Position of the camera in world space
    vec3 focalPoint = vec3(0.0f);                        // Look at point

    float filmWidth = 1.0f;                              // Width/height of the film
    float filmHeight = 1.0f;

    vec3 viewDirection = vec3(0.0f, 0.0f, 1.0f);         // The direction the camera is looking in
    vec3 right = vec3(1.0f, 0.0f, 0.0f);                 // The vector to the right
    vec3 up = vec3(0.0f, 1.0f, 0.0f);                    // The vector up

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

    const vec3& GetPosition() const
    {
        return position;
    }

    void SetPosition(const vec3& pos)
    {
        position = pos;
    }

    void SetFocalPoint(const vec3& pos)
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
    vec3 GetWorldRay(const vec2& imageSample)
    {
        // Could move some of this maths out of here for speed, but this isn't time critical
        vec3 dir(viewDirection);
        float x = ((imageSample.x * 2.0f) / filmWidth) - 1.0f;
        float y = ((imageSample.y * 2.0f) / filmHeight) - 1.0f;

        // Take the view direction and adjust it to point at the given sample, based on the 
        // the frustum 
        dir += (right * (halfAngle * aspectRatio * x));
        dir -= (up * (halfAngle * y));
        dir = normalize(dir);

        return dir;
    }

    void Orbit(float angle)
    {
        UpdateRays();

        //xAngle = glm::acos(viewDirection, glm::vec3(1.0f, 0.0f, 0.0f));
        //glm::vec3 pos;
        //pos.x = glm::sin(glm::radians(cameraAngle)) * cameraDistance;
        //pos.z = glm::cos(glm::radians(cameraAngle)) * cameraDistance;
        //pos.y = 5.0f;
        //pos = lookAt + pos;
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
        right = glm::cross(viewDirection, vec3(0.0f, 1.0, 0.0));

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