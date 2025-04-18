#include "Triangle.h"
#include <FunctionLayer/Acceleration/Linear.h>
//--- Triangle ---
Triangle::Triangle(int _primID, int _vtx0Idx, int _vtx1Idx, int _vtx2Idx,
                   const TriangleMesh *_mesh)
    : primID(_primID), vtx0Idx(_vtx0Idx), vtx1Idx(_vtx1Idx), vtx2Idx(_vtx2Idx),
      mesh(_mesh) {
  Point3f vtx0 = mesh->transform.toWorld(mesh->meshData->vertexBuffer[vtx0Idx]),
          vtx1 = mesh->transform.toWorld(mesh->meshData->vertexBuffer[vtx1Idx]),
          vtx2 = mesh->transform.toWorld(mesh->meshData->vertexBuffer[vtx2Idx]);
  boundingBox.Expand(vtx0);
  boundingBox.Expand(vtx1);
  boundingBox.Expand(vtx2);
  this->geometryID = mesh->geometryID;
}

bool Triangle::rayIntersectShape(Ray &ray, int *primID, float *u,
                                 float *v) const {
  Point3f origin = ray.origin;
  Vector3f direction = ray.direction;
  Point3f vtx0 = mesh->transform.toWorld(mesh->meshData->vertexBuffer[vtx0Idx]),
          vtx1 = mesh->transform.toWorld(mesh->meshData->vertexBuffer[vtx1Idx]),
          vtx2 = mesh->transform.toWorld(mesh->meshData->vertexBuffer[vtx2Idx]);

  Vector3f edge0 = vtx1 - vtx0, edge1 = vtx2 - vtx0;

  Vector3f paralNormal = normalize(cross(edge0, edge1));
  float d = -dot(paralNormal, Vector3f{vtx0[0], vtx0[1], vtx0[2]});
  float a = dot(paralNormal, Vector3f{origin[0], origin[1], origin[2]}) + d;
  float b = dot(paralNormal, direction);
  if (b == .0f)
    return false; // miss
  float t = -a / b;

  if (t < ray.tNear || t > ray.tFar)
    return false;

  Point3f hitpoint = origin + t * direction;
  // hitpoint = vtx0 + u * e0 + v * e1, 0 <= u, v <= 1
  Vector3f v1 = cross(hitpoint - vtx0, edge1), v2 = cross(edge0, edge1);
  float u_ = v1.length() / v2.length();
  if (dot(v1, v2) < 0)
    u_ *= -1;

  v1 = cross(hitpoint - vtx0, edge0);
  v2 = cross(edge1, edge0);
  float v_ = v1.length() / v2.length();
  if (dot(v1, v2) < 0)
    v_ *= -1;

  if (u_ >= .0f && v_ >= .0f && (u_ + v_ <= 1.f)) {
    ray.tFar = t;
    *primID = this->primID;
    *u = u_;
    *v = v_;
    return true;
  }

  return false;
}

void Triangle::fillIntersection(float distance, int primID, float u, float v,
                                Intersection *intersection) const {
  // 该函数实际上不会被调用
  return;
}

//--- TriangleMesh ---
TriangleMesh::TriangleMesh(const Json &json) : Shape(json) {
  const auto &filepath = fetchRequired<std::string>(json, "file");
  meshData = MeshData::loadFromFile(filepath);
}

float TriangleMesh::getArea() const {
  float area = 0.f;
  for (int i = 0; i < meshData->faceCount; ++i) {
    auto faceInfo = meshData->faceBuffer[i];
    // 转到世界尺度保证缩放合适
    Point3f vtx0 = transform.toWorld(meshData->vertexBuffer[faceInfo[0].vertexIndex]),
            vtx1 = transform.toWorld(meshData->vertexBuffer[faceInfo[1].vertexIndex]),
            vtx2 = transform.toWorld(meshData->vertexBuffer[faceInfo[2].vertexIndex]);
    area += cross(vtx1 - vtx0, vtx2 - vtx0).length() / 2.f;
  }
  return area;
}

std::vector<float> TriangleMesh::getArea(int t) const {
  std::vector<float> cu_areas(meshData->faceCount, .0f); // cumulative areas，单调递增的面积序列
  for (int i = 0; i < meshData->faceCount; ++i) {
    auto faceInfo = meshData->faceBuffer[i];
    // 转到世界尺度保证缩放合适
    Point3f vtx0 = transform.toWorld(meshData->vertexBuffer[faceInfo[0].vertexIndex]),
            vtx1 = transform.toWorld(meshData->vertexBuffer[faceInfo[1].vertexIndex]),
            vtx2 = transform.toWorld(meshData->vertexBuffer[faceInfo[2].vertexIndex]);
    if (i == 0) {
      cu_areas[i] = cross(vtx1 - vtx0, vtx2 - vtx0).length() / 2.f;
      continue;
    } else {
      cu_areas[i] = cu_areas[i - 1] + cross(vtx1 - vtx0, vtx2 - vtx0).length() / 2.f;
    }
  }
  return cu_areas;
}

RTCGeometry TriangleMesh::getEmbreeGeometry(RTCDevice device) const {
  RTCGeometry geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

  float *vertexBuffer = (float *)rtcSetNewGeometryBuffer(
      geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float),
      meshData->vertexCount);
  for (int i = 0; i < meshData->vertexCount; ++i) {
    Point3f vertex = transform.toWorld(meshData->vertexBuffer[i]);
    vertexBuffer[3 * i] = vertex[0];
    vertexBuffer[3 * i + 1] = vertex[1];
    vertexBuffer[3 * i + 2] = vertex[2];
  }

  unsigned *indexBuffer = (unsigned *)rtcSetNewGeometryBuffer(
      geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
      3 * sizeof(unsigned), meshData->faceCount);
  for (int i = 0; i < meshData->faceCount; ++i) {
    indexBuffer[i * 3] = meshData->faceBuffer[i][0].vertexIndex;
    indexBuffer[i * 3 + 1] = meshData->faceBuffer[i][1].vertexIndex;
    indexBuffer[i * 3 + 2] = meshData->faceBuffer[i][2].vertexIndex;
  }
  rtcCommitGeometry(geometry);
  return geometry;
}

