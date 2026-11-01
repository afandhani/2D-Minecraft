#pragma once

#include <cstdint>
#include <cmath>
#include "Chunk.hpp"

class WorldGenerator {
private:
    uint32_t seed;

    uint32_t hash(int x, int y) const {
        uint32_t h = seed;

        h ^= static_cast<uint32_t>(x) * 374761393u;
        h ^= static_cast<uint32_t>(y) * 668265263u;

        h = (h ^ (h >> 13)) * 1274126177u;

        return h ^ (h >> 16);
    }

    int randomRange(int x, int y, int min, int max) const {
        uint32_t value = hash(x, y);

        return min + value % static_cast<uint32_t>(max - min + 1);
    }

    int getSurfaceHeight(int worldX) const {
        constexpr int BASE_HEIGHT = 14;
        constexpr int INTERVAL = 12;

        int left = static_cast<int>(
            std::floor(static_cast<float>(worldX) / INTERVAL)
        ) * INTERVAL;

        int right = left + INTERVAL;

        int leftHeight =
            BASE_HEIGHT + randomRange(left, 0, -2, 2);

        int rightHeight =
            BASE_HEIGHT + randomRange(right, 0, -2, 2);

        float t = static_cast<float>(worldX - left) / INTERVAL;

        // Smooth interpolation
        t = t * t * (3.0f - 2.0f * t);

        return static_cast<int>(
            std::round(leftHeight + (rightHeight - leftHeight) * t)
        );
    }

    bool shouldGenerateTree(int worldX) const {
        if (worldX % 8 != 0) {
            return false;
        }

        return randomRange(worldX, 100, 1, 100) <= 35;
    }

public:
    explicit WorldGenerator(uint32_t worldSeed = 12345)
        : seed(worldSeed) {}

    uint32_t getSeed() const {
        return seed;
    }

    void generate(Chunk& chunk) const {
        const int chunkWorldX = chunk.getWorldX();
        const int chunkWorldY = chunk.getWorldY();

        auto& blocks = chunk.getBlocks();

        // 1. Generate terrain
        for (int x = 0; x < Chunk::WIDTH; x++) {
            const int worldX = chunkWorldX + x;
            const int surface = getSurfaceHeight(worldX);

            for (int y = 0; y < Chunk::HEIGHT; y++) {
                const int worldY = chunkWorldY + y;

                if (worldY < surface) {
                    blocks[y][x] = BlockID::Air;
                }
                else if (worldY == surface) {
                    blocks[y][x] = BlockID::Grass;
                }
                else if (worldY <= surface + 3) {
                    blocks[y][x] = BlockID::Dirt;
                }
                else {
                    const int depth = worldY - surface;
                    const int ore = randomRange(worldX, worldY, 1, 100);

                    blocks[y][x] = BlockID::Stone;

                    if (depth > 20 && ore == 1) {
                        blocks[y][x] = BlockID::DiamondOre;
                    }
                    else if (depth > 10 && ore <= 4) {
                        blocks[y][x] = BlockID::IronOre;
                    }
                    else if (depth > 5 && ore <= 8) {
                        blocks[y][x] = BlockID::CoalOre;
                    }
                }
            }
        }

        // 2. Generate trees
        // Scan extra columns so leaves can cross chunk boundaries.
        for (int worldX = chunkWorldX - 2;
             worldX < chunkWorldX + Chunk::WIDTH + 2;
             worldX++) {

            if (!shouldGenerateTree(worldX)) {
                continue;
            }

            const int ground = getSurfaceHeight(worldX);
            const int treeHeight = randomRange(worldX, 200, 3, 4);

            const int top = ground - treeHeight;

            // Trunk
            for (int i = 1; i <= treeHeight; i++) {
                const int trunkWorldY = ground - i;

                const int localX = worldX - chunkWorldX;
                const int localY = trunkWorldY - chunkWorldY;

                if (localX >= 0 && localX < Chunk::WIDTH &&
                    localY >= 0 && localY < Chunk::HEIGHT) {

                    blocks[localY][localX] = BlockID::Log;
                }
            }

            // Leaves
            for (int dx = -2; dx <= 2; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    const int leafWorldX = worldX + dx;
                    const int leafWorldY = top + dy;

                    const int localX = leafWorldX - chunkWorldX;
                    const int localY = leafWorldY - chunkWorldY;

                    if (localX < 0 || localX >= Chunk::WIDTH ||
                        localY < 0 || localY >= Chunk::HEIGHT) {
                        continue;
                    }

                    if (blocks[localY][localX] == BlockID::Air) {
                        blocks[localY][localX] = BlockID::Leaves;
                    }
                }
            }
        }
    }
};