#pragma once

#include <unordered_map>
#include <memory>
#include <cstdint>
#include <cmath>

#include "Chunk.hpp"
#include "WorldGenerator.hpp"

struct ChunkCoord {
    int x;
    int y;

    bool operator==(const ChunkCoord& other) const {
        return x == other.x && y == other.y;
    }
};

struct ChunkCoordHash {
    std::size_t operator()(const ChunkCoord& coord) const {
        const uint64_t x = static_cast<uint32_t>(coord.x);
        const uint64_t y = static_cast<uint32_t>(coord.y);

        return std::hash<uint64_t>{}(
            (x << 32) | y
        );
    }
};

class ChunkManager {
private:
    std::unordered_map<
        ChunkCoord,
        std::unique_ptr<Chunk>,
        ChunkCoordHash
    > chunks;

    WorldGenerator generator;

    int loadRadiusX = 3;
    int loadRadiusY = 2;

    int worldToChunk(int position, int chunkSize) const {
        return static_cast<int>(
            std::floor(
                static_cast<double>(position) / chunkSize
            )
        );
    }

public:
    explicit ChunkManager(uint32_t seed = 12345)
        : generator(seed) {}
        const auto& getChunks() const {
            return chunks;
        }

        auto& getChunks() {
            return chunks;
        }

    void update(float playerX, float playerY) {
        const int centerX = worldToChunk(
            static_cast<int>(std::floor(playerX)),
            Chunk::WIDTH
        );

        const int centerY = worldToChunk(
            static_cast<int>(std::floor(playerY)),
            Chunk::HEIGHT
        );

        // Load nearby chunks
        for (int y = centerY - loadRadiusY;
             y <= centerY + loadRadiusY; y++) {

            for (int x = centerX - loadRadiusX;
                 x <= centerX + loadRadiusX; x++) {

                ChunkCoord coord{x, y};

                if (chunks.find(coord) == chunks.end()) {
                    auto chunk = std::make_unique<Chunk>(x, y);

                    generator.generate(*chunk);

                    chunks.emplace(coord, std::move(chunk));
                }
            }
        }

        // Remove chunks outside loading radius
        for (auto it = chunks.begin(); it != chunks.end();) {
            const int dx = std::abs(it->first.x - centerX);
            const int dy = std::abs(it->first.y - centerY);

            if (dx > loadRadiusX || dy > loadRadiusY) {
                it = chunks.erase(it);
            } else {
                ++it;
            }
        }
    }

    Chunk* getChunk(int chunkX, int chunkY) {
        auto it = chunks.find({chunkX, chunkY});

        if (it == chunks.end()) {
            return nullptr;
        }

        return it->second.get();
    }

    const Chunk* getChunk(int chunkX, int chunkY) const {
        auto it = chunks.find({chunkX, chunkY});

        if (it == chunks.end()) {
            return nullptr;
        }

        return it->second.get();
    }

    std::size_t getLoadedChunkCount() const {
        return chunks.size();
    }

    void setLoadRadius(int x, int y) {
        loadRadiusX = x;
        loadRadiusY = y;
    }
};