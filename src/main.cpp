#include <SDL3/SDL.h>

#include <iostream>
#include <cstdint>
#include <cmath>

#include "Player.hpp"
#include "Camera.hpp"
#include "World.hpp"
#include "Inventory.hpp"

// =============================================================
// HOTBAR RENDERING
// =============================================================

void renderDigit(
    SDL_Renderer* renderer,
    int digit,
    float x,
    float y,
    float pixelSize
) {
    static constexpr uint8_t digits[10][5] = {
        {7, 5, 5, 5, 7},
        {2, 6, 2, 2, 7},
        {7, 1, 7, 4, 7},
        {7, 1, 7, 1, 7},
        {5, 5, 7, 1, 1},
        {7, 4, 7, 1, 7},
        {7, 4, 7, 5, 7},
        {7, 1, 1, 1, 1},
        {7, 5, 7, 5, 7},
        {7, 5, 7, 1, 7}
    };

    if (digit < 0 || digit > 9) {
        return;
    }

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            if (digits[digit][row] & (1 << (2 - col))) {
                SDL_FRect pixel{
                    x + col * pixelSize,
                    y + row * pixelSize,
                    pixelSize,
                    pixelSize
                };

                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
}

void renderNumber(
    SDL_Renderer* renderer,
    int number,
    float x,
    float y
) {
    if (number <= 0) {
        return;
    }

    constexpr float PIXEL_SIZE = 2.0f;
    constexpr float DIGIT_WIDTH = 8.0f;

    int digits[10];
    int digitCount = 0;

    while (number > 0 && digitCount < 10) {
        digits[digitCount++] = number % 10;
        number /= 10;
    }

    // Shadow
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);

    for (int i = digitCount - 1; i >= 0; i--) {
        float digitX = x + (digitCount - 1 - i) * DIGIT_WIDTH;

        renderDigit(
            renderer,
            digits[i],
            digitX + 1.0f,
            y + 1.0f,
            PIXEL_SIZE
        );
    }

    // Foreground
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int i = digitCount - 1; i >= 0; i--) {
        float digitX = x + (digitCount - 1 - i) * DIGIT_WIDTH;

        renderDigit(
            renderer,
            digits[i],
            digitX,
            y,
            PIXEL_SIZE
        );
    }
}

void renderHotbar(
    SDL_Renderer* renderer,
    const World& world,
    const Inventory& inventory,
    const BlockID hotbarBlocks[],
    int hotbarSize,
    BlockID selectedBlock,
    int screenWidth,
    int screenHeight
) {
    constexpr float SLOT_SIZE = 54.0f;
    constexpr float SLOT_GAP = 5.0f;
    constexpr float ICON_SIZE = 32.0f;

    const float totalWidth =
        hotbarSize * SLOT_SIZE +
        (hotbarSize - 1) * SLOT_GAP;

    const float startX =
        (screenWidth - totalWidth) * 0.5f;

    const float startY =
        screenHeight - SLOT_SIZE - 20.0f;

    for (int i = 0; i < hotbarSize; i++) {
        const BlockID id = hotbarBlocks[i];

        const float x = startX + i * (SLOT_SIZE + SLOT_GAP);
        const float y = startY;

        const bool selected = id == selectedBlock;

        SDL_FRect slot{
            x, y,
            SLOT_SIZE,
            SLOT_SIZE
        };

        // Background
        SDL_SetRenderDrawColor(
            renderer,
            30, 30, 30, 220
        );

        SDL_RenderFillRect(renderer, &slot);

        // Border
        if (selected) {
            SDL_SetRenderDrawColor(
                renderer,
                255, 220, 90, 255
            );
        } else {
            SDL_SetRenderDrawColor(
                renderer,
                120, 120, 120, 255
            );
        }

        SDL_RenderRect(renderer, &slot);

        // Block icon
        SDL_Texture* texture = world.getTexture(id);

        if (texture) {
            SDL_FRect icon{
                x + (SLOT_SIZE - ICON_SIZE) * 0.5f,
                y + (SLOT_SIZE - ICON_SIZE) * 0.5f,
                ICON_SIZE,
                ICON_SIZE
            };

            SDL_RenderTexture(
                renderer,
                texture,
                nullptr,
                &icon
            );
        }

        // Item count
        const int count = inventory.getCount(id);

        if (count > 0) {
            const float numberWidth =
                count >= 100 ? 24.0f :
                count >= 10  ? 16.0f : 8.0f;

            renderNumber(
                renderer,
                count,
                x + SLOT_SIZE - numberWidth - 4.0f,
                y + SLOT_SIZE - 13.0f
            );
        }
    }
}

