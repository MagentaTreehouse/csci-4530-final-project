#include "utils.h"
#include "matrix.h"
#include "face.h"
#include "argparser.h"

// =========================================================================
// =========================================================================

float Face::getArea() const {
  auto vs{getVertices()};
  const auto
    &a = vs[0]->get(),
    &b = vs[1]->get(),
    &c = vs[2]->get(),
    &d = vs[3]->get();
  return
    AreaOfTriangle(
      DistanceBetweenTwoPoints(a,b),
      DistanceBetweenTwoPoints(a,c),
      DistanceBetweenTwoPoints(b,c)
    ) +
    AreaOfTriangle(
      DistanceBetweenTwoPoints(c,d),
      DistanceBetweenTwoPoints(a,d),
      DistanceBetweenTwoPoints(a,c)
    );
}

// =========================================================================

Vec3f Face::randPoint() const {
  return ::randPoint(getVertices());
}

Vec3f randPoint(const std::array<Vertex *, 4> &vs, float offsetS, float offsetT, float scaleS, float scaleT) {
  const auto
    &a{vs[0]->get()},
    &b{vs[1]->get()},
    &c{vs[2]->get()},
    &d{vs[3]->get()};
  float s = ArgParser::rand() * scaleS + offsetS; // random real in [0,1]
  float t = ArgParser::rand() * scaleT + offsetT; // random real in [0,1]
  return s*t*a + (1-s)*t*b + s*(1-t)*d + (1-s)*(1-t)*c;
}

std::array<std::size_t, 2> Face::sampleLayout(std::size_t n) const {
  auto vs{getVertices()};
  const auto
    &a = vs[0]->get(),
    &b = vs[1]->get(),
    &c = vs[2]->get();
  const float ratio{DistanceBetweenTwoPoints(b, c) / DistanceBetweenTwoPoints(a, b)};
  const auto samplesAB{std::sqrt(n / ratio)};
  if (samplesAB > n) return {n, 1};
  if (samplesAB < 1) return {1, n};
  return {
    static_cast<std::size_t>(samplesAB),
    static_cast<std::size_t>(samplesAB * ratio)
  };
}

// =========================================================================
// the intersection routines

bool Face::intersect(const Ray &r, Hit &h, bool intersect_backfacing) const {
  // intersect with each of the subtriangles
  auto [a, b, c, d]{getVertices()};
  return triangle_intersect(r,h,a,b,c,intersect_backfacing) || triangle_intersect(r,h,a,c,d,intersect_backfacing);
}

bool Face::triangle_intersect(const Ray &r, Hit &h, Vertex *a, Vertex *b, Vertex *c, bool intersect_backfacing) const {

  // compute the intersection with the plane of the triangle
  Hit h2 = h;
  if (!plane_intersect(r,h2,intersect_backfacing)) return 0;

  // figure out the barycentric coordinates:
  Vec3f Ro = r.getOrigin();
  Vec3f Rd = r.getDirection();
  // [ ax-bx   ax-cx  Rdx ][ beta  ]     [ ax-Rox ]
  // [ ay-by   ay-cy  Rdy ][ gamma ]  =  [ ay-Roy ]
  // [ az-bz   az-cz  Rdz ][ t     ]     [ az-Roz ]
  // solve for beta, gamma, & t using Cramer's rule

  float detA = Matrix::det3x3(a->get().x()-b->get().x(),a->get().x()-c->get().x(),Rd.x(),
                              a->get().y()-b->get().y(),a->get().y()-c->get().y(),Rd.y(),
                              a->get().z()-b->get().z(),a->get().z()-c->get().z(),Rd.z());

  if (fabs(detA) <= 0.000001) return 0;
  assert (fabs(detA) >= 0.000001);

  float beta = Matrix::det3x3(a->get().x()-Ro.x(),a->get().x()-c->get().x(),Rd.x(),
                              a->get().y()-Ro.y(),a->get().y()-c->get().y(),Rd.y(),
                              a->get().z()-Ro.z(),a->get().z()-c->get().z(),Rd.z()) / detA;
  float gamma = Matrix::det3x3(a->get().x()-b->get().x(),a->get().x()-Ro.x(),Rd.x(),
                               a->get().y()-b->get().y(),a->get().y()-Ro.y(),Rd.y(),
                               a->get().z()-b->get().z(),a->get().z()-Ro.z(),Rd.z()) / detA;

  if (beta >= -0.00001 && beta <= 1.00001 &&
      gamma >= -0.00001 && gamma <= 1.00001 &&
      beta + gamma <= 1.00001) {
    h = h2;
    // interpolate the texture coordinates
    float alpha = 1 - beta - gamma;
    float t_s = alpha * a->get_s() + beta * b->get_s() + gamma * c->get_s();
    float t_t = alpha * a->get_t() + beta * b->get_t() + gamma * c->get_t();
    h.setTextureCoords(t_s,t_t);
    assert (h.getT() >= EPSILON);
    return 1;
  }

  return 0;
}


bool Face::plane_intersect(const Ray &r, Hit &h, bool intersect_backfacing) const {

  // insert the explicit equation for the ray into the implicit equation of the plane

  // equation for a plane
  // ax + by + cz = d;
  // normal . p + direction = 0
  // plug in ray
  // origin + direction * t = p(t)
  // origin . normal + t * direction . normal = d;
  // t = d - origin.normal / direction.normal;

  Vec3f normal = computeNormal();
  float d = normal.Dot3((*this)[0]->get());

  float numer = d - r.getOrigin().Dot3(normal);
  float denom = r.getDirection().Dot3(normal);

  if (denom == 0) return 0;  // parallel to plane

  if (!intersect_backfacing && normal.Dot3(r.getDirection()) >= 0)
    return 0; // hit the backside

  float t = numer / denom;
  if (t > EPSILON && t < h.getT()) {
    h.set(t,this->getMaterial(),normal);
    assert (h.getT() >= EPSILON);
    return 1;
  }
  return 0;
}

Vec3f Face::computeNormal() const {
  // note: this face might be non-planar, so average the two triangle normals
  auto vs{getVertices()};
  const auto
    &a = vs[0]->get(),
    &b = vs[1]->get(),
    &c = vs[2]->get(),
    &d = vs[3]->get();
  return 0.5f * (ComputeNormal(a,b,c) + ComputeNormal(a,c,d));
}

// =========================================================================
