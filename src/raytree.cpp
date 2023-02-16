#include "raytree.h"
#include "utils.h"

// ====================================================================
// Initialize the static variables
int RayTree::activated = 0;  
std::vector<Segment> RayTree::main_segments;
std::vector<Segment> RayTree::shadow_segments;
std::vector<Segment> RayTree::reflected_segments;
std::vector<Segment> RayTree::transmitted_segments;

std::size_t RayTree::triCount() {
  return numSegments()*12;
}

void addBox(float* &current, const Vec3f &start, const Vec3f &end, const Vec3f &color, float width) {

  // find perpendicular axes
  Vec3f dir = (end-start);
  Vec3f one;
  Vec3f two;
  if (dir.Length() < 0.01*width) {
    dir = one = two = {0,0,0};
  } else {
    dir.Normalize(); 
    Vec3f tmp; Vec3f::Cross3(tmp,dir,{1,0,0});
    if (tmp.Length() < 0.1) {
      Vec3f::Cross3(tmp,dir,{0,0,1});
    }
    tmp.Normalize();
    Vec3f::Cross3(one,dir,tmp);
    assert (fabs(one.Length()-1.0) < 0.001);
    Vec3f::Cross3(two,dir,one);
    assert (fabs(two.Length()-1.0) < 0.001);
  }

  const Vec3f pos[]{
    {start + width*one + width*two},
    {start + width*one - width*two},
    {start - width*one + width*two},
    {start - width*one - width*two},
    {end   + width*one + width*two},
    {end   + width*one - width*two},
    {end   - width*one + width*two},
    {end   - width*one - width*two}
  };

  AddBox(current,pos,color);
}

void RayTree::packMesh(float* &current) {

  const Vec3f
    main_color{0.7,0.7,0.7},
    shadow_color{0.1,0.9,0.1},
    reflected_color{0.9,0.1,0.1},
    transmitted_color{0.1,0.1,0.9};

  float width = 0.01;

  for (const auto &seg: main_segments) {
    addBox(current, seg.getStart(), seg.getEnd(), main_color, width);
  }
  for (const auto &seg: shadow_segments) {
    addBox(current, seg.getStart(), seg.getEnd(), shadow_color, width);
  }
  for (const auto &seg: reflected_segments) {
    addBox(current, seg.getStart(), seg.getEnd(), reflected_color, width);
  }
  for (const auto &seg: transmitted_segments) {
    addBox(current, seg.getStart(), seg.getEnd(), transmitted_color, width);
  }
}

// ===========================================================================
