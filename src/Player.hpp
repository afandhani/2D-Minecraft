#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <cmath>
#include <iostream>

#include "Transform.hpp"
#include "Camera.hpp"
#include "World.hpp"

#include <algorithm>


struct Anchor
{
    float x = 0.0f;
    float y = 0.0f;
};


struct SpritePart
{
    SDL_Texture* texture = nullptr;

    Transform transform;

    Anchor pivot;

    float animationRotation = 0.0f;
};


struct Player
{
    Transform transform;

    SpritePart body;
    SpritePart head;

    SpritePart armFront;
    SpritePart armBack;

    SpritePart legFront;
    SpritePart legBack;


    // =========================================================
    // PHYSICS
    // =========================================================

    float velocityX = 0.0f;
    float velocityY = 0.0f;

    float moveSpeed = 80.0f;
    float jumpForce = 250.0f;
    float gravity = 600.0f;


    // =========================================================
    // WALK ANIMATION
    // =========================================================

    float walkAnimationSpeed = 9.0f;

    float legSwingPower = 25.0f;
    float armSwingPower = 20.0f;


    // =========================================================
    // STATE
    // =========================================================

    bool grounded = false;

    bool facingRight = true;

    bool walking = false;

    float animationTime = 0.0f;


    // =========================================================
    // LOAD TEXTURE
    // =========================================================

    bool loadTexture(
        SDL_Renderer* renderer,
        SpritePart& part,
        const char* path,
        float x,
        float y)
    {
        part.texture =
            IMG_LoadTexture(
                renderer,
                path);

        if (!part.texture)
        {
            std::cerr
                << "Failed to load "
                << path
                << ": "
                << SDL_GetError()
                << '\n';

            return false;
        }


        // Pixel art:
        // jangan blur ketika texture diperbesar.

        if (!SDL_SetTextureScaleMode(
                part.texture,
                SDL_SCALEMODE_NEAREST))
        {
            std::cerr
                << "Failed to set nearest filtering for "
                << path
                << ": "
                << SDL_GetError()
                << '\n';
        }


        part.transform.x = x;
        part.transform.y = y;

        return true;
    }


    // =========================================================
    // LOAD PLAYER
    // =========================================================

    bool load(SDL_Renderer* renderer)
    {
        bool success = true;


        // -----------------------------------------------------
        // BODY
        // -----------------------------------------------------

        success &=
            loadTexture(
                renderer,
                body,
                "assets/player/body.png",
                0.0f,
                0.0f);


        // -----------------------------------------------------
        // HEAD
        // -----------------------------------------------------

        success &=
            loadTexture(
                renderer,
                head,
                "assets/player/head.png",
                -2.0f,
                -8.0f);


        // -----------------------------------------------------
        // ARMS
        // L = BACK
        // R = FRONT
        // -----------------------------------------------------

        success &=
            loadTexture(
                renderer,
                armBack,
                "assets/player/Arm(L).png",
                0.0f,
                0.0f);


        success &=
            loadTexture(
                renderer,
                armFront,
                "assets/player/Arm(R).png",
                0.0f,
                0.0f);


        // -----------------------------------------------------
        // LEGS
        // L = BACK
        // R = FRONT
        // -----------------------------------------------------

        success &=
            loadTexture(
                renderer,
                legBack,
                "assets/player/Leg(L).png",
                0.0f,
                12.0f);


        success &=
            loadTexture(
                renderer,
                legFront,
                "assets/player/Leg(R).png",
                0.0f,
                12.0f);


        // =====================================================
        // PIVOTS
        // =====================================================

        body.pivot =
            {2.0f, 0.0f};

        head.pivot =
            {4.0f, 8.0f};

        armFront.pivot =
            {2.0f, 0.0f};

        armBack.pivot =
            {2.0f, 0.0f};

        legFront.pivot =
            {2.0f, 0.0f};

        legBack.pivot =
            {2.0f, 0.0f};


        return success;
    }


