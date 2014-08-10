#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>

struct vec2 {
  float x,y;
};

struct vec3 {
  float x,y,z;
};

struct vec4 {
  float x,y,z,w;
};

struct BoundingBox {
  vec3 emin, emax;
};

struct Mesh {
  std::vector<vec3> vertices;
  std::vector<vec2> uvs;
  std::vector<vec3> normals;
  std::vector<uint32_t> indices;

  BoundingBox bb;
  vec3 ObjCenter;
  vec3 ObjSize;
  bool loadOBJ(const char * path);
};

#endif
