#include <iostream>
#include <algorithm>
#include <cstring>

#include "argparser.h"
#include "photon_mapping.h"
#include "mesh.h"
#include "meshdata.h"
#include "face.h"
#include "kdtree.h"
#include "utils.h"
#include "raytracer.h"


// ==========
// Clear/reset
void PhotonMapping::Clear() {
  // cleanup all the photons
  delete kdtree;
  kdtree = nullptr;
}


// ========================================================================
// Recursively trace a single photon

void PhotonMapping::TracePhoton(const Vec3f &/*position*/, const Vec3f &/*direction*/,
				const Vec3f &/*energy*/, int /*iter*/) {

  // ==============================================
  // ASSIGNMENT: IMPLEMENT RECURSIVE PHOTON TRACING
  // ==============================================

  // Trace the photon through the scene.  At each diffuse or
  // reflective bounce, store the photon in the kd tree.

  // One optimization is to *not* store the first bounce, since that
  // direct light can be efficiently computed using classic ray
  // tracing.


}


// ========================================================================
// Trace the specified number of photons through the scene

void PhotonMapping::TracePhotons() {

  // first, throw away any existing photons
  delete kdtree;

  // consruct a kdtree to store the photons
  BoundingBox *bb = mesh->getBoundingBox();
  Vec3f min = bb->getMin();
  Vec3f max = bb->getMax();
  Vec3f diff = max-min;
  min -= 0.001f*diff;
  max += 0.001f*diff;
  kdtree = new KDTree({min,max});

  // photons emanate from the light sources
  const std::vector<Face*>& lights = mesh->getLights();

  // compute the total area of the lights
  float total_lights_area{};
  for (const Face *faceP: lights) {
    total_lights_area += faceP->getArea();
  }

  // shoot a constant number of photons per unit area of light source
  // (alternatively, this could be based on the total energy of each light)
  for (const Face *faceP: lights) {  
    const float my_area = faceP->getArea();
    const int num = args->mesh_data->num_photons_to_shoot * my_area / total_lights_area;
    // the initial energy for this photon
    Vec3f energy = my_area/num * faceP->getMaterial()->getEmittedColor();
    Vec3f normal = faceP->computeNormal();
    for (int j = 0; j < num; j++) {
      const Vec3f start = faceP->RandomPoint();
      // the initial direction for this photon (for diffuse light sources)
      const Vec3f direction = RandomDiffuseDirection(normal);
      TracePhoton(start,direction,energy,0);
    }
  }
}


// ======================================================================

// helper function
bool closest_photon(const std::pair<Photon,float> &a, const std::pair<Photon,float> &b) {
  return (a.second < b.second);
}


// ======================================================================
Vec3f PhotonMapping::GatherIndirect(const Vec3f &/*point*/, const Vec3f &/*normal*/,
                                    const Vec3f &/*direction_from*/) const {


  if (kdtree == nullptr) { 
    std::cout << "WARNING: Photons have not been traced throughout the scene." << std::endl;
    return {0,0,0}; 
  }

  // ================================================================
  // ASSIGNMENT: GATHER THE INDIRECT ILLUMINATION FROM THE PHOTON MAP
  // ================================================================

  // collect the closest args->num_photons_to_collect photons
  // determine the radius that was necessary to collect that many photons
  // average the energy of those photons over that radius
  
  // return the color
  return {0,0,0};
}

// ======================================================================
// ======================================================================
// Helper functions to render the photons & kdtree

std::size_t PhotonMapping::triCount() const {
  std::size_t tri_count{};
  if (GLOBAL_args->mesh_data->render_kdtree == true && kdtree != nullptr) 
    tri_count += kdtree->numBoxes()*12*12;
  if (GLOBAL_args->mesh_data->render_photon_directions == true && kdtree != nullptr) 
    tri_count += kdtree->numPhotons()*12;
  return tri_count;
}

std::size_t PhotonMapping::pointCount() const {
  if (GLOBAL_args->mesh_data->render_photons == false || kdtree == nullptr) return 0;
  return kdtree->numPhotons();
}

// defined in raytree.cpp
void addBox(float* &current, const Vec3f &start, const Vec3f &end, const Vec3f &color, float width);

// ======================================================================

