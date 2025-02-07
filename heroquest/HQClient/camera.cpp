//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: camera.cpp                                                 |
//  ==================================================================

#include <cmath>

#include <mesh/mesh.h>
#include <renderer/font.h>
#include <renderer/renderer.h>
#include "math.h"
#include "renderer.h"
//#include "font.hh"
#include "message.h"
#include "world.h"
#include "gamestate.h"
#include "player.h"
#include "camera.h"

#define COLL_STEP 0.1f
#define SPEED_SCALE 1.0f

Camera::Camera(){
  //Set some standard values
  position_ = Vector3D();
  view_	= Vector3D(0.0,0.0,1.0);
  upVector_ = Vector3D(0.0,1.0,0.0);
  speed_ = 8.0f;
  modelPos_ = Vector2D();
}

Camera::~Camera(){
}

//Set camera parameters
void Camera::positionCamera(const Vector3D position, const Vector3D view, const Vector3D up){
  position_ = position;
  view_ = view;
  upVector_ = up;
  oldPos_ = modelPos_;
  modelPos_ = wrld.realToModelPos(position_);
  wrld.updateCollisionVertices(modelPos_);
}

//rotate the view around the camera position
void Camera::rotateView(float angle, Vector3D v){

  Vector3D newView;
  // Get the view vector
  Vector3D vView = view_ - position_;

  float cosTheta = cos(angle);
  float sinTheta = sin(angle);

  // find the new x position for the new rotated point
  newView.x  = (cosTheta + (1 - cosTheta) * v.x * v.x)	* vView.x;
  newView.x += ((1 - cosTheta) * v.x * v.y - v.z * sinTheta) * vView.y;
  newView.x += ((1 - cosTheta) * v.x * v.z + v.y * sinTheta) * vView.z;

  // find the new y position for the new rotated point
  newView.y  = ((1 - cosTheta) * v.x * v.y + v.z * sinTheta) * vView.x;
  newView.y += (cosTheta + (1 - cosTheta) * v.y * v.y)	* vView.y;
  newView.y += ((1 - cosTheta) * v.y * v.z - v.x * sinTheta) * vView.z;

  // Find the new z position for the new rotated point
  newView.z  = ((1 - cosTheta) * v.x * v.z - v.y * sinTheta) * vView.x;
  newView.z += ((1 - cosTheta) * v.y * v.z + v.x * sinTheta) * vView.y;
  newView.z += (cosTheta + (1 - cosTheta) * v.z * v.z)	* vView.z;

  view_ = position_ + newView;
}


//strafe the camera left and right depending on the speed
void Camera::strafeCamera(float speed){
  float step = 0;
  if (speed > 0){
    while (step < speed){
      //update position and view
      position_.x += strafe_.x * COLL_STEP;
      position_.z += strafe_.z * COLL_STEP;
      view_.x += strafe_.x * COLL_STEP;
      view_.z += strafe_.z * COLL_STEP;
    
      //get the nearest vertices and check them for camera collision
      cam.checkCameraCollision(wrld.getCollisionModels(), wrld.getCollisionVertices());
      step += COLL_STEP;
    }
  }
  else{
     while (step > speed){
      //update position and view
      position_.x += strafe_.x * -COLL_STEP;
      position_.z += strafe_.z * -COLL_STEP;
      view_.x += strafe_.x * -COLL_STEP;
      view_.z += strafe_.z * -COLL_STEP;
    
      //get the nearest vertices and check them for camera collision
      cam.checkCameraCollision(wrld.getCollisionModels(), wrld.getCollisionVertices());
      step -= COLL_STEP;
    }
  }

  //The position changed one field
  Vector2D temp = wrld.realToModelPos(position_);
  if (temp != modelPos_){
    game.setMoves(game.getMoves()-1);
    wrld.updateCollisionVertices(temp);
    game.setMoves(game.getMoves()+1);
    oldPos_ = modelPos_;
    modelPos_ = temp;
    update2D();
  }
}

//move the camera forward or backward depending on the speed
void Camera::moveCamera(float speed){
  // the current view vector
  Vector3D vVector = view_ - position_;
  vVector.y = 0;
  vVector = vVector.normalized()*SPEED_SCALE;

  float step = 0;
  if (speed > 0){
    while(step < speed){
      position_.x += vVector.x * COLL_STEP;
      position_.z += vVector.z * COLL_STEP;
      view_.x += vVector.x * COLL_STEP;
      view_.z += vVector.z * COLL_STEP;
    
      //get the nearest vertices and check them for camera collision
      cam.checkCameraCollision(wrld.getCollisionModels(), wrld.getCollisionVertices());
      step += COLL_STEP;
    }
  }
  else{
    while(step > speed){
      position_.x += vVector.x * -COLL_STEP;
      position_.z += vVector.z * -COLL_STEP;
      view_.x += vVector.x * -COLL_STEP;
      view_.z += vVector.z * -COLL_STEP;
    
      //get the nearest vertices and check them for camera collision
      cam.checkCameraCollision(wrld.getCollisionModels(), wrld.getCollisionVertices());
      step -= COLL_STEP;
    }

  }

  //The position changed one field
  Vector2D temp = wrld.realToModelPos(position_);
  if (temp != modelPos_){
    game.setMoves(game.getMoves()-1);
    wrld.updateCollisionVertices(temp);
    game.setMoves(game.getMoves()+1);
    oldPos_ = modelPos_;
    modelPos_ = temp;
    update2D();
  }
}

