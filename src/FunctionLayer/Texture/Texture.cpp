#include "Texture.h"

TextureCoord UVMapping::map(const Intersection &intersection) const {
  // Point3f hitpoint = intersection.position;
  // Vector3f p = normalize(hitpoint - Point3f{0.f, 0.f, 0.f}); //这里的p是单位向量
  float u = intersection.texCoord[0];
  float v = intersection.texCoord[1];

  // float phi = acos(p[2]);
  // float theta = atan(p[1]/p[0]);
  // u = 2 * theta / (2 * M_PI) + 0.5; //注意值域问题，前面这个系数就是映射用的
  // if (u < 0)
  //   u += 0.5;
  // v = phi / M_PI;

  // float theta = atan(p[1]/p[0]);
  // u = 2 * theta / (2 * M_PI) + 0.5;
  // if (p[2] >= 0)
  //   v = p[2];
  // else
  //   v = p[2] + 1;

  // Vector3f alpha = {1, 0, 0};
  // Vector3f beta = {0, 1, 0};

  // p = hitpoint - Point3f{0.f, 0.f, 0.f};
  // u = dot(p, alpha);
  // v = dot(p, beta);
  // u = u - floor(u);
  // v = v - floor(v);

  assert(u >= 0);
  assert(u <= 1);
  assert(v >= 0);
  assert(v <= 1);
  return TextureCoord{Vector2f{u, v},
                      Vector2f{intersection.dudx, intersection.dvdx},
                      Vector2f{intersection.dudy, intersection.dvdy}};




  // return TextureCoord{intersection.texCoord,
  //                     Vector2f{intersection.dudx, intersection.dvdx},
  //                     Vector2f{intersection.dudy, intersection.dvdy}};
}