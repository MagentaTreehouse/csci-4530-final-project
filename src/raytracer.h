#ifndef _RAY_TRACER_
#define _RAY_TRACER_

#include <vector>
#include <filesystem>
#include "ray.h"
#include "hit.h"

class Mesh;
class ArgParser;
class Radiosity;
class PhotonMapping;

struct Pixel {
  Vec3f v1,v2,v3,v4;
  Vec3f color;
};


// ====================================================================
// ====================================================================
// This class manages the ray casting and ray tracing work.

class RayTracer {
public:
  // CONSTRUCTOR & DESTRUCTOR
  RayTracer(Mesh *m, ArgParser *a): mesh{m}, args{a}, render_to_a{true} {}

  [[nodiscard]] std::size_t triCount() const;
  void packMesh(float* &current);
  void renderToFile(const std::filesystem::path &) const;
  template<bool Visualize = false> Vec3f renderPixel(double i, double j) const;
  int DrawPixel();

  // set access to the other modules for hybrid rendering options
  void setRadiosity(Radiosity *r) { radiosity = r; }
  void setPhotonMapping(PhotonMapping *pm) { photon_mapping = pm; }

  // casts a single ray through the scene geometry and finds the closest hit
  bool CastRay(const Ray &ray, Hit &h, bool use_sphere_patches) const;
  Vec3f TraceRay(const Ray &, Hit &, int depth = 0) const;

private:
  template<class F> Vec3f TraceRayImpl(const Ray &, Hit &, const Vec3f &ambientLt,
    int depth, F directIllum) const;
  void drawVBOs_a();
  void drawVBOs_b();

  // REPRESENTATION
  Mesh *mesh;
  ArgParser *args;
  Radiosity *radiosity;
  PhotonMapping *photon_mapping;

public:
  bool render_to_a;
  std::vector<Pixel> pixels_a;
  std::vector<Pixel> pixels_b;

};

// ====================================================================
// ====================================================================

Vec3f VisualizeTraceRay(double i, double j);

#endif
