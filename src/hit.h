#ifndef _HIT_H_
#define _HIT_H_

#include <float.h>
#include <ostream>
#include <limits>

#include "ray.h"

class Material;

// Hit class mostly copied from Peter Shirley and Keith Morley
// ====================================================================
// ====================================================================

class Hit {

public:

  // CONSTRUCTOR & DESTRUCTOR
  Hit():
    t{std::numeric_limits<float>::max()},
    material{},
    normal{},
    texture_s{},
    texture_t{}
  {}

  // ACCESSORS
  [[nodiscard]] float getT() const { return t; }
  [[nodiscard]] Material* getMaterial() const { return material; }
  [[nodiscard]] const Vec3f &getNormal() const { return normal; }
  [[nodiscard]] float get_s() const { return texture_s; }
  [[nodiscard]] float get_t() const { return texture_t; }

  // MODIFIER
  void set(float _t, Material *m, Vec3f n) {
    t = _t; material = m; normal = n; 
    texture_s = 0; texture_t = 0; }

  void setTextureCoords(float t_s, float t_t) {
    texture_s = t_s; texture_t = t_t; 
  }

private: 

  // REPRESENTATION
  float t;
  Material *material;
  Vec3f normal;
  float texture_s, texture_t;
};

inline std::ostream &operator<<(std::ostream &os, const Hit &h) {
  os << "Hit <" << h.getT() << ", < "
     << h.getNormal().x() << "," 
     << h.getNormal().y() << "," 
     << h.getNormal().z() << " > > ";
  return os;
}
// ====================================================================
// ====================================================================

#endif
