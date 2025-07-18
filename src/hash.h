#ifndef _HASH_H_
#define _HASH_H_

#include <unordered_map>
#include "vertex.h"

class Edge;
class Triangle;

#define LARGE_PRIME_A 10007
#define LARGE_PRIME_B 11003


// ===================================================================================
// DIRECTED EDGES are stored in a hash table using a simple hash
// function based on the indices of the start and end vertices
// ===================================================================================

inline unsigned int ordered_two_int_hash(unsigned int a, unsigned int b) {
  return LARGE_PRIME_A * a + LARGE_PRIME_B * b;
}

struct orderedvertexpairhash {
  size_t operator()(std::pair<Vertex*,Vertex*> p) const {
    return ordered_two_int_hash(p.first->getIndex(),p.second->getIndex());
  }
};

struct orderedsamevertexpair {
  bool operator()(std::pair<Vertex*,Vertex*> p1, std::pair<Vertex*,Vertex*>p2) const {
    return p1.first->getIndex() == p2.first->getIndex() && p1.second->getIndex() == p2.second->getIndex();
  }
};


// ===================================================================================
// PARENT/CHILD VERTEX relationships (for subdivision) are stored in a
// hash table using a simple hash function based on the indices of the
// parent vertices, smaller index first
// ===================================================================================

inline unsigned int unordered_two_int_hash(unsigned int a, unsigned int b) {
  assert (a != b);
  if (b < a) {
    return ordered_two_int_hash(b,a);
  } else {
    assert (a < b);
    return ordered_two_int_hash(a,b);
  }
}

struct unorderedvertexpairhash {
  size_t operator()(std::pair<Vertex*,Vertex*> p) const {
    return unordered_two_int_hash(p.first->getIndex(),p.second->getIndex());
  }
};

struct unorderedsamevertexpair {
  bool operator()(std::pair<Vertex*,Vertex*> p1, std::pair<Vertex*,Vertex*>p2) const {
    if ((p1.first->getIndex() == p2.first->getIndex() && p1.second->getIndex() == p2.second->getIndex()) ||
	(p1.first->getIndex() == p2.second->getIndex() && p1.second->getIndex() == p2.first->getIndex())) return true;
    return false;
  }
};

/*
  LEGACY: to handle different platforms with different variants of a developing standard
  NOTE: You may need to adjust these depending on your installation
*/
typedef std::unordered_map<std::pair<Vertex*,Vertex*>,Vertex*,unorderedvertexpairhash,unorderedsamevertexpair> vphashtype;
typedef std::unordered_map<std::pair<Vertex*,Vertex*>,Edge*,orderedvertexpairhash,orderedsamevertexpair> edgeshashtype;

#endif // _HASH_H_
