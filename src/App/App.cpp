#include "App.h"

#include <functional>

App::App()
{
  InitWindow(this->m_width, this->m_height, this->m_title.c_str());
  SetTargetFPS(60);

  for (int i = 0; i < 8; i++)
  {
    Image image = LoadImage(("resources/" + std::to_string(i) + ".png").c_str());
    ImageResize(&image, this->m_tileSize, this->m_tileSize);
    Texture2D texture = LoadTextureFromImage(image);
    this->m_textures.push_back(texture);
  }

  this->m_numTilesX = this->m_width / this->m_tileSize;
  this->m_numTilesY = this->m_height / this->m_tileSize;

  for (int i = 0; i < this->m_numTilesX; i++)
  {
    for (int j = 0; j < this->m_numTilesY; j++)
    {
        std::shared_ptr<Tile> tile = std::make_shared<Tile>();
        this->m_grid.push_back(tile);
    }
  }

  this->m_rules[0] = TileOptions{
      {0, 2, 6, 7},
      {0, 2, 4, 5},
      {0, 1, 5, 7},
      {0, 1, 4, 6}};

  this->m_rules[1] = TileOptions{
      {1, 3, 4, 5},
      {1, 3, 6, 7},
      {0, 1, 5, 7},
      {0, 1, 4, 6}};

  this->m_rules[2] = TileOptions{
      {0, 2, 6, 7},
      {0, 2, 4, 5},
      {2, 3, 4, 6},
      {2, 3, 5, 7}};
  this->m_rules[3] = TileOptions{
      {1, 3, 4, 5},
      {1, 3, 6, 7},
      {2, 3, 4, 6},
      {2, 3, 5, 7}};

  this->m_rules[4] = TileOptions{
      {0, 2, 6, 7},
      {1, 3, 6, 7},
      {0, 1, 5, 7},
      {2, 3, 5, 7}};
  this->m_rules[5] = TileOptions{
      {0, 2, 6, 7},
      {1, 3, 6, 7},
      {2, 3, 4, 6},
      {0, 1, 4, 6}};
  this->m_rules[6] = TileOptions{
      {1, 3, 4, 5},
      {0, 2, 4, 5},
      {0, 1, 5, 7},
      {2, 3, 5, 7}};
  this->m_rules[7] = TileOptions{
      {1, 3, 4, 5},
      {0, 2, 4, 5},
      {2, 3, 4, 6},
      {0, 1, 4, 6}};
}

App::~App()
{
  CloseWindow();
}

App &App::GetInstance()
{
  static App instance;
  return instance;
}

void App::Run()
{
  while (!WindowShouldClose())
  {
    this->HandleInput();
    this->Update();
    this->Draw();
  }
}

void App::Update() {}

void App::Draw()
{
  BeginDrawing();
  ClearBackground(RAYWHITE);
  for (int i = 0; i < this->m_numTilesX; i++)
  {
    for (int j = 0; j < this->m_numTilesY; j++)
    {
      int index = i * this->m_numTilesY + j;
      auto tile = this->m_grid[index];
      if (tile->collapsed)
      {
        DrawTexture(this->m_textures[tile->options[0]], i * this->m_tileSize, j * this->m_tileSize, WHITE);
        //DrawText(std::to_string(tile->options.size()).c_str(), i * this->m_tileSize + this->m_tileSize / 2, j * this->m_tileSize + this->m_tileSize / 2, 10, BLACK);
      }
      else
      {
          DrawRectangle(i * this->m_tileSize, j * this->m_tileSize, this->m_tileSize, this->m_tileSize, BLACK);
          DrawText(std::to_string(tile->options.size()).c_str(), i * this->m_tileSize + this->m_tileSize / 2, j * this->m_tileSize + this->m_tileSize / 2, 10, WHITE);
      }
    }
  }
  EndDrawing();
}

