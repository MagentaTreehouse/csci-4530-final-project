#ifndef _KDTREE_H_
#define _KDTREE_H_

#include <cstdlib>
#include <vector>
#include "boundingbox.h"
#include "photon.h"

// ==================================================================
// A hierarchical spatial data structure to store photons.  This data
// struture allows for fast nearby neighbor queries for use in photon
// mapping.

class KDTree {
 public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  KDTree(const BoundingBox &_bbox, int _depth=0) {
    bbox = _bbox;
    depth = _depth;
    child1=nullptr;
    child2=nullptr;      
  }
  ~KDTree();

  // =========
  // ACCESSORS
  [[nodiscard]] std::size_t numPhotons() const;
  [[nodiscard]] std::size_t numBoxes() const;
  // boundingbox
  [[nodiscard]] const Vec3f& getMin() const { return bbox.getMin(); }
  [[nodiscard]] const Vec3f& getMax() const { return bbox.getMax(); }
  [[nodiscard]] bool overlaps(const BoundingBox &bb) const;
  // hierarchy
  [[nodiscard]] int getDepth() const { return depth; }
  [[nodiscard]] bool isLeaf() const { 
    if (child1==nullptr&&child2==nullptr) return true;
    assert (child1 != nullptr && child2 != nullptr);
    return false; }
  const KDTree* getChild1() const { assert (!isLeaf()); assert (child1 != nullptr); return child1; }
  const KDTree* getChild2() const { assert (!isLeaf()); assert (child2 != nullptr); return child2; }
  // photons
  [[nodiscard]] const std::vector<Photon>& getPhotons() const { return photons; }
  void CollectPhotonsInBox(const BoundingBox &bb, std::vector<Photon> &photons) const;

  // =========
  // MODIFIERS
  void AddPhoton(const Photon &p);
  bool PhotonInCell(const Photon &p);

 private:

  // HELPER FUNCTION
  void SplitCell();

  // REPRESENTATION
  BoundingBox bbox;
  KDTree* child1;
  KDTree* child2;
  int split_axis;
  float split_value;
  std::vector<Photon> photons;
  int depth;
};

#endif
