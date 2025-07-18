#include "material.h"
#include "utils.h"
#include "ray.h"
#include "hit.h"

// ==================================================================
// DESTRUCTOR
// ==================================================================
Material::~Material() {
  if (hasTextureMap()) {
    //glDeleteTextures(1,&texture_id);
    assert (image != nullptr);
    delete image;
  }
}

// ==================================================================
// TEXTURE LOOKUP FOR DIFFUSE COLOR
// ==================================================================
Vec3f Material::getDiffuseColor(float s, float t) const {
  if (!hasTextureMap()) return diffuseColor; 

  assert (image != nullptr);

  // this is just using nearest neighbor and could be improved to
  // bilinear interpolation, etc.
  int i = int(s * image->Width()) % image->Width();
  int j = int(t * image->Height()) % image->Height();
  if (i < 0) i += image->Width();
  if (j < 0) j += image->Height();
  assert (i >= 0 && i < image->Width());
  assert (j >= 0 && j < image->Height());
  Color c = image->GetPixel(i,j);

  // we assume the texture is stored in sRGB and convert to linear for
  // computation.  It will be converted back to sRGB before display.
  const float r = srgb_to_linear(c.r/255.0);
  const float g = srgb_to_linear(c.g/255.0);
  const float b = srgb_to_linear(c.b/255.0);

  return {r,g,b};
}

/*
// ==================================================================
// OpenGL setup for textures
// ==================================================================
GLuint Material::getTextureID() { 
  assert (hasTextureMap()); 

  // if this is the first time the texture is being used, we must
  // initialize it
  if (texture_id == 0)  {
    glGenTextures(1,&texture_id);
    assert (texture_id != 0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    // select modulate to mix texture with color for shading
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    // or decal to not mix local shading
    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		     GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the original
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    // the texture wraps over at the edges (repeat)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    // to be most compatible, textures should be square and a power of 2
    assert (image->Width() == image->Height());
    assert (image->Width() == 256);
    // build our texture mipmaps
    //gluBuild2DMipmaps( GL_TEXTURE_2D, 3, image->Width(), image->Height(),
    //		       GL_RGB, GL_UNSIGNED_BYTE, image->getGLPixelData());
  }
  
  return texture_id;
}
*/

// ==================================================================
// An average texture color, a hack for use in radiosity
// ==================================================================
void Material::ComputeAverageTextureColor() {
  assert (hasTextureMap());
  float r = 0;
  float g = 0;
  float b = 0;
  for (int i = 0; i < image->Width(); i++) {
    for (int j = 0; j < image->Height(); j++) {
      Color c = image->GetPixel(i,j);
       r += srgb_to_linear(c.r/255.0);
       g += srgb_to_linear(c.g/255.0);
       b += srgb_to_linear(c.b/255.0);
    }
  }
  int count = image->Width() * image->Height();
  r /= count;
  g /= count;
  b /= count;
  diffuseColor = {r,g,b};
}

Vec3f Material::brdf(const Hit &hit, const Vec3f &in, const Vec3f &out) const {
  Vec3f answer{.5 / M_PI * getDiffuseColor(hit.get_s(), hit.get_t())};
  if (getReflectiveColor() == Vec3f{} || getRoughness() == 0)
    return answer;
  const double p{std::pow(1 / getRoughness() - 1, 2)};
  const auto glossyBias{std::pow(std::cos(
    std::acos(Reflection(in, hit.getNormal()).Dot3(out.Normalized())) / 2
  ), p)};
  // normalization = pi / (sqrt(pi) * ...)
  // 1 / (2pi) * glossyBias * normalization * reflectiveColor
  // cancel out the pi's
  const auto normalization = 1 / (std::sqrt(M_PI) * std::tgamma((p + 1) / 2) / (2 * std::tgamma(p / 2 + 1)));
  return answer + .5 * glossyBias * normalization * getReflectiveColor();
}

// ==================================================================
// PHONG LOCAL ILLUMINATION

// this function should be called to compute the light contributed by
// a particular light source to the intersection point.  Note that
// this function does not calculate any global effects (e.g., shadows). 

Vec3f Material::Shade(const Ray &ray, const Hit &hit, 
                      const Vec3f &dirToLight, 
                      const Vec3f &lightColor) const {
  
  const Vec3f &n = hit.getNormal();
  Vec3f e = ray.getDirection()*-1.0f;
  Vec3f l = dirToLight;

  Vec3f answer{};

  // emitted component
  // -----------------
  answer += getEmittedColor();

  // diffuse component
  // -----------------
  float dot_nl = n.Dot3(l);
  if (dot_nl < 0) dot_nl = 0;
  answer += lightColor * getDiffuseColor(hit.get_s(),hit.get_t()) * dot_nl;
  if (reflectiveColor == Vec3f{})
    return answer;

  // specular component (Phong)
  // ------------------
  // make up reasonable values for other Phong parameters
  const Vec3f &specularColor = reflectiveColor;
  float exponent = 100;

  // compute ideal reflection angle
  const Vec3f r = ((l*-1.0f) + n * (2 * dot_nl)).Normalized();
  float dot_er = e.Dot3(r);
  if (dot_er < 0) dot_er = 0;
  answer += lightColor*specularColor*float(pow(dot_er,exponent))* dot_nl;

  return answer;
}

// ==================================================================
