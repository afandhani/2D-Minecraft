#pragma once

#include <array>
#include <cstdint>

#include "Block.hpp"

class Chunk
{
public:
    static constexpr int WIDTH = 16;
    static constexpr int HEIGHT = 40;

    using BlockArray =
        std::array<
            std::array<BlockID, WIDTH>,
            HEIGHT
        >;

private:
    int chunkX = 0;
    int chunkY = 0;

    BlockArray blocks{};

public:
    Chunk(int x, int y)
        : chunkX(x), chunkY(y)
    {
        blocks.fill({});
    }

    int getX() const
    {
        return chunkX;
    }

    int getY() const
    {
        return chunkY;
    }

    int getWorldX() const
    {
        return chunkX * WIDTH;
    }

    int getWorldY() const
    {
        return chunkY * HEIGHT;
    }

    BlockID getBlock(int localX, int localY) const
    {
        if (localX < 0 || localX >= WIDTH ||
            localY < 0 || localY >= HEIGHT)
        {
            return BlockID::Air;
        }

        return blocks[localY][localX];
    }

    void setBlock(int localX, int localY, BlockID id)
    {
        if (localX < 0 || localX >= WIDTH ||
            localY < 0 || localY >= HEIGHT)
        {
            return;
        }

        blocks[localY][localX] = id;
    }

    bool isSolid(int localX, int localY) const
    {
        return getBlock(localX, localY) != BlockID::Air;
    }

    BlockArray& getBlocks()
    {
        return blocks;
    }

    const BlockArray& getBlocks() const
    {
        return blocks;
    }
};