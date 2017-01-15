#pragma once

#include "camera.h"

class Manipulator
{
private:
    glm::vec2 startPos;
    glm::vec2 currentPos;
    std::shared_ptr<Camera> spCamera;
    bool mouseDown;

public:
    Manipulator(std::shared_ptr<Camera> pCam)
        : spCamera(pCam),
        mouseDown(false)
    {

    }

    void MouseDown(const glm::vec2& pos)
    {
        startPos = pos;
        mouseDown = true;
    }

    void MouseUp(const glm::vec2& pos)
    {
        currentPos = pos;
        mouseDown = false;
    }

    bool MouseMove(const glm::vec2& pos)
    {
        currentPos = pos;
        if (mouseDown)
        {
            spCamera->Orbit(currentPos - startPos);
            startPos = pos;
        }
        return true;
    }

};