// =============================================================
// BLOCK INTERACTION
// =============================================================

void handleBlockInteraction(
    World& world,
    Player& player,
    Inventory& inventory,
    BlockID selectedBlock,
    const Camera& camera,
    float mouseX,
    float mouseY,
    bool breakPressed,
    bool placePressed
) {
    if (!breakPressed && !placePressed) {
        return;
    }

    // Screen position -> world position
    const float worldMouseX = mouseX + camera.x;
    const float worldMouseY = mouseY + camera.y;

    // World position -> block coordinate
    const int targetX = static_cast<int>(
        std::floor(worldMouseX / World::TILE_SIZE)
    );

    const int targetY = static_cast<int>(
        std::floor(worldMouseY / World::TILE_SIZE)
    );

    // Player reference point
    const float playerCenterX =
        player.transform.x + 2.0f * player.transform.scale;

    const float playerCenterY =
        player.transform.y + 12.0f * player.transform.scale;

    // Target block center
    const float blockCenterX =
        (targetX + 0.5f) * World::TILE_SIZE;

    const float blockCenterY =
        (targetY + 0.5f) * World::TILE_SIZE;

    const float dx = blockCenterX - playerCenterX;
    const float dy = blockCenterY - playerCenterY;

    constexpr float REACH = 5.0f * World::TILE_SIZE;

    const bool inReach =
        dx * dx + dy * dy <= REACH * REACH;

    if (!inReach) {
        return;
    }

    // ---------------------------------------------------------
    // BREAK BLOCK
    // ---------------------------------------------------------

    if (breakPressed) {
        const BlockID droppedBlock =
            world.breakBlock(targetX, targetY);

        if (droppedBlock != BlockID::Air) {
            inventory.add(droppedBlock);
        }
    }

    // ---------------------------------------------------------
    // PLACE BLOCK
    // ---------------------------------------------------------

    if (placePressed) {
        const BlockID targetBlock =
            world.getBlock(targetX, targetY);

        if (
            targetBlock == BlockID::Air &&
            inventory.getCount(selectedBlock) > 0
        ) {
            constexpr float EPSILON = 0.001f;

            const float blockLeft =
                targetX * World::TILE_SIZE;

            const float blockRight =
                blockLeft + World::TILE_SIZE;

            const float blockTop =
                targetY * World::TILE_SIZE;

            const float blockBottom =
                blockTop + World::TILE_SIZE;

            const float playerLeft =
                player.transform.x;

            const float playerRight =
                playerLeft + 4.0f * player.transform.scale;

            const float playerTop =
                player.transform.y;

            const float playerBottom =
                playerTop + 24.0f * player.transform.scale;

            const bool overlapsPlayer =
                playerRight > blockLeft + EPSILON &&
                playerLeft < blockRight - EPSILON &&
                playerBottom > blockTop + EPSILON &&
                playerTop < blockBottom - EPSILON;

            if (!overlapsPlayer) {
                if (world.setBlock(
                    targetX,
                    targetY,
                    selectedBlock
                )) {
                    inventory.remove(selectedBlock);
                }
            }
        }
    }
}

// =============================================================
// MAIN
// =============================================================

