#include <thread>
#include <chrono>
#include <type_traits>
#include "raytracer.h"
#include "material.h"
#include "raytree.h"
#include "utils.h"
#include "mesh.h"
#include "meshdata.h"
#include "face.h"
#include "primitive.h"
#include "photon_mapping.h"
#include "camera.h"
#include "image.h"


inline auto ToUnitSquare(std::tuple<double, double> p) {
  const auto &md{*GLOBAL_args->mesh_data};
  const int max_d{std::max(md.width,md.height)};
  return std::tuple{
    (std::get<0>(p) - md.width / 2.) / max_d + .5,
    (std::get<1>(p) - md.height / 2.) / max_d + .5
  };
}


// ===========================================================================
// casts a single ray through the scene geometry and finds the closest hit
bool RayTracer::CastRay(const Ray &ray, Hit &h, bool use_rasterized_patches) const {
  bool answer = false;

  // intersect each of the quads
  for (int i = 0; i < mesh->numOriginalQuads(); i++) {
    Face *f = mesh->getOriginalQuad(i);
    answer |= f->intersect(ray,h,args->mesh_data->intersect_backfacing);
  }

  // intersect each of the primitives (either the patches, or the original primitives)
  if (use_rasterized_patches) {
    for (int i = 0; i < mesh->numRasterizedPrimitiveFaces(); i++) {
      Face *f = mesh->getRasterizedPrimitiveFace(i);
      answer |= f->intersect(ray,h,args->mesh_data->intersect_backfacing);
    }
  } else {
    int num_primitives = mesh->numPrimitives();
    for (int i = 0; i < num_primitives; i++) {
      answer |= mesh->getPrimitive(i)->intersect(ray,h);
    }
  }
  return answer;
}


// does the recursive (shadow rays & recursive rays) work
template<class F, bool Visualize>
Vec3f RayTracer::TraceRayImpl(const Ray &ray, Hit &hit, const Vec3f &ambient, int depth, F directIllum, std::bool_constant<Visualize>) const {

  hit = {};
  // First cast a ray and see if we hit anything.
  // if there is no intersection, simply return the background color
  if (!CastRay(ray,hit,false)) {
    return {
      srgb_to_linear(mesh->background_color.r()),
      srgb_to_linear(mesh->background_color.g()),
      srgb_to_linear(mesh->background_color.b())
    };
  }

  // otherwise decide what to do based on the material
  const Material *m{hit.getMaterial()};
  assert (m != nullptr);

  // rays coming from the light source are set to white, don't bother to ray trace further.
  if (m->getEmittedColor().Length() > 0.001) {
    return {1,1,1};
  }

  const Vec3f &d{ray.getDirection()};
  const Vec3f &normal{hit.getNormal()};
  const Vec3f point{ray.pointAtParameter(hit.getT())};

  // ----------------------------------------------
  // start with the indirect light (ambient light)
  const Vec3f diffuse_color{m->getDiffuseColor(hit.get_s(),hit.get_t())};
  Vec3f answer{args->mesh_data->gather_indirect?
    diffuse_color * (photon_mapping->GatherIndirect(point, normal, d) + ambient) : // photon mapping for more accurate indirect light
    diffuse_color * ambient // the usual ray tracing hack for indirect light
  };

  // ----------------------------------------------
  // direct illumination
  for (const Face *f: mesh->getLights()) {
    const Vec3f ltColor{f->getMaterial()->getEmittedColor() * f->getArea()};
    // ===========================================
    // ASSIGNMENT:  ADD SHADOW & SOFT SHADOW LOGIC
    // ===========================================
    // add the lighting contribution from this particular light at this point
    answer += directIllum(*f, point,
      [&] (const Vec3f &ptLtSample) {
        const float dist = ptLtSample.Length();
        const Vec3f ltIrradiance{1 / (float(M_PI)*dist*dist) * ltColor};
        return m->Shade(ray, hit, ptLtSample.Normalized(), ltIrradiance);
      });
  }

  // ----------------------------------------------
  // add contribution from reflection, if the surface is shiny
  // =================================
  // ASSIGNMENT:  ADD REFLECTIVE LOGIC
  // =================================
  if (m->getReflectiveColor() != Vec3f{} && depth) {
    const Ray r{point, Reflection(d, normal)};
    Hit h{};
    answer +=
      m->getReflectiveColor() *
      TraceRayImpl<F, Visualize>(r, h, ambient, depth - 1, directIllum, {});
    if constexpr (Visualize) RayTree::AddReflectedSegment(r, 0, h.getT());
  }

  return answer;

}


