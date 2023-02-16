#ifndef _EDGE_H_
#define _EDGE_H_

#include <cassert>
#include <cstdlib>

class Vertex;
class Face;

// ===================================================================
// half-edge data structure

class Edge { 

public:

  // ========================
  // CONSTRUCTORS & DESTRUCTOR
  Edge(Vertex *vs, Vertex *ve, Face *f);
  ~Edge();

  // =========
  // ACCESSORS
  [[nodiscard]] Vertex* getStartVertex() const { assert (start_vertex != nullptr); return start_vertex; }
  [[nodiscard]] Vertex* getEndVertex() const { assert (end_vertex != nullptr); return end_vertex; }
  [[nodiscard]] Edge* getNext() const { assert (next != nullptr); return next; }
  [[nodiscard]] Face* getFace() const { assert (face != nullptr); return face; }
  [[nodiscard]] Edge* getOpposite() const {
    // warning!  the opposite edge might be nullptr!
    return opposite; }
  float Length() const;

  // =========
  // MODIFIERS
  void setOpposite(Edge *e) {
    assert (opposite == nullptr); 
    assert (e != nullptr);
    assert (e->opposite == nullptr);
    opposite = e; 
    e->opposite = this; 
  }
  void clearOpposite() { 
    if (!opposite) return; 
    assert (opposite->opposite == this); 
    opposite->opposite = nullptr;
    opposite = nullptr; 
  }
  void setNext(Edge *e) {
    assert (next == nullptr);
    assert (e != nullptr);
    assert (face == e->face);
    next = e;
  }

private:

  Edge(const Edge&) = delete;
  Edge& operator=(const Edge&) = delete;

  // ==============
  // REPRESENTATION
  // in the half edge data adjacency data structure, the edge stores everything!
  // note: it's technically not necessary to store both vertices, but it makes
  //   dealing with non-closed meshes easier
  Vertex *start_vertex;
  Vertex *end_vertex;
  Face *face;
  Edge *opposite;
  Edge *next;
};

// ===================================================================

#endif

