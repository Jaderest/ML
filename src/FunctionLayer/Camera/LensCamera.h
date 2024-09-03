#pragma once

#include "Camera.h"
class LensCamera : public PerspectiveCamera {
public:
  LensCamera() = delete;

  LensCamera(const Json &json);

  virtual Ray sampleRay(const CameraSample &sample,
                        Vector2f NDC) const override;

  virtual Ray sampleRayDifferentials(const CameraSample &sample,
                                     Vector2f NDC) const override;
};