template<bool Visualize>
Vec3f RayTracer::TraceRay(const Ray &ray, Hit &hit, int depth) const {
  const auto &md{*args->mesh_data};
  const Vec3f ambientLt{
    md.ambient_light[0],
    md.ambient_light[1],
    md.ambient_light[2]
  };

  // "shadow ray"
  auto directIllum{[&] (const Vec3f &pt, const Vec3f &ptLt, auto shadeLocal) {
    const Ray r{pt, ptLt};
    Hit block{};
    CastRay(r, block, false);
    if constexpr (Visualize) RayTree::AddShadowSegment(r, 0, block.getT());
    return block.getT() > 1 - EPSILON? shadeLocal(ptLt) : Vec3f{};
  }};

  auto vis{std::bool_constant<Visualize>{}};
  // The behavior of direct illumination calculation differs for certain values of shadow
  // samples and antialiasing samples
  switch (int sSamp{md.num_shadow_samples}; sSamp * md.num_antialias_samples) {
  case 0:
  return TraceRayImpl(ray, hit, ambientLt, depth,
    [] (const Face &f, const Vec3f &pt, auto shadeLocal) { // no shadows considered
      const auto ptLtC{f.computeCentroid() - pt};
      if constexpr (Visualize) RayTree::AddShadowSegment({pt, ptLtC}, 0, 1);
      return shadeLocal(ptLtC);
    }, vis);

  case 1:
  return TraceRayImpl(ray, hit, ambientLt, depth,
    [&] (const Face &f, const Vec3f &pt, auto shadeLocal) { // "decay" to hard shadows
      return directIllum(pt, f.computeCentroid() - pt, shadeLocal);
    }, vis);

  default:
  return TraceRayImpl(ray, hit, ambientLt, depth,
    [&] (const Face &f, const Vec3f &pt, auto shadeLocal) { // soft shadows
      Vec3f directIllumSum{};
      for (int i{}; i < sSamp; ++i)
        directIllumSum += directIllum(pt, f.RandomPoint() - pt, shadeLocal);
      return 1. / sSamp * directIllumSum;
    }, vis);
  }
}


// trace a ray through pixel (i,j) of the image an return the color
template<bool Visualize>
Vec3f RayTracer::renderPixel(double i, double j) const {

  // ==================================
  // ASSIGNMENT: IMPLEMENT ANTIALIASING
  // ==================================
  const auto &md{*args->mesh_data};
  const auto aa{static_cast<std::size_t>(std::sqrt(md.num_antialias_samples))};
  const auto
    ds{1. / aa},
    i0{i + ds / 2},
    j0{j + ds / 2};

  Vec3f colorSum{};
  for (std::size_t si{}; si < aa; ++si)
    for (std::size_t sj{}; sj < aa; ++sj) {
      // Here's what we do with a single sample per pixel:
      // construct & trace a ray through the center of the pixel
      const auto [x, y]{ToUnitSquare({i0 + ds * si, j0 + ds * sj})};
      const Ray r = args->mesh->camera->generateRay(x,y);
      Hit hit;
      colorSum += TraceRay<Visualize>(r, hit, md.num_bounces);
      // add that ray for visualization
      if constexpr (Visualize) RayTree::AddMainSegment(r, 0, hit.getT());
    }

  return 1. / (aa * aa) * colorSum;
}

Vec3f VisualizeTraceRay(double i, double j) {
  return GLOBAL_args->raytracer->renderPixel<true>(i - .5, j - .5);
}


// for visualization: find the "corners" of a pixel on an image plane
// 1/2 way between the camera & point of interest
Vec3f PixelGetPos(double i, double j) {
  const auto &cam = *GLOBAL_args->mesh->camera;
  const auto [x, y]{ToUnitSquare({i, j})};
  const Ray r = cam.generateRay(x,y);
  const Vec3f &cp = cam.camera_position;
  const Vec3f &poi = cam.point_of_interest;
  const float distance = DistanceBetweenTwoPoints(cp, poi)/2.0f;
  return r.getOrigin()+distance*r.getDirection();
}


