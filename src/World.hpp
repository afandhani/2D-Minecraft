#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>

#include "Block.hpp"
#include "Camera.hpp"
#include "ChunkManager.hpp"

class World {
public:
    static constexpr float TILE_SIZE = 64.0f;

private:
    std::array<SDL_Texture*, 9> textures{};

    ChunkManager chunkManager{12345};

    // Floor division agar koordinat negatif tetap benar.
    int floorDiv(int value, int divisor) const {
        int result = value / divisor;
        int remainder = value % divisor;

        if (remainder < 0) {
            result--;
        }

        return result;
    }

public:
    // Memuat semua tekstur blok.
    bool load(SDL_Renderer* renderer) {
        struct TextureInfo {
            BlockID id;
            const char* path;
        };

        const TextureInfo textureInfo[] = {
            {BlockID::Grass, "assets/blocks/grass_block.png"},
            {BlockID::Dirt, "assets/blocks/dirt.png"},
            {BlockID::Stone, "assets/blocks/stone.png"},
            {BlockID::CoalOre, "assets/blocks/coal_ore.png"},
            {BlockID::IronOre, "assets/blocks/iron_ore.png"},
            {BlockID::DiamondOre, "assets/blocks/diamond_ore.png"},
            {BlockID::Log, "assets/blocks/oak_log.png"},
            {BlockID::Leaves, "assets/blocks/leaves.png"}
        };

        for (const auto& info : textureInfo) {
            int index = static_cast<int>(info.id);

            textures[index] = IMG_LoadTexture(renderer, info.path);

            if (!textures[index]) {
                std::cerr
                    << "Texture load failed: "
                    << info.path << '\n'
                    << SDL_GetError() << '\n';

                destroy();
                return false;
            }

            SDL_SetTextureScaleMode(
                textures[index],
                SDL_SCALEMODE_NEAREST
            );
        }

        return true;
    }

    // Inisialisasi chunk awal.
    void generate() {
        chunkManager.update(0.0f, 0.0f);
    }

    // Parameter menggunakan koordinat pixel dunia.
    void update(float playerWorldX, float playerWorldY) {
        const float tileX = playerWorldX / TILE_SIZE;
        const float tileY = playerWorldY / TILE_SIZE;

        chunkManager.update(tileX, tileY);
    }

    // Mengambil blok berdasarkan koordinat blok dunia.
    BlockID getBlock(int x, int y) const {
        const int chunkX = floorDiv(x, Chunk::WIDTH);
        const int chunkY = floorDiv(y, Chunk::HEIGHT);

        const int localX = x - chunkX * Chunk::WIDTH;
        const int localY = y - chunkY * Chunk::HEIGHT;

        const Chunk* chunk = chunkManager.getChunk(
            chunkX,
            chunkY
        );

        if (!chunk) {
            return BlockID::Air;
        }

        return chunk->getBlock(localX, localY);
    }

    // Mengubah blok berdasarkan koordinat blok dunia.
    bool setBlock(int x, int y, BlockID id) {
        const int chunkX = floorDiv(x, Chunk::WIDTH);
        const int chunkY = floorDiv(y, Chunk::HEIGHT);

        const int localX = x - chunkX * Chunk::WIDTH;
        const int localY = y - chunkY * Chunk::HEIGHT;

        Chunk* chunk = chunkManager.getChunk(
            chunkX,
            chunkY
        );

        if (!chunk) {
            return false;
        }

        chunk->setBlock(localX, localY, id);
        return true;
    }

    // Menghancurkan blok dan mengembalikan ID blok yang dihancurkan.
    BlockID breakBlock(int x, int y) {
        const BlockID id = getBlock(x, y);

        if (id == BlockID::Air) {
            return BlockID::Air;
        }

        if (setBlock(x, y, BlockID::Air)) {
            return id;
        }

        return BlockID::Air;
    }

    // Mengambil tekstur berdasarkan ID blok.
    SDL_Texture* getTexture(BlockID id) const {
        const int index = static_cast<int>(id);

        if (index < 0 || index >= static_cast<int>(textures.size())) {
            return nullptr;
        }

        return textures[index];
    }

    // Menggambar semua chunk yang sedang dimuat.
    void render(
        SDL_Renderer* renderer,
        const Camera& camera
    ) {
        const auto& chunks = chunkManager.getChunks();

        for (const auto& [coord, chunkPtr] : chunks) {
            const Chunk& chunk = *chunkPtr;

            const int startX = chunk.getWorldX();
            const int startY = chunk.getWorldY();

            for (int y = 0; y < Chunk::HEIGHT; y++) {
                for (int x = 0; x < Chunk::WIDTH; x++) {
                    const BlockID id = chunk.getBlock(x, y);

                    if (id == BlockID::Air) {
                        continue;
                    }

                    SDL_Texture* texture = getTexture(id);

                    if (!texture) {
                        continue;
                    }

                    SDL_FRect destination{
                        (startX + x) * TILE_SIZE - camera.x,
                        (startY + y) * TILE_SIZE - camera.y,
                        TILE_SIZE,
                        TILE_SIZE
                    };

                    SDL_RenderTexture(
                        renderer,
                        texture,
                        nullptr,
                        &destination
                    );
                }
            }
        }
    }

    // Memeriksa apakah blok solid.
    // x dan y menggunakan koordinat blok, bukan pixel.
    bool isSolid(int x, int y) const {
        const int chunkX = floorDiv(x, Chunk::WIDTH);
        const int chunkY = floorDiv(y, Chunk::HEIGHT);

        const int localX = x - chunkX * Chunk::WIDTH;
        const int localY = y - chunkY * Chunk::HEIGHT;

        const Chunk* chunk = chunkManager.getChunk(
            chunkX,
            chunkY
        );

        if (!chunk) {
            return false;
        }

        return chunk->isSolid(localX, localY);
    }

    std::size_t getLoadedChunkCount() const {
        return chunkManager.getLoadedChunkCount();
    }

    // Membersihkan seluruh tekstur.
    void destroy() {
        for (SDL_Texture*& texture : textures) {
            if (texture) {
                SDL_DestroyTexture(texture);
                texture = nullptr;
            }
        }
    }
};