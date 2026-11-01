#pragma once

#include <array>
#include "Block.hpp"

class Inventory {
private:
    std::array<int, 9> items{};

public:
    void add(BlockID id, int amount = 1) {
        if (id == BlockID::Air || amount <= 0) {
            return;
        }

        int index = static_cast<int>(id);

        if (index >= 0 && index < static_cast<int>(items.size())) {
            items[index] += amount;
        }
    }

    bool remove(BlockID id, int amount = 1) {
        if (id == BlockID::Air || amount <= 0) {
            return false;
        }

        int index = static_cast<int>(id);

        if (index < 0 || index >= static_cast<int>(items.size())) {
            return false;
        }

        if (items[index] < amount) {
            return false;
        }

        items[index] -= amount;
        return true;
    }

    int getCount(BlockID id) const {
        int index = static_cast<int>(id);

        if (index < 0 || index >= static_cast<int>(items.size())) {
            return 0;
        }

        return items[index];
    }
};