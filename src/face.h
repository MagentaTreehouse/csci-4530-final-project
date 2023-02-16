#ifndef _FACE_H_
#define _FACE_H_

#include "edge.h"
#include "ray.h"
#include "vertex.h"
#include "hit.h"

class Material;

// ==============================================================
// Simple class to store quads for use in radiosity & raytracing.

class Face {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Face(Material *m): edge{}, material{m} {}

  // =========
  // ACCESSORS
  [[nodiscard]] Vertex* operator[](int i) const { 
    assert (edge != nullptr);
    if (i==0) return edge->getStartVertex();
    if (i==1) return edge->getNext()->getStartVertex();
    if (i==2) return edge->getNext()->getNext()->getStartVertex();
    if (i==3) return edge->getNext()->getNext()->getNext()->getStartVertex();
    assert(0);
    exit(0);
  }
  [[nodiscard]] Edge* getEdge() const { 
    assert (edge != nullptr);
    return edge; 
  }
  [[nodiscard]] Vec3f computeCentroid() const {
    return 0.25f * ((*this)[0]->get() +
                    (*this)[1]->get() +
                    (*this)[2]->get() +
                    (*this)[3]->get());
  }
  [[nodiscard]] Material* getMaterial() const { return material; }
  [[nodiscard]] float getArea() const;
  [[nodiscard]] Vec3f RandomPoint() const;
  [[nodiscard]] Vec3f computeNormal() const;

  // =========
  // MODIFIERS
  void setEdge(Edge *e) {
    assert (edge == nullptr);
    assert (e != nullptr);
    edge = e;
  }

  // ==========
  // RAYTRACING
  bool intersect(const Ray &r, Hit &h, bool intersect_backfacing) const;

  // =========
  // RADIOSITY
  [[nodiscard]] int getRadiosityPatchIndex() const { return radiosity_patch_index; }
  void setRadiosityPatchIndex(int i) { radiosity_patch_index = i; }

protected:

  // helper functions
  bool triangle_intersect(const Ray &r, Hit &h, Vertex *a, Vertex *b, Vertex *c, bool intersect_backfacing) const;
  bool plane_intersect(const Ray &r, Hit &h, bool intersect_backfacing) const;

  Face& operator=(const Face&) = delete;

  // ==============
  // REPRESENTATION
  Edge *edge;
  // NOTE: If you want to modify a face, remove it from the mesh,
  // delete it, create a new copy with the changes, and re-add it.
  // This will ensure the edges get updated appropriately.

  int radiosity_patch_index;  // an awkward pointer to this patch in the Radiosity patch array
  Material *material;
};

// ===========================================================

#endif
