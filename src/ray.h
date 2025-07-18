#ifndef _RAY_H
#define _RAY_H

#include <iostream>
#include "vectors.h"

// Ray class mostly copied from Peter Shirley and Keith Morley
// ====================================================================
// ====================================================================

class Ray {

public:
  // CONSTRUCTOR & DESTRUCTOR
  Ray(const Vec3f &orig, const Vec3f &dir):
    origin{orig}, direction{dir}
  {}

  // ACCESSORS
  [[nodiscard]] const Vec3f& getOrigin() const { return origin; }
  [[nodiscard]] const Vec3f& getDirection() const { return direction; }
  [[nodiscard]] Vec3f pointAtParameter(float t) const {
    return origin+direction*t; }

private:
  Ray() = delete;

  // REPRESENTATION
  Vec3f origin;
  Vec3f direction;
};

inline std::ostream &operator<<(std::ostream &os, const Ray &r) {
  os << "Ray < < " 
     << r.getOrigin().x() << "," 
     << r.getOrigin().y() << "," 
     << r.getOrigin().z() << " > " 
     <<", "
     << r.getDirection().x() << "," 
     << r.getDirection().y() << "," 
     << r.getDirection().z() << " > >";
  return os;
}

// ====================================================================
// ====================================================================

#endif
