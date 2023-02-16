#include "vectors.h"
#include "radiosity.h"
#include "mesh.h"
#include "face.h"
#include "sphere.h"
#include "raytree.h"
#include "raytracer.h"
#include "utils.h"

// ================================================================
// CONSTRUCTOR & DESTRUCTOR
// ================================================================
Radiosity::Radiosity(Mesh *m, ArgParser *a):
  mesh{m},
  args{a},
  num_faces{-1},
  formfactors{},
  area{},
  undistributed{},
  absorbed{},
  radiance{},
  max_undistributed_patch{-1},
  total_area{-1}
{
  Reset();
}

Radiosity::~Radiosity() {
  Cleanup();
}

void Radiosity::Cleanup() {
  delete [] formfactors;
  delete [] area;
  delete [] undistributed;
  delete [] absorbed;
  delete [] radiance;
  num_faces = -1;
  formfactors = nullptr;
  area = nullptr;
  undistributed = nullptr;
  absorbed = nullptr;
  radiance = nullptr;
  max_undistributed_patch = -1;
  total_area = -1;
}

void Radiosity::Reset() {
  delete [] area;
  delete [] undistributed;
  delete [] absorbed;
  delete [] radiance;

  // create and fill the data structures
  num_faces = mesh->numFaces();
  area = new float[num_faces];
  undistributed = new Vec3f[num_faces];
  absorbed = new Vec3f[num_faces];
  radiance = new Vec3f[num_faces];
  for (int i = 0; i < num_faces; i++) {
    Face *f = mesh->getFace(i);
    f->setRadiosityPatchIndex(i);
    setArea(i,f->getArea());
    Vec3f emit = f->getMaterial()->getEmittedColor();
    setUndistributed(i,emit);
    setAbsorbed(i,{0,0,0});
    setRadiance(i,emit);
  }

  // find the patch with the most undistributed energy
  findMaxUndistributed();
}


// =======================================================================================
// =======================================================================================

void Radiosity::findMaxUndistributed() {
  // find the patch with the most undistributed energy 
  // don't forget that the patches may have different sizes!
  max_undistributed_patch = -1;
  total_undistributed = 0;
  total_area = 0;
  float max = -1;
  for (int i = 0; i < num_faces; i++) {
    float m = getUndistributed(i).Length() * getArea(i);
    total_undistributed += m;
    total_area += getArea(i);
    if (max < m) {
      max = m;
      max_undistributed_patch = i;
    }
  }
  assert (max_undistributed_patch >= 0 && max_undistributed_patch < num_faces);
}


void Radiosity::ComputeFormFactors() {
  assert (formfactors == nullptr);
  assert (num_faces > 0);
  formfactors = new float[num_faces*num_faces];


  // =====================================
  // ASSIGNMENT:  COMPUTE THE FORM FACTORS
  // =====================================


}


// ================================================================
// ================================================================

float Radiosity::Iterate() {
  if (formfactors == nullptr) 
    ComputeFormFactors();
  assert (formfactors != nullptr);




  // ==========================================
  // ASSIGNMENT:  IMPLEMENT RADIOSITY ALGORITHM
  // ==========================================


  // return the total light yet undistributed
  // (so we can decide when the solution has sufficiently converged)
  return 0;


}



// =======================================================================================
// HELPER FUNCTIONS FOR RENDERING
// =======================================================================================

// for interpolation
void CollectFacesWithVertex(Vertex *have, Face *f, std::vector<Face*> &faces) {
  for (unsigned int i = 0; i < faces.size(); i++) {
    if (faces[i] == f) return;
  }
  if (have != (*f)[0] && have != (*f)[1] && have != (*f)[2] && have != (*f)[3]) return;
  faces.push_back(f);
  for (int i = 0; i < 4; i++) {
    Edge *ea = f->getEdge()->getOpposite();
    Edge *eb = f->getEdge()->getNext()->getOpposite();
    Edge *ec = f->getEdge()->getNext()->getNext()->getOpposite();
    Edge *ed = f->getEdge()->getNext()->getNext()->getNext()->getOpposite();
    if (ea != nullptr) CollectFacesWithVertex(have,ea->getFace(),faces);
    if (eb != nullptr) CollectFacesWithVertex(have,eb->getFace(),faces);
    if (ec != nullptr) CollectFacesWithVertex(have,ec->getFace(),faces);
    if (ed != nullptr) CollectFacesWithVertex(have,ed->getFace(),faces);
  }
}

