
#include "model.h"
#include "../math/matrix.h"
#include "../math/bsphere.h"
#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../mesh/mesh.h"

#include <cassert>

using namespace CGE;

Model::Model() : SceneNode(MESH){
  trafo_ = Matrix(Matrix::Identity);
  mesh_ = NULL;
  boundingObj_ = NULL;
  for (int i = 0; i < MAX_TEXTURES; i++){
    textures_[i] = NULL;
  }
  for (int i = 0; i < MAX_ATTRIBS; i++){
    attribs_[i] = 0;
  }
}

Model::Model(Mesh* mesh) : SceneNode(MESH){
  trafo_ = Matrix(Matrix::Identity);
  mesh_ = mesh;
  BBox box = mesh_->getExtents();
  Vector3D center = (box.getMin()+box.getMax())/2.0f;
  float radius = (box.getMax()-box.getMin()).length()/2.0f;
  boundingObj_ = new BSphere(center, radius);
  for (int i = 0; i < MAX_TEXTURES; i++){
    textures_[i] = NULL;
  }
  for (int i = 0; i < MAX_ATTRIBS; i++){
    attribs_[i] = 0;
  }
}

//! Copy constructor
Model::Model(const Model& m) : SceneNode(m){
  //trafo_ = m.trafo_;
  mesh_ = m.mesh_;
  boundingObj_ = new BSphere(*dynamic_cast<BSphere*>(m.boundingObj_));
  for (int i = 0; i < MAX_TEXTURES; i++){
    textures_[i] = m.textures_[i];
  }
  for (int i = 0; i < MAX_ATTRIBS; i++){
    attribs_[i] = m.attribs_[i];
  }
}

Model::~Model(){
  SAFE_DELETE(boundingObj_);
}

// Render the model
void Model::render(Mesh::DrawPart part){
  assert(mesh_ && "A mesh needs to be assigned to the model");
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->pushMatrix();
  rend->multiplyMatrix(trafo_);
  mesh_->draw(part);
  rend->popMatrix();
}

//! Setup material
void Model::setupMaterial(){
  /*
  for (int i = 0; i < MAX_TEXTURES; i++){
    if (textures_[i])
      textures_[i]->activate();
  }*/
  if (textures_[0])
    textures_[0]->activate();
}
  
//! Reset material
void Model::resetMaterial(){
  if (textures_[0])
    textures_[0]->deactivate();
}

//! Assign a texture
void Model::assignTexture(CGE::Texture* tex, int stage){
  textures_[stage] = tex;
}

