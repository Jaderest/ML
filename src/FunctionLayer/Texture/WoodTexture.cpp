// (newly created) WoodTexture.cpp
#include "WoodTexture.h"
#include <ResourceLayer/Factory.h>

WoodTexture::WoodTexture(const Json &json) : Texture<Spectrum>() {
  // 给出的示例场景中，json文件并不包含额外的域；你可以自行添加
  srand(233);
  if (this->grid[0][0] == 0.f) {
    for (int i = 0; i < SIZE; ++i) {
      for (int j = 0; j < SIZE; ++j) {
        this->grid[i][j] = ((double)rand() / RAND_MAX) * 2 * M_PI;
        this->grid[i][j] = this->grid[i][j] * (i + j);
        while (this->grid[i][j] > 2 * M_PI) {
          this->grid[i][j] -= 2 * M_PI;
        }
      }
    }
  }
}

float WoodTexture::perlinNoise (Vector2f uv) const {
  Vector2f pos = Vector2f(uv[0] * SIZE, uv[1] * SIZE); // (0, 1) -> (0, size)
  int x0 = std::floor(pos[0]), x1 = x0 + 1;
  int y0 = std::floor(pos[1]), y1 = y0 + 1;
  Vector2f v0 = {cos(this->grid[x0][y1]), sin(this->grid[x0][y1])};
  Vector2f v1 = {cos(this->grid[x1][y1]), sin(this->grid[x1][y1])};
  Vector2f v2 = {cos(this->grid[x1][y0]), sin(this->grid[x1][y0])};
  Vector2f v3 = {cos(this->grid[x0][y0]), sin(this->grid[x0][y0])};
  Vector2f d0 = pos - Vector2f(x0, y1);
  Vector2f d1 = pos - Vector2f(x1, y1);
  Vector2f d2 = pos - Vector2f(x1, y0);
  Vector2f d3 = pos - Vector2f(x0, y0);
  float f0 = dot(v0, d0);
  float f1 = dot(v1, d1);
  float f2 = dot(v2, d2);
  float f3 = dot(v3, d3);
  float rx = (pos[0] - x0)/(x1 - x0);
  float ry = (pos[1] - y0)/(y1 - y0);
  
  float xlow = (1 - rx) * f3 + rx * f2;
  float xhigh = (1 - rx) * f0 + rx * f1;
  return (1 - ry) * xlow + ry * xhigh;
}

Spectrum WoodTexture::evaluate(const Intersection &intersection) const {
  TextureCoord texCoord = mapping->map(intersection);
  return evaluate(texCoord);
}

Spectrum WoodTexture::evaluate(const TextureCoord &texCoord) const {
  Vector2f uv = texCoord.coord;
  float f = perlinNoise(uv);
  Spectrum woodColor = SpectrumRGB(164.f / 255, 116.f / 255, 73.f / 255);
  float f1 = 10 * f - floor(10 * f);
  woodColor = woodColor * (6 * pow(f1, 5) - 15 * pow(f1, 4) + 10 * pow(f1, 3));
  return woodColor;
}

// 为了让json文件可以索引到WoodTexture，这一语句不可以删除
REGISTER_CLASS(WoodTexture, "woodTex")