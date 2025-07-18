#ifndef _RADIOSITY_H_
#define _RADIOSITY_H_

#include <cassert>
#include "argparser.h"
#include "vectors.h"

class Mesh;
class Face;
class Vertex;
class RayTracer;
class PhotonMapping;

// ====================================================================
// ====================================================================
// This class manages the radiosity calculations, including form factors
// and radiance solution.

class Radiosity {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Radiosity(Mesh *m, ArgParser *args);
  ~Radiosity();
  void Reset();
  void Cleanup();
  void ComputeFormFactors();
  void setRayTracer(RayTracer *r) { raytracer = r; }
  void setPhotonMapping(PhotonMapping *pm) { photon_mapping = pm; }

  // =========
  // ACCESSORS
  [[nodiscard]] Mesh* getMesh() const { return mesh; }
  [[nodiscard]] float getFormFactor(int i, int j) const {
    assert (i >= 0 && i < num_faces);
    assert (j >= 0 && j < num_faces);
    assert (formfactors != nullptr);
    return formfactors[i*num_faces+j]; }
  [[nodiscard]] float getArea(int i) const {
    assert (i >= 0 && i < num_faces);
    return area[i]; }
  [[nodiscard]] const Vec3f &getUndistributed(int i) const {
    assert (i >= 0 && i < num_faces);
    return undistributed[i]; }
  [[nodiscard]] const Vec3f &getAbsorbed(int i) const {
    assert (i >= 0 && i < num_faces);
    return absorbed[i]; }
  [[nodiscard]] const Vec3f &getRadiance(int i) const {
    assert (i >= 0 && i < num_faces);
    return radiance[i]; }

  // =========
  // MODIFIERS
  float Iterate();
  void setFormFactor(int i, int j, float value) {
    assert (i >= 0 && i < num_faces);
    assert (j >= 0 && j < num_faces);
    assert (formfactors != nullptr);
    formfactors[i*num_faces+j] = value; }
  void normalizeFormFactors(int i) {
    float sum = 0;
    int j;
    for (j = 0; j < num_faces; j++) {
      sum += getFormFactor(i,j); }
    if (sum == 0) return;
    for (j = 0; j < num_faces; j++) {
      setFormFactor(i,j,getFormFactor(i,j)/sum); } }
  void setArea(int i, float value) {
    assert (i >= 0 && i < num_faces);
    area[i] = value; }
  void setUndistributed(int i, const Vec3f &value) {
    assert (i >= 0 && i < num_faces);
    undistributed[i] = value; }
  void findMaxUndistributed();
  void setAbsorbed(int i, const Vec3f &value) {
    assert (i >= 0 && i < num_faces);
    absorbed[i] = value; }
  void setRadiance(int i, const Vec3f &value) {
    assert (i >= 0 && i < num_faces);
    radiance[i] = value; }

  std::size_t triCount() const;
  void packMesh(float* &current);

private:
  Vec3f setupHelperForColor(Face *f, int i, int j);

  // ==============
  // REPRESENTATION
  Mesh *mesh;
  ArgParser *args;
  int num_faces;
  RayTracer *raytracer;
  PhotonMapping *photon_mapping;

  // a nxn matrix
  // F_i,j radiant energy leaving i arriving at j
  float *formfactors;

  // length n vectors
  float *area;
  Vec3f *undistributed; // energy per unit area
  Vec3f *absorbed;      // energy per unit area
  Vec3f *radiance;      // energy per unit area
  Vec3f *normals;

  int max_undistributed_patch;  // the patch with the most undistributed energy
  float total_undistributed;    // the total amount of undistributed light
  float total_area;             // the total area of the scene
};

// ====================================================================
// ====================================================================

#endif