    // =========================================================
    // AIM
    // =========================================================

    void updateAim(
        float mouseX,
        float mouseY,
        const Camera& camera)
    {
        // -----------------------------------------------------
        // HEAD PIVOT WORLD POSITION
        // -----------------------------------------------------

        float headX =
            transform.x
            + head.transform.x
            + head.pivot.x;

        float headY =
            transform.y
            + head.transform.y
            + head.pivot.y;


        // -----------------------------------------------------
        // SCREEN SPACE → WORLD SPACE
        // -----------------------------------------------------

        float worldMouseX =
            mouseX + camera.x;

        float worldMouseY =
            mouseY + camera.y;


        // -----------------------------------------------------
        // DIRECTION
        // -----------------------------------------------------

        float dx =
            worldMouseX - headX;

        float dy =
            worldMouseY - headY;


        // -----------------------------------------------------
        // PLAYER FACING
        // -----------------------------------------------------

        if (dx > 0.0f)
        {
            facingRight = true;
        }
        else if (dx < 0.0f)
        {
            facingRight = false;
        }


        // -----------------------------------------------------
        // ANGLE
        // -----------------------------------------------------

        float angle =
            std::atan2(
                dy,
                std::fabs(dx));


        float angleDegrees =
            angle
            * 180.0f
            / 3.14159265f;


        // -----------------------------------------------------
        // HEAD ROTATION
        //
        // Kepala hanya mengayun dari arah badan.
        // Tidak lagi melakukan rotasi penuh 180°.
        // -----------------------------------------------------

        if (facingRight)
        {
            head.transform.rotation =
                angleDegrees;
        }
        else
        {
            head.transform.rotation =
                -angleDegrees;
        }
    }


    // =========================================================
    // ANIMATION
    // =========================================================

    void updateAnimation(float deltaTime)
    {
        if (!walking)
        {
            armFront.animationRotation *= 0.8f;
            armBack.animationRotation *= 0.8f;

            legFront.animationRotation *= 0.8f;
            legBack.animationRotation *= 0.8f;

            animationTime = 0.0f;

            return;
        }


        animationTime +=
            deltaTime;


        float wave =
            std::sin(
                animationTime
                * walkAnimationSpeed);


        legFront.animationRotation =
            wave
            * legSwingPower;


        legBack.animationRotation =
            -wave
            * legSwingPower;


        armFront.animationRotation =
            -wave
            * armSwingPower;


        armBack.animationRotation =
            wave
            * armSwingPower;
    }


    // =========================================================
    // PHYSICS
    // =========================================================