//move the camera forward or backward depending on the speed
void Camera::moveTo(float dist, const Vector3D dir){
  if (!plyr.yourTurn())
    return;
  
  Vector3D vVector = dir.normalized();

  position_.x += vVector.x * dist;
  position_.y += vVector.y * dist;
  position_.z += vVector.z * dist;
  view_.x += vVector.x * dist;
  view_.y += vVector.y * dist;
  view_.z += vVector.z * dist;

  //The position changed one field
  Vector2D temp = wrld.realToModelPos(position_);
  if (temp != modelPos_){
    game.setMoves(game.getMoves()-1);
    wrld.updateCollisionVertices(temp);
    game.setMoves(game.getMoves()+1);
    oldPos_ = modelPos_;
    modelPos_ = temp;
    //update2D();
  }
}



//checks all the polygons in list and resets the camera if collided
void Camera::checkCameraCollision(const std::list<CGE::Model*>& models, const CGE::Mesh* mesh){
  //Can happen directly after loading world, that this is NULL
  //because the world is loaded asynchronously within another thread
  //if (pVertices == NULL)
  //  return;
  // go through all given triangles
  //for(int i = 0; i < numOfVerts; i += 3){
    // the current triangle
    //Vector3D triangle[3] = { *pVertices[i], *pVertices[i+1], *pVertices[i+2] };
    //collisionHelper(triangle);
  //}
  for (std::list<CGE::Model*>::const_iterator iter = models.begin(); iter != models.end(); ++iter){
    CGE::Model* mdl = *iter;
    CGE::Mesh* msh = mdl->getMesh();
    for (int i = 0; i < msh->getNumTriangles(); ++i){
      Vector3D triangle[3];
      Vector3D* p = &triangle[0];
      msh->getTriangle(i, &p);
      for (int j = 0; j < 3; ++j){
        triangle[j] = mdl->getTrafo()*triangle[j];
      }
      collisionHelper(triangle);
    }
  }
  for (int i = 0; i < mesh->getNumTriangles(); ++i){
    Vector3D triangle[3];
    Vector3D* p = &triangle[0];
    mesh->getTriangle(i, &p);
    collisionHelper(triangle);
  }
}

void Camera::collisionHelper(const Vector3D* triangle){
  Vector3D normal = Maths::Normal(triangle);

  float distance = 0.0f;
  int classification = Maths::ClassifySphere(position_, normal, 
    triangle[0], radius_, distance);
  // If the sphere intersects the polygon's plane, then check further
  if(classification == INTERSECTS){
    //offset to the plane
    Vector3D offset = normal * distance;
    Vector3D intersection = position_ - offset;

    //collision?
    if(Maths::InsidePolygon(intersection, triangle, 3) ||
      Maths::EdgeSphereCollision(position_, triangle, 3, radius_ / 2)){

        //correct camera position
        offset = Maths::GetCollisionOffset(normal, radius_, distance);
        position_ = position_ + offset;
        view_ = view_ + offset;
    }
  }
}

// update camera
void Camera::update(){
  //strafe vector
  Vector3D cross = (view_ - position_).cross(upVector_);
  strafe_ = cross.normalized()*SPEED_SCALE;

  // Calculate frame rate for time based movement
  //gl->calculateFrameRate();
  
  //Print position
  char temp[100];
  sprintf(temp, "Coordinates(2D): x: %d y: %d", modelPos_.x, modelPos_.y);
  CGE::Engine::instance()->getFont(0)->setColor(0, 1, 0);
  CGE::Engine::instance()->getFont(0)->print(20, 700, temp, 0);
}


// set lookat of the camera
void Camera::look(){
  CGE::Engine::instance()->getRenderer()->lookAt(position_, view_, upVector_);
  //gluLookAt(position_.x, position_.y, position_.z,
	//    view_.x, view_.y, view_.z,
	//    upVector_.x, upVector_.y, upVector_.z);
}

// update movement on the map
void Camera::update2D(){
  if (modelPos_.x > oldPos_.x){
    msg.process("d");
  }
  if (modelPos_.x < oldPos_.x){
    msg.process("a");
  }
  if (modelPos_.y < oldPos_.y){
    msg.process("w");
  }
  if (modelPos_.y > oldPos_.y){
    msg.process("s");
  }
}

// find looking direction of the camera
Direction Camera::getLookDirection(){
  Vector3D view = view_ - position_;
  Direction dir;
  //looking mainly in x-direction
  if (fabs(view.x) > fabs(view.z)){
    if (view.x > 0)
      dir = RIGHT;
    else
      dir = LEFT;
  }
  //... y-direction
  else{
    if (view.z > 0)
      dir = BOTTOM;
    else
      dir = TOP;
  }
  return dir;
}