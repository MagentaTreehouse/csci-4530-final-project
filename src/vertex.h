#ifndef _VERTEX_H
#define _VERTEX_H

#include "vectors.h"

// ==========================================================

class Vertex {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Vertex(int i, const Vec3f &pos): position{pos}, s{}, t{}, index{i} {}

  // =========
  // ACCESSORS
  [[nodiscard]] int getIndex() const { return index; }
  [[nodiscard]] const Vec3f& get() const { return position; }
  [[nodiscard]] float get_s() const { return s; }
  [[nodiscard]] float get_t() const { return t; }

  // =========
  // MODIFIERS
  void setTextureCoordinates(float _s, float _t) { s = _s; t = _t; }

private:

  // ==============
  // REPRESENTATION
  Vec3f position;

  // texture coordinates
  // NOTE: arguably these should be stored at the faces of the mesh
  // rather than the vertices
  float s,t;

  // this is the index from the original .obj file.
  // technically not part of the half-edge data structure
  int index;  

  // NOTE: the vertices don't know anything about adjacency.  In some
  // versions of this data structure they have a pointer to one of
  // their incoming edges.  However, this data is complicated to
  // maintain during mesh manipulation.
};

// ==========================================================

#endif
