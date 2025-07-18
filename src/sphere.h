#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "primitive.h"

// ====================================================================
// ====================================================================
// Simple implicit repesentation of a sphere, that can also be
// rasterized for use in radiosity.

class Sphere : public Primitive {

public:
  // CONSTRUCTOR & DESTRUCTOR
  Sphere(const Vec3f &c, float r, Material *m) {
    center = c; radius = r; material = m;
    assert (radius >= 0); }

  // for ray tracing
  [[nodiscard]] virtual bool intersect(const Ray &r, Hit &h) const;

  // for OpenGL rendering & radiosity
  void addRasterizedFaces(Mesh *m, ArgParser *args);

private:

  // REPRESENTATION
  Vec3f center;
  float radius;
};

// ====================================================================
// ====================================================================

#endif