bool TriangleMesh::rayIntersectShape(Ray &ray, int *primID, float *u,
                                     float *v) const {
  //* 当使用embree加速时，该方法不会被调用
  int geomID = -1;
  return acceleration->rayIntersect(ray, &geomID, primID, u, v);
}

void TriangleMesh::fillIntersection(float distance, int primID, float u,
                                    float v, Intersection *intersection) const {
  intersection->distance = distance;
  intersection->shape = this;
  //* 在三角形内部用插值计算交点、法线以及纹理坐标
  auto faceInfo = meshData->faceBuffer[primID];
  float w = 1.f - u - v;

  //* 计算交点。这里得到三个顶点的世界坐标系
  Point3f pw = transform.toWorld(
              meshData->vertexBuffer[faceInfo[0].vertexIndex]),
          pu = transform.toWorld(
              meshData->vertexBuffer[faceInfo[1].vertexIndex]),
          pv = transform.toWorld(
              meshData->vertexBuffer[faceInfo[2].vertexIndex]);
  intersection->position = Point3f{w * pw[0] + u * pu[0] + v * pv[0],
                                   w * pw[1] + u * pu[1] + v * pv[1],
                                   w * pw[2] + u * pu[2] + v * pv[2]};
  //* 计算法线
  if (meshData->normalBuffer.size() != 0) {
    Vector3f nw = transform.toWorld(
                 meshData->normalBuffer[faceInfo[0].normalIndex]),
             nu = transform.toWorld(
                 meshData->normalBuffer[faceInfo[1].normalIndex]),
             nv = transform.toWorld(
                 meshData->normalBuffer[faceInfo[2].normalIndex]);
    intersection->normal = normalize(w * nw + u * nu + v * nv);
  } else {
    intersection->normal = normalize(cross(pu - pw, pv - pw));
  }

  //* 计算纹理坐标
  Vector2f tw = meshData->texcodBuffer[faceInfo[0].texcodIndex],  // 得到了三个顶点的纹理坐标
            tu = meshData->texcodBuffer[faceInfo[1].texcodIndex],
            tv = meshData->texcodBuffer[faceInfo[2].texcodIndex];
  if (meshData->texcodBuffer.size() != 0) {
    intersection->texCoord = w * tw + u * tu + v * tv;
  } else {
    intersection->texCoord = Vector2f{.0f, .0f};
  }

  // TODO 计算交点的切线和副切线
  Vector3f tangent{1.f, 0.f, .0f};
  Vector3f bitangent;
  if (std::abs(dot(tangent, intersection->normal)) > .9f) {
    tangent = Vector3f(.0f, 1.f, .0f);
  }
  bitangent = normalize(cross(tangent, intersection->normal));
  tangent = normalize(cross(intersection->normal, bitangent));
  intersection->tangent = tangent;
  intersection->bitangent = bitangent;

  // TODO 填写dpdu和dpdv
  float A[2][2] = {
    {tu[0] - tw[0], tu[1] - tw[1]},
    {tv[0] - tw[0], tv[1] - tw[1]}
  };
  float Bx[2] = {pu[0] - pw[0], pv[0] - pw[0]};
  float By[2] = {pu[1] - pw[1], pv[1] - pw[1]};
  float Bz[2] = {pu[2] - pw[2], pv[2] - pw[2]};
  auto solveLinearSystem2x2 = [](const float A[2][2], const float B[2],
                                 float *x0, float *x1) {
    float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
    if (std::abs(det) < 1e-10f)
      return false;
    *x0 = (A[1][1] * B[0] - A[0][1] * B[1]) / det;
    *x1 = (A[0][0] * B[1] - A[1][0] * B[0]) / det;
    if (std::isnan(*x0) || std::isnan(*x1))
      return false;
    return true;
  };
  Vector3f dpdu, dpdv;
  if (!solveLinearSystem2x2(A, Bx, &dpdu[0], &dpdv[0]))
    dpdu = Vector3f{.0f, .0f, .0f};
  if (!solveLinearSystem2x2(A, By, &dpdv[1], &dpdv[1]))
    dpdv = Vector3f{.0f, .0f, .0f};
  if (!solveLinearSystem2x2(A, Bz, &dpdu[2], &dpdv[2]))
    dpdu = Vector3f{.0f, .0f, .0f};
  intersection->dpdu = dpdu;
  intersection->dpdv = dpdv;
}

void TriangleMesh::initInternalAcceleration() {
  acceleration = Acceleration::createAcceleration();
  int primCount = meshData->faceCount;
  for (int primID = 0; primID < primCount; ++primID) {
    int vtx0Idx = meshData->faceBuffer[primID][0].vertexIndex,
        vtx1Idx = meshData->faceBuffer[primID][1].vertexIndex,
        vtx2Idx = meshData->faceBuffer[primID][2].vertexIndex;
    std::shared_ptr<Triangle> triangle =
        std::make_shared<Triangle>(primID, vtx0Idx, vtx1Idx, vtx2Idx, this);
    acceleration->attachShape(triangle);
  }
  acceleration->build();
  // TriangleMesh的包围盒就是其内部加速结构的包围盒
  boundingBox = acceleration->boundingBox;
}
REGISTER_CLASS(TriangleMesh, "triangle")