void App::HandleInput()
{
    if(IsKeyPressed(KEY_SPACE) && this->m_done) {
        // reset the grid
        this->m_done = false;
        for (int i = 0; i < this->m_numTilesX; i++)
        {
            for (int j = 0; j < this->m_numTilesY; j++)
            {
                auto tile = this->m_grid[i * this->m_numTilesY + j];
                tile->collapsed = false;
                tile->options = {0, 1, 2, 3, 4, 5, 6, 7};

                std::shuffle(tile->options.begin(), tile->options.end(), std::mt19937(std::random_device()()));
            }
        }

        size_t stackSize = 1024 * 1024;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, stackSize);

        pthread_t thread;
        pthread_create(&thread, &attr, [](void *app) -> void * {
            App *appPtr = static_cast<App *>(app);
            std::random_device rd;
              std::mt19937 gen(rd());
              std::uniform_int_distribution<> dis(0, appPtr->m_grid.size() - 1);
            int randomIndex = dis(gen);
            appPtr->Collapse(randomIndex);
            appPtr->m_done = true;
            return nullptr;
            },
                       this);

        pthread_detach(thread);
        pthread_attr_destroy(&attr);
    }
}

void App::Collapse(int index) {
    auto tile = this->m_grid[index];

    if(tile->collapsed || tile->options.size() == 1) {
        return;
    }

    tile->collapsed = true;
    tile->options = {tile->options[0]};

    int x = index / this->m_numTilesY;
    int y = index % this->m_numTilesY;

    if(x > 0) {
        auto leftTile = this->m_grid[(x - 1) * this->m_numTilesY + y];
        if(!leftTile->collapsed) {
            // remove the option from the left tile that is not in rule for right
            leftTile->options.erase(std::remove_if(leftTile->options.begin(), leftTile->options.end(), [tile, this](int option) {
                return std::find(this->m_rules[option].right.begin(), this->m_rules[option].right.end(), tile->options[0]) == this->m_rules[option].right.end();
            }), leftTile->options.end());
        }
    }

    if(x < this->m_numTilesX - 1) {
        auto rightTile = this->m_grid[(x + 1) * this->m_numTilesY + y];
        if(!rightTile->collapsed) {
            rightTile->options.erase(std::remove_if(rightTile->options.begin(), rightTile->options.end(), [tile, this](int option) {
                return std::find(this->m_rules[option].left.begin(), this->m_rules[option].left.end(), tile->options[0]) == this->m_rules[option].left.end();
            }), rightTile->options.end());
        }
    }

    if(y < this->m_numTilesY - 1) {
        auto downTile = this->m_grid[x * this->m_numTilesY + y + 1];
        downTile->options.erase(std::remove_if(downTile->options.begin(), downTile->options.end(), [tile, this](int option) {
            return std::find(this->m_rules[option].up.begin(), this->m_rules[option].up.end(), tile->options[0]) == this->m_rules[option].up.end();
        }), downTile->options.end());
    }


    if(y > 0) {
        auto upTile = this->m_grid[x * this->m_numTilesY + y - 1];
        upTile->options.erase(std::remove_if(upTile->options.begin(), upTile->options.end(), [tile, this](int option) {
            return std::find(this->m_rules[option].down.begin(), this->m_rules[option].down.end(), tile->options[0]) == this->m_rules[option].down.end();
        }), upTile->options.end());
    }


    WaitTime(0.01f);
    int nextIndex = this->GetIndexCrossEntropy();
    if(nextIndex == INT_MAX) {
        return;
    }
    this->Collapse(nextIndex);
}

int App::GetIndexCrossEntropy() {
    // get the minimum entropy
    int minEntropy = INT_MAX;
    int minIndex = 0;
    for(int i = 0; i < this->m_numTilesX * this->m_numTilesY; i++) {
        auto tile = this->m_grid[i];
        if(!tile->collapsed) {
            int entropy = tile->options.size();
            if(entropy < minEntropy) {
                minEntropy = entropy;
                minIndex = i;
            }
        }
    }

    return minIndex;
}
