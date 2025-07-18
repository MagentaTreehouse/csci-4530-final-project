#ifndef _CYLINDER_RING_H_
#define _CYLINDER_RING_H_

#include <cassert>
#include "primitive.h"
#include "vectors.h"

// ====================================================================
// ====================================================================
// Simple implicit repesentation of a ring, that can also be
// rasterized for use in radiosity.

class CylinderRing : public Primitive {

public:

  // CONSTRUCTOR & DESTRUCTOR
  CylinderRing(const Vec3f &c, float h, float i_r, float o_r, Material *m) {
    center = c; height = h; inner_radius = i_r; outer_radius = o_r; material = m;
    assert (height > 0);
    assert (inner_radius > 0);
    assert (outer_radius > inner_radius); }
  ~CylinderRing() {}

  // for ray tracing
  [[nodiscard]] bool intersect(const Ray &r, Hit &h) const;

  // for OpenGL rendering & radiosity
  void addRasterizedFaces(Mesh *m, ArgParser *args);

private:

  // REPRESENTATION
  Vec3f center;
  float height;
  float inner_radius;
  float outer_radius;
};

// ====================================================================
// ====================================================================

#endif