void packKDTree(const KDTree *kdtree, float* &current, std::size_t &count) {
  if (!kdtree->isLeaf()) {
    if (kdtree->getChild1()) packKDTree(kdtree->getChild1(),current,count);
    if (kdtree->getChild2()) packKDTree(kdtree->getChild2(),current,count);
  } else {
    const Vec3f &a = kdtree->getMin();
    const Vec3f &b = kdtree->getMax();

    Vec3f corners[]{
      {a.x(),a.y(),a.z()},
      {a.x(),a.y(),b.z()},
      {a.x(),b.y(),a.z()},
      {a.x(),b.y(),b.z()},
      {b.x(),a.y(),a.z()},
      {b.x(),a.y(),b.z()},
      {b.x(),b.y(),a.z()},
      {b.x(),b.y(),b.z()}
    };

    const float width = 0.01 * (a-b).Length();

    addBox(current,corners[0],corners[1],{1,1,0},width);
    addBox(current,corners[1],corners[3],{1,1,0},width);
    addBox(current,corners[3],corners[2],{1,1,0},width);
    addBox(current,corners[2],corners[0],{1,1,0},width);

    addBox(current,corners[4],corners[5],{1,1,0},width);
    addBox(current,corners[5],corners[7],{1,1,0},width);
    addBox(current,corners[7],corners[6],{1,1,0},width);
    addBox(current,corners[6],corners[4],{1,1,0},width);

    addBox(current,corners[0],corners[4],{1,1,0},width);
    addBox(current,corners[1],corners[5],{1,1,0},width);
    addBox(current,corners[2],corners[6],{1,1,0},width);
    addBox(current,corners[3],corners[7],{1,1,0},width);

    count++;
  }
}

// ======================================================================

void packPhotons(const KDTree *kdtree, float* &current_points, std::size_t &count) {
  if (!kdtree->isLeaf()) {
      if (kdtree->getChild1()) packPhotons(kdtree->getChild1(),current_points,count);
      if (kdtree->getChild2()) packPhotons(kdtree->getChild2(),current_points,count);
  } else {
    for (std::size_t i{}; i < kdtree->getPhotons().size(); i++) {
      const Photon &p = kdtree->getPhotons()[i];
      const Vec3f &v = p.getPosition();
      const Vec3f color = p.getEnergy()*GLOBAL_args->mesh_data->num_photons_to_shoot;
      float12 t{ float(v.x()),float(v.y()),float(v.z()),1,   0,0,0,0,   float(color.r()),float(color.g()),float(color.b()),1 };
      memcpy(current_points, &t, sizeof(float)*12); current_points += 12; 
      count++;
    }
  }
}


void packPhotonDirections(const KDTree *kdtree, float* &current, std::size_t &count) {
  if (!kdtree->isLeaf()) {
      if (kdtree->getChild1()) packPhotonDirections(kdtree->getChild1(),current,count);
      if (kdtree->getChild2()) packPhotonDirections(kdtree->getChild2(),current,count);
  } else {
    for (std::size_t i{}; i < kdtree->getPhotons().size(); i++) {
      const Photon &p = kdtree->getPhotons()[i];
      const Vec3f &v = p.getPosition();
      const Vec3f v2 = p.getPosition() - p.getDirectionFrom() * 0.5;
      const Vec3f color = p.getEnergy()*float(GLOBAL_args->mesh_data->num_photons_to_shoot);
      const float width = 0.01f;
      addBox(current,v,v2,color,width);
      count++;
    }
  }
}

// ======================================================================

void PhotonMapping::packMesh(float* &current, float* &current_points) {

  // the photons
  if (GLOBAL_args->mesh_data->render_photons && kdtree != nullptr) {
    std::size_t count{};
    packPhotons(kdtree,current_points,count);
    assert (count == kdtree->numPhotons());
  }
  // photon directions
  if (GLOBAL_args->mesh_data->render_photon_directions && kdtree != nullptr) {
    std::size_t count{};
    packPhotonDirections(kdtree,current,count);
    assert (count == kdtree->numPhotons());
  }

  // the wireframe kdtree
  if (GLOBAL_args->mesh_data->render_kdtree && kdtree != nullptr) {
    std::size_t count{};
    packKDTree(kdtree,current,count);
    assert (count == kdtree->numBoxes());
  }
  
}

// ======================================================================