int main() {

    // =========================================================
    // SDL INITIALIZATION
    // =========================================================

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: "
                  << SDL_GetError() << '\n';
        return 1;
    }

    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;

    SDL_Window* window = SDL_CreateWindow(
        "Minecraft2D",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    if (!window) {
        std::cerr << "Window creation failed: "
                  << SDL_GetError() << '\n';

        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        nullptr
    );

    if (!renderer) {
        std::cerr << "Renderer creation failed: "
                  << SDL_GetError() << '\n';

        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // =========================================================
    // PLAYER
    // =========================================================

    Player player;

    player.transform.x = 600.0f;
    player.transform.y = 520.0f;
    player.transform.scale = 4.0f;

    player.moveSpeed = 200.0f;
    player.gravity = 1500.0f;
    player.jumpForce = 450.0f;

    player.walkAnimationSpeed = 9.0f;
    player.legSwingPower = 25.0f;
    player.armSwingPower = 20.0f;

    if (!player.load(renderer)) {
        player.destroy();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

    // =========================================================
    // CAMERA
    // =========================================================

    Camera camera;

    camera.screenWidth = WINDOW_WIDTH;
    camera.screenHeight = WINDOW_HEIGHT;
    camera.followSpeed = 5.0f;

    camera.x = player.transform.x - WINDOW_WIDTH * 0.5f;
    camera.y = player.transform.y - WINDOW_HEIGHT * 0.5f;

    // =========================================================
    // WORLD
    // =========================================================

    World world;

    if (!world.load(renderer)) {
        std::cerr << "World loading failed!\n";

        world.destroy();
        player.destroy();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

    world.generate();

    // =========================================================
    // INVENTORY
    // =========================================================

    Inventory inventory;

    BlockID selectedBlock = BlockID::Dirt;

    const BlockID hotbarBlocks[] = {
        BlockID::Grass,
        BlockID::Dirt,
        BlockID::Stone,
        BlockID::CoalOre,
        BlockID::IronOre,
        BlockID::DiamondOre,
        BlockID::Log,
        BlockID::Leaves
    };

    constexpr int HOTBAR_SIZE = 8;

    // =========================================================
    // INPUT STATE
    // =========================================================

    bool breakPressed = false;
    bool placePressed = false;
    bool running = true;

    Uint64 previousTime = SDL_GetTicks();

    // =========================================================
    // GAME LOOP
    // =========================================================

    while (running) {

        // -----------------------------------------------------
        // DELTA TIME
        // -----------------------------------------------------

        const Uint64 currentTime = SDL_GetTicks();

        float deltaTime =
            (currentTime - previousTime) / 1000.0f;

        previousTime = currentTime;

        if (deltaTime > 0.05f) {
            deltaTime = 0.05f;
        }

        // -----------------------------------------------------
        // EVENTS
        // -----------------------------------------------------

        SDL_Event event;

        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (
                event.type == SDL_EVENT_KEY_DOWN &&
                !event.key.repeat
            ) {
                // Hotbar selection
                if (
                    event.key.key >= SDLK_1 &&
                    event.key.key <= SDLK_8
                ) {
                    const int index =
                        event.key.key - SDLK_1;

                    selectedBlock = hotbarBlocks[index];
                }

                // Jump
                if (event.key.key == SDLK_SPACE) {
                    player.jump();
                }
            }

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {

                if (event.button.button == SDL_BUTTON_LEFT) {
                    breakPressed = true;
                }

                if (event.button.button == SDL_BUTTON_RIGHT) {
                    placePressed = true;
                }
            }
        }

        // -----------------------------------------------------
        // KEYBOARD MOVEMENT
        // -----------------------------------------------------

        const bool* keyboard = SDL_GetKeyboardState(nullptr);

        float movement = 0.0f;

        if (keyboard[SDL_SCANCODE_A]) {
            movement -= 1.0f;
        }

        if (keyboard[SDL_SCANCODE_D]) {
            movement += 1.0f;
        }

        player.walking = movement != 0.0f;

        player.velocityX = movement * player.moveSpeed;

        // -----------------------------------------------------
        // CAMERA
        // -----------------------------------------------------

        camera.follow(
            player.transform.x,
            player.transform.y,
            deltaTime
        );

        // -----------------------------------------------------
        // MOUSE
        // -----------------------------------------------------

        float mouseX = 0.0f;
        float mouseY = 0.0f;

        SDL_GetMouseState(&mouseX, &mouseY);

        player.updateAim(
            mouseX,
            mouseY,
            camera
        );

        // -----------------------------------------------------
        // WORLD UPDATE
        // -----------------------------------------------------

        world.update(
            player.transform.x,
            player.transform.y
        );

        // -----------------------------------------------------
        // BLOCK INTERACTION
        // -----------------------------------------------------

        handleBlockInteraction(
            world,
            player,
            inventory,
            selectedBlock,
            camera,
            mouseX,
            mouseY,
            breakPressed,
            placePressed
        );

        breakPressed = false;
        placePressed = false;

        // -----------------------------------------------------
        // PHYSICS & ANIMATION
        // -----------------------------------------------------

        player.updatePhysics(deltaTime, world);
        player.updateAnimation(deltaTime);

        // -----------------------------------------------------
        // RENDERING
        // -----------------------------------------------------

        SDL_SetRenderDrawColor(
            renderer,
            30, 30, 30, 255
        );

        SDL_RenderClear(renderer);

        // World
        world.render(renderer, camera);

        // Player
        player.render(renderer, camera);

        // Hotbar (screen-space, not affected by camera)
        renderHotbar(
            renderer,
            world,
            inventory,
            hotbarBlocks,
            HOTBAR_SIZE,
            selectedBlock,
            WINDOW_WIDTH,
            WINDOW_HEIGHT
        );

        SDL_RenderPresent(renderer);
    }

    // =========================================================
    // CLEANUP
    // =========================================================

    world.destroy();
    player.destroy();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}