// different visualization modes
Vec3f Radiosity::setupHelperForColor(Face *f, int i, int j) {
  assert (mesh->getFace(i) == f);
  assert (j >= 0 && j < 4);
  if (args->mesh_data->render_mode == RENDER_MATERIALS) {
    return f->getMaterial()->getDiffuseColor();
  } else if (args->mesh_data->render_mode == RENDER_RADIANCE && args->mesh_data->interpolate == true) {
    std::vector<Face*> faces;
    CollectFacesWithVertex((*f)[j],f,faces);
    float total = 0;
    Vec3f color{0,0,0};
    const Vec3f normal = f->computeNormal();
    for (auto fp: faces) {
      const Vec3f normal2 = fp->computeNormal();
      float area = fp->getArea();
      if (normal.Dot3(normal2) < 0.5) continue;
      assert (area > 0);
      total += area;
      color += area * getRadiance(fp->getRadiosityPatchIndex());
    }
    assert (total > 0);
    color /= total;
    return color;
  } else if (args->mesh_data->render_mode == RENDER_LIGHTS) {
    return f->getMaterial()->getEmittedColor();
  } else if (args->mesh_data->render_mode == RENDER_UNDISTRIBUTED) { 
    return getUndistributed(i);
  } else if (args->mesh_data->render_mode == RENDER_ABSORBED) {
    return getAbsorbed(i);
  } else if (args->mesh_data->render_mode == RENDER_RADIANCE) {
    return getRadiance(i);
  } else if (args->mesh_data->render_mode == RENDER_FORM_FACTORS) {
    if (formfactors == nullptr) ComputeFormFactors();
    float scale = 0.2 * total_area/getArea(i);
    float factor = scale * getFormFactor(max_undistributed_patch,i);
    return {factor,factor,factor};
  } else {
    assert(0);
  }
  exit(0);
}

// =======================================================================================

std::size_t Radiosity::triCount() const {
  return 12*num_faces;
}

void Radiosity::packMesh(float* &current) {
  
  for (int i = 0; i < num_faces; i++) {
    Face *f = mesh->getFace(i);
    const Vec3f normal = f->computeNormal();
    //double avg_s = 0;
    //double avg_t = 0;

    // wireframe is normally black, except when it's the special
    // patch, then the wireframe is red
    const Vec3f wireframe_color{
      1. * (args->mesh_data->render_mode == RENDER_FORM_FACTORS && i == max_undistributed_patch), 0, 0};

    // 4 corner vertices
    const Vec3f &a_pos = ((*f)[0])->get();
    Vec3f a_color = setupHelperForColor(f,i,0);
    a_color = {linear_to_srgb(a_color.r()),linear_to_srgb(a_color.g()),linear_to_srgb(a_color.b())};
    const Vec3f &b_pos = ((*f)[1])->get();
    Vec3f b_color = setupHelperForColor(f,i,1);
    b_color = {linear_to_srgb(b_color.r()),linear_to_srgb(b_color.g()),linear_to_srgb(b_color.b())};
    const Vec3f &c_pos = ((*f)[2])->get();
    Vec3f c_color = setupHelperForColor(f,i,2);
    c_color = {linear_to_srgb(c_color.r()),linear_to_srgb(c_color.g()),linear_to_srgb(c_color.b())};
    const Vec3f &d_pos = ((*f)[3])->get();
    Vec3f d_color = setupHelperForColor(f,i,3);
    d_color = {linear_to_srgb(d_color.r()),linear_to_srgb(d_color.g()),linear_to_srgb(d_color.b())};

    const Vec3f avg_color = 0.25f * (a_color+b_color+c_color+d_color);

    // the centroid (for wireframe rendering)
    const Vec3f centroid = f->computeCentroid();

    AddWireFrameTriangle(current,
                         a_pos,b_pos,centroid,
                         normal,normal,normal,
                         wireframe_color,
                         a_color,b_color,avg_color);
    AddWireFrameTriangle(current,
                         b_pos,c_pos,centroid,
                         normal,normal,normal,
                         wireframe_color,
                         b_color,c_color,avg_color);
    AddWireFrameTriangle(current,
                         c_pos,d_pos,centroid,
                         normal,normal,normal,
                         wireframe_color,
                         c_color,d_color,avg_color);
    AddWireFrameTriangle(current,
                         d_pos,a_pos,centroid,
                         normal,normal,normal,
                         wireframe_color,
                         d_color,a_color,avg_color);

  }
}

// =======================================================================================
