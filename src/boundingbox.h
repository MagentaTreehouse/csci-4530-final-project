#ifndef _BOUNDING_BOX_H_
#define _BOUNDING_BOX_H_

#include <cassert>
#include <algorithm>
#include "vectors.h"


// ====================================================================
// ====================================================================

class BoundingBox {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  BoundingBox() { 
    Set({0,0,0},{0,0,0}); }
  BoundingBox(const Vec3f &pt) {
    Set(pt,pt); }
  BoundingBox(const Vec3f &_minimum, const Vec3f &_maximum) { 
    Set(_minimum,_maximum); }

  // =========
  // ACCESSORS
  void Get(Vec3f &_minimum, Vec3f &_maximum) const {
    _minimum = minimum;
    _maximum = maximum; }
  const Vec3f& getMin() const { return minimum; }
  const Vec3f& getMax() const { return maximum; }
  void getCenter(Vec3f &c) const {
    c = maximum; 
    c -= minimum;
    c *= 0.5f;
    c += minimum;
  }
  double maxDim() const {
    double x = maximum.x() - minimum.x();
    double y = maximum.y() - minimum.y();
    double z = maximum.z() - minimum.z();
    return std::max({x, y, z});
  }

  // =========
  // MODIFIERS
  void Set(const BoundingBox &bb) {
    minimum = bb.minimum;
    maximum = bb.maximum; }
  void Set(const Vec3f &_minimum, const Vec3f &_maximum) {
    assert (minimum.x() <= maximum.x() &&
	    minimum.y() <= maximum.y() &&
	    minimum.z() <= maximum.z());
    minimum = _minimum;
    maximum = _maximum; }
  void Extend(const Vec3f v) {
    minimum = {
      std::min(minimum.x(),v.x()),
      std::min(minimum.y(),v.y()),
      std::min(minimum.z(),v.z())
    };
    maximum = {
      std::max(maximum.x(),v.x()),
      std::max(maximum.y(),v.y()),
      std::max(maximum.z(),v.z())
    };
  }
  void Extend(const BoundingBox &bb) {
    Extend(bb.minimum);
    Extend(bb.maximum); 
  }

private:

  // ==============
  // REPRESENTATION
  Vec3f minimum;
  Vec3f maximum;
};

// ====================================================================
// ====================================================================

#endif
