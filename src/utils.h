#ifndef _UTILS_H
#define _UTILS_H

#include "vectors.h"
#include "argparser.h"

// needed by Windows
// allows us to use std::min & std::max
#ifndef NOMINMAX
  #define NOMINMAX
#endif


// =========================================================================
// EPSILON is a necessary evil for raytracing implementations
// The appropriate value for epsilon depends on the precision of
// the floating point calculations on your hardware **AND** on the
// overall dimensions of the scene and your camera projection matrix.
constexpr auto EPSILON{0.0001};


// =========================================================================
// These two functions convert between linear intensity values
// (approximate range 0->1) to an sRGB value (approximate range 0->1).
// The sRGB values make best use of 8 bit storage and are the best
// input for most displays and darkened viewing environments.

constexpr auto SRGB_ALPHA{0.055};

inline float linear_to_srgb(float x) {
  return x <= 0.0031308f?
    12.92f*x :
    (1+SRGB_ALPHA)*(pow(x,1/2.4)-SRGB_ALPHA);
}

inline float srgb_to_linear(float x) {
  return x <= 0.04045f?
    x/12.92f :
    static_cast<float>(pow((x+SRGB_ALPHA)/(1+SRGB_ALPHA),2.4));
}

// =========================================================================
// utility functions 
inline float DistanceBetweenTwoPoints(const Vec3f &p1, const Vec3f &p2) {
  return static_cast<float>((p1-p2).Length());
}

inline float AreaOfTriangle(float a, float b, float c) {
  // from the lengths of the 3 edges, compute the area
  // Area of Triangle = (using Heron's Formula)
  //  sqrt[s*(s-a)*(s-b)*(s-c)]
  //    where s = (a+b+c)/2
  // also... Area of Triangle = 0.5 * x * c
  const float s = (a+b+c) / 2;
  return sqrt(s*(s-a)*(s-b)*(s-c));
}

inline float AreaOfTriangle(const Vec3f &a, const Vec3f &b, const Vec3f &c) {
  return AreaOfTriangle(
    DistanceBetweenTwoPoints(a,b),
    DistanceBetweenTwoPoints(b,c),
    DistanceBetweenTwoPoints(c,a)
  );
}

inline Vec3f ComputeNormal(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3) {
  Vec3f v12 = p2;
  v12 -= p1;
  Vec3f v23 = p3;
  v23 -= p2;
  Vec3f normal;
  Vec3f::Cross3(normal,v12,v23);
  normal.Normalize();
  return normal;
}

// utility function to generate random numbers used for sampling
inline Vec3f RandomUnitVector() {
  Vec3f tmp;
  while (true) {
    tmp = {
      2*ArgParser::rand()-1,  // random real in [-1,1]
      2*ArgParser::rand()-1,  // random real in [-1,1]
      2*ArgParser::rand()-1   // random real in [-1,1]
    };
    if (tmp.Length() < 1) break;
  }
  tmp.Normalize();
  return tmp;
}

// compute the perfect mirror direction
constexpr Vec3f Reflection(const Vec3f &incoming, const Vec3f &normal) {
  return incoming - incoming.Dot3(normal) * 2 * normal;
}

// compute a random diffuse direction
// (not the same as a uniform random direction on the hemisphere)
inline Vec3f RandomDiffuseDirection(const Vec3f &normal) {
  return (normal+RandomUnitVector()).Normalized();
}

void AddWireFrameTriangle(float* &current,
                          const Vec3f &apos, const Vec3f &bpos, const Vec3f &cpos,
                          const Vec3f &anormal, const Vec3f &bnormal, const Vec3f &cnormal,
                          const Vec3f &color,
                          const Vec3f &abcolor, const Vec3f &bccolor, const Vec3f &cacolor);

void AddQuad(float* &current,
             const Vec3f &apos, const Vec3f &bpos, const Vec3f &cpos, const Vec3f &dpos,
             const Vec3f &normal,
             const Vec3f &color);


void AddBox(float* &current,
            const Vec3f pos[8],
            const Vec3f &color);


#endif
