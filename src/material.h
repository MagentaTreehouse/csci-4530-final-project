#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <cassert>
#include <string>

#include "vectors.h"
#include "image.h"

class Ray;
class Hit;

// ====================================================================
// ====================================================================
// A simple Phong-like material 

class Material {

public:

  Material(const std::string &texture_file, const Vec3f &d_color,
	   const Vec3f &r_color, const Vec3f &e_color, float roughness_) {
    textureFile = texture_file;
    if (textureFile != "") {
      image = new Image(textureFile);
      ComputeAverageTextureColor();
    } else {
      diffuseColor = d_color;
      image = nullptr;
    }
    reflectiveColor = r_color;
    emittedColor = e_color;
    roughness = roughness_;
    // need to initialize texture_id after glut has started
    //texture_id = 0;
  }

  ~Material();

  // ACCESSORS
  [[nodiscard]] const Vec3f& getDiffuseColor() const { return diffuseColor; }
  [[nodiscard]] Vec3f getDiffuseColor(float s, float t) const;
  [[nodiscard]] const Vec3f& getReflectiveColor() const { return reflectiveColor; }
  [[nodiscard]] const Vec3f& getEmittedColor() const { return emittedColor; }  
  [[nodiscard]] float getRoughness() const { return roughness; } 
  [[nodiscard]] bool hasTextureMap() const { return textureFile != ""; }
  [[nodiscard]] bool isEmitting(double x = .001) const {
    return getEmittedColor().Length() > x;
  }

  // in should be normalized, out does not have to be
  Vec3f brdf(const Hit &hit, const Vec3f &in, const Vec3f &out) const;

  // SHADE
  // compute the contribution to local illumination at this point for
  // a particular light source
  Vec3f Shade(const Ray &ray, const Hit &hit, const Vec3f &dirToLight, 
   const Vec3f &lightColor) const;

protected:

  Material() = delete;
  Material(const Material&) = delete;
  const Material& operator=(const Material&) = delete;

  void ComputeAverageTextureColor();

  // REPRESENTATION
  Vec3f diffuseColor;
  Vec3f reflectiveColor;
  Vec3f emittedColor;
  float roughness;

  std::string textureFile;
  //GLuint texture_id;
  Image *image;
};

// ====================================================================
// ====================================================================

#endif