// Scan through the image from the lower left corner across each row
// and then up to the top right.  Initially the image is sampled very
// coarsely.  Increment the static variables that track the progress
// through the scans
int RayTracer::DrawPixel() {
  auto &md{*args->mesh_data};
  if (md.raytracing_x >= md.raytracing_divs_x) {
    // end of row
    md.raytracing_x = 0;
    ++md.raytracing_y;
  }
  if (md.raytracing_y >= md.raytracing_divs_y) {
    // last row
    if (md.raytracing_divs_x >= md.width ||
        md.raytracing_divs_y >= md.height) {
      // stop rendering, matches resolution of current camera
      return 0;
    }
    // else decrease pixel size & start over again in the bottom left corner
    md.raytracing_divs_x *= 3;
    md.raytracing_divs_y *= 3;
    if (md.raytracing_divs_x > md.width * 0.51 ||
        md.raytracing_divs_x > md.height * 0.51) {
      md.raytracing_divs_x = md.width;
      md.raytracing_divs_y = md.height;
    }
    md.raytracing_x = 0;
    md.raytracing_y = 0;

    if (render_to_a) {
      pixels_b.clear();
      render_to_a = false;
    } else {
      pixels_a.clear();
      render_to_a = true;
    }
  }

  double x_spacing = md.width / double (md.raytracing_divs_x);
  double y_spacing = md.height / double (md.raytracing_divs_y);

  // compute the color and position of intersection
  Vec3f pos1 = PixelGetPos((md.raytracing_x  )*x_spacing, (md.raytracing_y  )*y_spacing);
  Vec3f pos2 = PixelGetPos((md.raytracing_x+1)*x_spacing, (md.raytracing_y  )*y_spacing);
  Vec3f pos3 = PixelGetPos((md.raytracing_x+1)*x_spacing, (md.raytracing_y+1)*y_spacing);
  Vec3f pos4 = PixelGetPos((md.raytracing_x  )*x_spacing, (md.raytracing_y+1)*y_spacing);

  Vec3f color = VisualizeTraceRay((md.raytracing_x+0.5)*x_spacing, (md.raytracing_y+0.5)*y_spacing);

  const double
    r = linear_to_srgb(color.r()),
    g = linear_to_srgb(color.g()),
    b = linear_to_srgb(color.b());

  Pixel p{pos1, pos2, pos3, pos4, {r,g,b}};

  if (render_to_a) {
    pixels_a.push_back(p);
  } else {
    pixels_b.push_back(p);
  }

  ++md.raytracing_x;
  return 1;
}

// ===========================================================================

std::size_t RayTracer::triCount() const {
  return (pixels_a.size() + pixels_b.size()) * 2;
}

void RayTracer::packMesh(float* &current) {
  for (const auto &p: pixels_a) {
    Vec3f v1 = p.v1;
    Vec3f v2 = p.v2;
    Vec3f v3 = p.v3;
    Vec3f v4 = p.v4;
    const Vec3f normal = (ComputeNormal(v1,v2,v3) + ComputeNormal(v1,v3,v4)).Normalized();
    if (render_to_a) {
      v1 += 0.02*normal;
      v2 += 0.02*normal;
      v3 += 0.02*normal;
      v4 += 0.02*normal;
    }
    AddQuad(current,v1,v2,v3,v4,{},p.color);
  }

  for (const auto &p: pixels_b) {
    Vec3f v1 = p.v1;
    Vec3f v2 = p.v2;
    Vec3f v3 = p.v3;
    Vec3f v4 = p.v4;
    const Vec3f normal = (ComputeNormal(v1,v2,v3) + ComputeNormal(v1,v3,v4)).Normalized();
    if (!render_to_a) {
      v1 += 0.02*normal;
      v2 += 0.02*normal;
      v3 += 0.02*normal;
      v4 += 0.02*normal;
    }
    AddQuad(current,v1,v2,v3,v4,{},p.color);
  }
}


void RayTracer::renderToFile(const std::filesystem::path &fPath) const {
  std::cout << "Starting raytracing render..." << std::endl;
  Image img{args->mesh_data->width, args->mesh_data->height};

  auto renderBlock{[&] (std::tuple<int, int> wRange, std::tuple<int, int> hRange) {
    const auto [wStart, wEnd]{wRange};
    const auto [hStart, hEnd]{hRange};
    for (int i{wStart}; i < wEnd; ++i)
      for (int j{hStart}; j < hEnd; ++j) {
        const auto p{renderPixel(i, j)};
        auto toUint8{[] (float x) -> std::uint8_t {
          return std::round(255 * std::min(linear_to_srgb(x), 1.f));
        }};
        img.SetPixel(i, j, {toUint8(p.r()), toUint8(p.g()), toUint8(p.b())});
      }
  }};
  static constexpr int blockSize{128};
  using namespace std::chrono;

  auto tStart{steady_clock::now()};
  std::vector<std::thread> ts;
  ts.reserve(std::ceil(1. * img.Width() / blockSize) * std::ceil(1. * img.Height() / blockSize));
  for (int i{}; i < img.Width(); i += blockSize)
    for (int j{}; j < img.Height(); j += blockSize)
      ts.emplace_back([&, i, j] () {renderBlock(
        {i, std::min(i + blockSize, img.Width())},
        {j, std::min(j + blockSize, img.Height())}
      );});
  for (auto &t: ts)
    t.join();
  auto renderTime{steady_clock::now() - tStart};

  auto p{std::cout.precision(2)};
  (std::cout << "Render completed in " << std::fixed
    << duration_cast<duration<float>>(renderTime).count() << " seconds." << std::endl
    << std::defaultfloat).precision(p);

  img.Save(fPath.string());
  std::cout << "Image saved as " << fPath << std::endl;
}

// ===========================================================================
