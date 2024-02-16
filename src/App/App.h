#pragma once

#include <string>
#include <algorithm>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <random>
#include <memory>
#include <map>
#include <vector>

#include <raylib.h>

#include "Tile/Tile.h"

class App
{
  App();

public:
  ~App();
  static App &GetInstance();
  void Run();

  App(App const &) = delete;
  void operator=(App const &) = delete;

private:
  void Update();
  void Draw();
  void HandleInput();

  void Collapse(int index);
  int GetIndexCrossEntropy();

  int intRand(const int & min, const int & max) {
      static thread_local std::mt19937 generator;
      std::uniform_int_distribution<int> distribution(min,max);
      return distribution(generator);
  }

private:
  float m_width = 1024;
  float m_height = 768;

  std::string m_title = "Wave Function Collapse";

  int m_numTilesX = 10;
  int m_numTilesY = 10;

  int m_tileSize = 32;

  std::vector<Texture2D> m_textures;
  std::vector<Tile> m_tiles;
  std::vector<std::shared_ptr<Tile>> m_grid;

  std::map<int, TileOptions> m_rules;
  bool m_done = true;
};
