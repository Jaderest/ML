#pragma once

#include "Mipmap.h"
#include "Texture.h"
#include <CoreLayer/ColorSpace/Spectrum.h>

#define SIZE 5

class WoodTexture : public Texture<Spectrum> { // 继承texture类
public:
  WoodTexture() = delete;

  WoodTexture(const Json &json);
  virtual Spectrum evaluate(const Intersection &intersection) const override;
  virtual Spectrum evaluate(const TextureCoord &texCoord) const override;

  float perlinNoise(Vector2f uv) const;

private:
  // necessary data
  float grid[SIZE][SIZE];
};