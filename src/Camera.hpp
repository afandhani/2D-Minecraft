#pragma once

struct Camera
{
    float x = 0.0f;
    float y = 0.0f;

    float followSpeed = 5.0f;

    float screenWidth = 1280.0f;
    float screenHeight = 720.0f;

    void follow(float targetX, float targetY, float deltaTime)
    {
        float targetXCamera =
            targetX - screenWidth * 0.5f;

        float targetYCamera =
            targetY - screenHeight * 0.5f;

        x +=
            (targetXCamera - x)
            * followSpeed
            * deltaTime;

        y +=
            (targetYCamera - y)
            * followSpeed
            * deltaTime;
    }
};