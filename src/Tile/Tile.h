#pragma once

#include <vector>
#include <map>

typedef struct Tile
{
    bool collapsed = false;
    std::vector<int> options = {0, 1, 2, 3, 4, 5, 6, 7};
} Tile;

typedef struct TileOptions {
    std::vector<int> up;
    std::vector<int> down;
    std::vector<int> left;
    std::vector<int> right;
} TileOptions;