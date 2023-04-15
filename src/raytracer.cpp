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
#include "camera.h"
#include "image.h"


inline auto ToUnitSquare(std::tuple<double, double> p) {
  const auto &md{*GLOBAL_args->mesh_data};
  const int max_d{std::max(md.width,md.height)};
  const auto [x, y]{p};
  return std::tuple{
    (x - md.width / 2.) / max_d + .5,
    (y - md.height / 2.) / max_d + .5
  };
}


// ===========================================================================
// casts a single ray through the scene geometry and finds the closest hit
bool RayTracer::CastRay(const Ray &ray, Hit &h, bool use_rasterized_patches) const {
  bool answer = false;

  // intersect each of the quads
  for (auto fp: mesh->getOriginalQuads())
    answer |= fp->intersect(ray,h,args->mesh_data->intersect_backfacing);

  // intersect each of the primitives (either the patches, or the original primitives)
  if (use_rasterized_patches) {
    for (auto fp: mesh->getRasterizedPrimitiveFaces())
      answer |= fp->intersect(ray,h,args->mesh_data->intersect_backfacing);
  } else {
    for (auto pp: mesh->getPrimitives())
      answer |= pp->intersect(ray,h);
  }
  return answer;
}


// shoot at random direction in a hemisphere based on a normal
Vec3f HemisphereRandom(std::tuple<float, float> unitSqrPt, Vec3f normal) {
  auto [px, py]{unitSqrPt};
  float r{std::sqrt(1.f - px * px)};
  float phi = 2 * M_PI * py;
  Vec3f dir{r * std::cos(phi), r * std::sin(phi), px};
  if (std::abs(normal.z() - 1) < EPSILON)
    return dir;
  float x = normal.x();
  float y = normal.y();
  float z = normal.z();
  auto sqrt{std::sqrt(x * x + y * y)};
  float t[]{
    y / sqrt,     -x / sqrt,    0,      0,
    x * z / sqrt, y * z / sqrt, -sqrt,  0,
    x,            y,            z,      0,
    0,            0,            0,      1
  };
  return Matrix{t} * dir;
}


Vec3f HemisphereRandom_legacy(Vec3f normal) {
    float phi = ArgParser::rand() * 2 * M_PI;
    float theta = ArgParser::rand() * M_PI / 2;
    float x = normal.x();
    float y = normal.y();
    float z = normal.z();
    float phi2;
    float theta2;
    theta2 = acos(z);
    if (fabs(x) < EPSILON) {
        if (y == 0) {
            phi2 = 0;
        }
        else if (y > 0) {
            phi2 = M_PI / 2;
        }
        else {
            phi2 = -M_PI / 2;
        }
    }
    else {
        phi2 = atan(y / x);
    }
    phi == phi2;
    theta += theta2;
    Vec3f dir = Vec3f(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));

    /*
    float t[16] = { (y / sqrt(x * x + y * y)),  -x / sqrt(x * x + y * y),       0,                  0,
                    x * z / sqrt(x * x + y * y), y * z / sqrt(x * x + y * y), -sqrt(x * x + y * y), 0,
                    x,                              y,                          z,                  1 };
    Matrix transform = Matrix(t);

    return transform * dir;*/
    return dir;
}


template<class F, bool Visualize>
Vec3f RayTracer::shade(const Ray &ray, Hit &hit, const Material &m, int depth, F directIllum, std::bool_constant<Visualize>) const {
  const Vec3f &d{ray.getDirection()};
  const Vec3f &normal{hit.getNormal()};
  const Vec3f point{ray.pointAtParameter(hit.getT())};
  const Vec3f diffuse_color{m.getDiffuseColor(hit.get_s(),hit.get_t())};

  Vec3f answer{};

  // direct illumination
  for (const Face *f: mesh->getLights())
    answer += directIllum(*f, point,
      [&] (const Vec3f &ptLtSample) {
        const float
          distSqr = ptLtSample.Dot3(ptLtSample),
          dist = std::sqrt(distSqr),
          cosTheta = std::max(ptLtSample.Dot3(normal), 0.) / dist,
          cosThetaP = std::max((-ptLtSample).Dot3(f->computeNormal()), 0.) / dist;
        const Vec3f ltColor{f->getMaterial()->getEmittedColor()};
        return cosTheta * cosThetaP / distSqr * f->getArea() * ltColor * m.brdf(hit, d, ptLtSample);
      });

  // indirect illumination
  if (!depth) return answer;
  auto dir{HemisphereRandom({static_cast<float>(ArgParser::rand()), static_cast<float>(ArgParser::rand())}, normal)};
  const Ray r{point, dir};
  Hit h{};
  if (CastRay(r, h, false) && !h.getMaterial()->isEmitting()) {
    if constexpr (Visualize) RayTree::AddReflectedSegment(r, 0, h.getT());
    Vec3f ptLtSample{r.pointAtParameter(h.getT()) - point};
    const float cosTheta = ptLtSample.Dot3(normal) / ptLtSample.Length();
    answer += cosTheta * 2 * M_PI *
      shade<F, Visualize>(r, h, *h.getMaterial(), depth - 1, directIllum, {}) *
      diffuse_color * m.brdf(hit, d, ptLtSample);
  }

  // add contribution from reflection, if the surface is shiny
  if (m.getReflectiveColor() != Vec3f{} && depth) {
    const Ray r{point, Reflection(d, normal)};
    Hit h{};
    answer +=
      m.getReflectiveColor() *
      TraceRayImpl<F, Visualize>(r, h, depth - 1, directIllum, {});
    if constexpr (Visualize) RayTree::AddReflectedSegment(r, 0, h.getT());
  }

  return answer;
}