    void updatePhysics(float deltaTime, const World& world)
    {
        constexpr float EPSILON = 0.001f;

        const float hitboxWidth = 4.0f * transform.scale;

        const float hitboxHeight = 24.0f * transform.scale;

        const float tileSize = World::TILE_SIZE;

        grounded = false;

    // =========================================
    // HORIZONTAL MOVEMENT
    // =========================================

        transform.x += velocityX * deltaTime;

        float left = transform.x;
        float right = transform.x + hitboxWidth;

        float top = transform.y;
        float bottom = transform.y + hitboxHeight;

        int minTileX = static_cast<int>(
            std::floor(left / tileSize)
        );

        int maxTileX = static_cast<int>(
            std::floor((right - EPSILON) / tileSize)
        );

        int minTileY = static_cast<int>(
            std::floor(top / tileSize)
        );

        int maxTileY = static_cast<int>(
            std::floor((bottom - EPSILON) / tileSize)
        );

        for (int y = minTileY; y <= maxTileY; y++)
        {
            for (int x = minTileX; x <= maxTileX; x++)
            {
                if (!world.isSolid(x, y))
                continue;

                float blockLeft = x * tileSize;
                float blockRight = blockLeft + tileSize;

                if (velocityX > 0.0f)
                {
                    transform.x = blockLeft - hitboxWidth;
                }
                else if (velocityX < 0.0f)
                {
                transform.x = blockRight;
                }
            }
        }

    // =========================================
    // VERTICAL MOVEMENT
    // =========================================

    velocityY += gravity * deltaTime;

    transform.y += velocityY * deltaTime;

    left = transform.x;
    right = transform.x + hitboxWidth;

    top = transform.y;
    bottom = transform.y + hitboxHeight;

    minTileX = static_cast<int>(
        std::floor(left / tileSize)
    );

    maxTileX = static_cast<int>(
        std::floor((right - EPSILON) / tileSize)
    );

    minTileY = static_cast<int>(
        std::floor(top / tileSize)
    );

    maxTileY = static_cast<int>(
        std::floor((bottom - EPSILON) / tileSize)
    );

    for (int y = minTileY; y <= maxTileY; y++)
    {
        for (int x = minTileX; x <= maxTileX; x++)
        {
            if (!world.isSolid(x, y))
                continue;

            float blockTop = y * tileSize;
            float blockBottom = blockTop + tileSize;

            if (velocityY > 0.0f)
            {
                transform.y = blockTop - hitboxHeight;
                velocityY = 0.0f;
                grounded = true;
            }
            else if (velocityY < 0.0f)
            {
                transform.y = blockBottom;
                velocityY = 0.0f;
            }
        }
    }
}

    // =========================================================
    // JUMP
    // =========================================================

    void jump()
    {
        if (!grounded)
            return;


        velocityY =
            -jumpForce;


        grounded =
            false;
    }


    // =========================================================
    // RENDER PART
    // =========================================================

    void renderPart(
        SDL_Renderer* renderer,
        const SpritePart& part,
        const Camera& camera)
    {
        if (!part.texture)
            return;


        float width = 0.0f;
        float height = 0.0f;


        if (!SDL_GetTextureSize(
                part.texture,
                &width,
                &height))
        {
            return;
        }


        float scale =
            transform.scale
            * part.transform.scale;


        // -----------------------------------------------------
        // WORLD → SCREEN
        // -----------------------------------------------------

        SDL_FRect destination
        {
            transform.x
            + part.transform.x
              * transform.scale
            - camera.x,

            transform.y
            + part.transform.y
              * transform.scale
            - camera.y,

            width * scale,
            height * scale
        };


        SDL_FPoint pivot
        {
            part.pivot.x * scale,
            part.pivot.y * scale
        };


        float rotation =
            part.transform.rotation
            + part.animationRotation;


        // -----------------------------------------------------
        // HORIZONTAL FLIP
        // -----------------------------------------------------

        SDL_FlipMode flip =
            facingRight
                ? SDL_FLIP_NONE
                : SDL_FLIP_HORIZONTAL;


        SDL_RenderTextureRotated(
            renderer,
            part.texture,
            nullptr,
            &destination,
            rotation,
            &pivot,
            flip);
    }


    // =========================================================
    // RENDER
    // =========================================================

    void render(
        SDL_Renderer* renderer,
        const Camera& camera)
    {
        // BACK

        renderPart(
            renderer,
            legBack,
            camera);


        renderPart(
            renderer,
            armBack,
            camera);


        // BODY

        renderPart(
            renderer,
            body,
            camera);


        // HEAD

        renderPart(
            renderer,
            head,
            camera);


        // FRONT

        renderPart(
            renderer,
            legFront,
            camera);


        renderPart(
            renderer,
            armFront,
            camera);
    }


    // =========================================================
    // DESTROY
    // =========================================================

    void destroy()
    {
        SpritePart* parts[]
        {
            &body,
            &head,

            &armFront,
            &armBack,

            &legFront,
            &legBack
        };


        for (SpritePart* part : parts)
        {
            if (part->texture)
            {
                SDL_DestroyTexture(
                    part->texture);

                part->texture =
                    nullptr;
            }
        }
    }
};