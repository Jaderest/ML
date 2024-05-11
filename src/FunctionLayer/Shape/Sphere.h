#pragma once
#include "Shape.h"
// TODO 当前只有Transform中的translate对sphere生效
class Sphere : public Shape {
public:
  Sphere() = delete;

  Sphere(const Json &json);

  virtual bool rayIntersectShape(Ray &ray, int *primID, float *u,
                                 float *v) const override;

  virtual void fillIntersection(float distance, int primID, float u, float v,
                                Intersection *intersection) const override;
  virtual void uniformSampleOnSurface(Vector2f sample,
                                      Intersection *intersection,
                                      float *pdf) const override {
    // TODO finish this
    float phi = 2 * PI * sample[0];
    float theta = acos(1 - 2 * sample[1]);

    intersection->position = center + radius * Vector3f{sin(theta) * cos(phi),
                                                        sin(theta) * sin(phi),
                                                        cos(theta)};
    intersection->normal = normalize(intersection->position - center);

    *pdf = 1.f / (4 * PI * radius * radius);
    return;
  }

public:
  Point3f center;
  float radius;
};