template<class F, bool Visualize>
Vec3f RayTracer::TraceRayImpl(const Ray &ray, Hit &hit, int depth, F directIllum, std::bool_constant<Visualize>) const {
  hit = {};
  // First cast a ray and see if we hit anything.
  // if there is no intersection, simply return the background color
  if (!CastRay(ray,hit,false))
    return {
      srgb_to_linear(mesh->background_color.r()),
      srgb_to_linear(mesh->background_color.g()),
      srgb_to_linear(mesh->background_color.b())
    };

  // otherwise decide what to do based on the material
  const Material *m{hit.getMaterial()};
  assert (m != nullptr);
  if (m->isEmitting())
    return m->getEmittedColor();
  return shade<F, Visualize>(ray, hit, *m, depth, directIllum, {});
}


template<bool Visualize>
Vec3f RayTracer::TraceRay(const Ray &ray, Hit &hit, int depth) const {
  const auto &md{*args->mesh_data};

  // "shadow ray"
  auto directIllum{[&] (const Ray &r, auto shadeLocal) {
    Hit block{};
    CastRay(r, block, false);
    if constexpr (Visualize) RayTree::AddShadowSegment(r, 0, block.getT());
    return block.getT() > 1 - EPSILON? shadeLocal(r.getDirection()) : Vec3f{};
  }};

  std::bool_constant<Visualize> vis{};
  // The behavior of direct illumination calculation differs for certain values of shadow
  // samples and antialiasing samples
  switch (int sSamp{md.num_shadow_samples}; sSamp * md.num_antialias_samples) {
  case 0:
  return TraceRayImpl(ray, hit, depth,
    [] (const Face &lt, const Vec3f &pt, auto shadeLocal) { // no shadows considered
      const auto ptLtC{lt.computeCentroid() - pt};
      if constexpr (Visualize) RayTree::AddShadowSegment({pt, ptLtC}, 0, 1);
      return shadeLocal(ptLtC);
    }, vis);

  case 1:
  return TraceRayImpl(ray, hit, depth,
    [&] (const Face &lt, const Vec3f &pt, auto shadeLocal) { // "decay" to hard shadows
      return directIllum({pt, lt.computeCentroid() - pt}, shadeLocal);
    }, vis);

  default:
  return TraceRayImpl(ray, hit, depth,
    [&] (const Face &lt, const Vec3f &pt, auto shadeLocal) { // soft shadows
      Vec3f directIllumSum{};
      const auto vs{lt.getVertices()};
      const auto sampleN{lt.sampleLayout(sSamp)};
      const float scaleI{1.f / sampleN[0]}, scaleJ{1.f / sampleN[1]};
      for (std::size_t i{}; i < sampleN[0]; ++i)
        for (std::size_t j{}; j < sampleN[1]; ++j) {
          const float offsetI{1.f * i / sampleN[0]}, offsetJ{1.f * j / sampleN[1]};
          directIllumSum +=
            directIllum({pt, randPoint(vs, offsetI, offsetJ, scaleI, scaleJ) - pt}, shadeLocal);
        }
      return 1. / (sampleN[0] * sampleN[1]) * directIllumSum;
    }, vis);
  }
}


// camera sampling, including antialiasing
// trace a ray through pixel (i,j) of the image an return the radiance
template<bool Visualize>
Vec3f RayTracer::renderPixel(double i, double j) const {
  const auto &md{*args->mesh_data};
  const auto aa{static_cast<std::size_t>(std::sqrt(md.num_antialias_samples))};
  const auto
    ds{1. / aa},
    i0{i + ds / 2},
    j0{j + ds / 2};

  Vec3f sum{};
  for (std::size_t si{}; si < aa; ++si)
    for (std::size_t sj{}; sj < aa; ++sj) {
      const auto [x, y]{ToUnitSquare({i0 + ds * si, j0 + ds * sj})};
      const Ray r = args->mesh->camera->generateRay(x,y);
      Hit hit;
      sum += TraceRay<Visualize>(r, hit, md.num_bounces);
      if constexpr (Visualize) RayTree::AddMainSegment(r, 0, hit.getT());
    }

  return 1. / (aa * aa) * sum;
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
  auto pack{[&] (const std::vector<Pixel> &pixels, bool renderToPixels) {
    for (const auto &p: pixels) {
      Vec3f
        v1 = p.v1,
        v2 = p.v2,
        v3 = p.v3,
        v4 = p.v4;
      const Vec3f normal{(ComputeNormal(v1,v2,v3) + ComputeNormal(v1,v3,v4)).Normalized()};
      if (renderToPixels) {
        v1 += 0.02*normal;
        v2 += 0.02*normal;
        v3 += 0.02*normal;
        v4 += 0.02*normal;
      }
      AddQuad(current,v1,v2,v3,v4,{},p.color);
    }
  }};
  pack(pixels_a, render_to_a);
  pack(pixels_b, !render_to_a);
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
        auto viewTransform{[] (float x) -> std::uint8_t {
          return std::round(255 * std::min(linear_to_srgb(x), 1.f));
        }};
        img.SetPixel(i, j,
          {viewTransform(p.r()), viewTransform(p.g()), viewTransform(p.b())});
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
