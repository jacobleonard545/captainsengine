#include "BlitObjects.h"

#include <system/engine.h>

#include "Engine.h"

#include <system/allocation.h>

TR_CHANNEL(ADV_Render_BlitObject);

using namespace adv;

#ifdef RENDER_TEGRA
#define glClearDepth glClearDepthf
#endif

BaseBlitObject::BaseBlitObject(int depth, const Vec2i& size) : 
mPos(), mSize(size), mDepth(depth){

}

BaseBlitObject::~BaseBlitObject(){

}

BlitObject::BlitObject(int width, int height, int depth) : 
BaseBlitObject(depth, Vec2i(width, height)), mOffset(), mScale(1.0f, 1.0f), mTex(0), mImage(NULL), mMirrorOffset(), mRotAngle(0.0f), mBlendMode(BLEND_ALPHA){
  Vec2i pow2(Engine::roundToPowerOf2(mSize.x), Engine::roundToPowerOf2(mSize.y));
  mScale.x = ((float)mSize.x)/pow2.x;
  mScale.y = ((float)mSize.y)/pow2.y;
  mZoomScale = Vec2f(1.0f,1.0f);
  mDeleteTex = true;
}

BlitObject::BlitObject(std::string texture, const Vec2i& size, int depth, Vec2i offset) : 
BaseBlitObject(depth, Vec2i()), mOffset(offset), mScale(1.0f, 1.0f), mTex(0), mMirrorOffset(), mRotAngle(0.0f), mBlendMode(BLEND_ALPHA){
  mImage = Engine::instance()->getImage(texture);
	if (mImage && size > Vec2i()){
		mImage->crop(-offset.x, -offset.y, size.x, size.y);
		if (mOffset.x < 0)
			mOffset.x = 0;
		if (mOffset.y < 0)
			mOffset.y = 0;
	}
  else if (mImage){
    mSize = Vec2i(mImage->getWidth(), mImage->getHeight());
  }
  mZoomScale = Vec2f(1.0f,1.0f);
  mDeleteTex = true;
}

BlitObject::BlitObject(CGE::Texture* texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset):
BaseBlitObject(depth, size), mOffset(offset), mScale(scale), mTex(texture), mImage(NULL), mMirrorOffset(), mRotAngle(0), mBlendMode(BLEND_ALPHA)
{
  mZoomScale = Vec2f(1.0f,1.0f);
  mDeleteTex = false;
}

BlitObject::~BlitObject(){
  delete mImage;
  if (mDeleteTex)
    delete mTex;
}

bool BlitObject::operator<(const BlitObject& obj){
  return mDepth < obj.mDepth;
}

void BlitObject::render(const Vec2i& pos, const Vec2f& scale, const Vec2i& parentsize){
  mMirrorOffset = parentsize;
  mPos.x = mOffset.x+pos.x;
  mPos.y = mOffset.y+pos.y;
  mZoomScale = scale;
  Engine::instance()->insertToBlit(this);
}

void BlitObject::blit(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  if (mBlendMode == BLEND_ADDITIVE)
    rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE);
  else if (mBlendMode == BLEND_PREMULT_ALPHA)
    rend->blendFunc(CGE::BLEND_ONE, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->pushMatrix();
  
  if (mZoomScale.x < 0){
    rend->translate(mMirrorOffset.x*-mZoomScale.x, 0.0f, 0.0f);
  }
  if (mZoomScale.y < 0){
    rend->translate(0.0f, mMirrorOffset.y*-mZoomScale.y, 0.0f);
  }

  rend->translate((float)mPos.x, (float)mPos.y, 0.0f);
  
  rend->translate((float)-mOffset.x, (float)-mOffset.y, 0.0f);
  rend->scale(mZoomScale.x, mZoomScale.y, 1.0f);
  rend->translate((float)mOffset.x, (float)mOffset.y, 0.0f);
  
  //TODO wrong for compound objects, maybe rotation of mOffset?
  rend->translate((float)mSize.x/2,(float)mSize.y/2, 0.0f);
  rend->rotate(mRotAngle, 0, 0, 1.0f);
  rend->translate((float)-mSize.x/2,(float)-mSize.y/2, 0.0f);

  rend->scale((float)mSize.x,(float)mSize.y,1.0f);
  
  rend->switchMatrixStack(CGE::MatTexture);
  rend->resetModelView();
  rend->scale(mScale.x, mScale.y, 1.0f);
  rend->switchMatrixStack(CGE::Modelview);

  mTex->activate();
  rend->setColor(mColor.r/255.0f, mColor.g/255.0f, mColor.b/255.0f, mColor.a/255.0f);
  Engine::instance()->drawQuad();
  rend->popMatrix();
  if (mBlendMode != BLEND_ALPHA)
    rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  mTex->deactivate();
}

BlitObject* BlitObject::clone(){
  BlitObject* obj = new BlitObject(mTex, mSize, mScale, mDepth, mOffset);
  return obj;
}

void BlitObject::updateTexture(unsigned width, unsigned height, void* data){
  mTex->update(0, 0, width, height, data);
}

void BlitObject::realize(){
  if (mImage == NULL)
    return;
  mTex = Engine::instance()->genTexture(mImage, mSize, mScale);
  delete mImage;
  mImage = NULL;
}

void BlitObject::realizeEmpty(CGE::Texture::Format format){
  Vec2i pow2(Engine::roundToPowerOf2(mSize.x), Engine::roundToPowerOf2(mSize.y));
  mTex = CGE::Engine::instance()->getRenderer()->createTexture(pow2.x, pow2.y, format);
  /*glGenTextures(1, &mTex);
  GLint mOldTex;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &mOldTex);
  glBindTexture(GL_TEXTURE_2D, mTex);
  glTexImage2D(GL_TEXTURE_2D, 0, format, pow2.x, pow2.y, 0, format, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, mOldTex);*/
}

LightingBlitObject::LightingBlitObject(int depth, const Vec2i& size) : BaseBlitObject(depth, size){
}

LightingBlitObject::~LightingBlitObject(){

}

void LightingBlitObject::render(const Vec2i& pos){
  mPos = pos;
  Engine::instance()->insertToBlit(this);
}

void LightingBlitObject::blit(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->enableTexturing(false);
  //glEnable(GL_BLEND);
  rend->blendFunc(CGE::BLEND_DST_COLOR, CGE::BLEND_ZERO);
  rend->pushMatrix();
  rend->translate((float)mPos.x,(float)mPos.y,0.0f);
  rend->scale((float)mSize.x,(float)mSize.y,1.0f);
  rend->setColor(mColor.r / 255.0f, mColor.g / 255.0f, mColor.b / 255.0f, mColor.a / 255.0f);
  Engine::instance()->drawQuad();
  rend->setColor(1.0f, 1.0f, 1.0f, 1.0f);
  //glDisable(GL_BLEND);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->popMatrix();
  rend->enableTexturing(true);
}

ScrollBlitObject::ScrollBlitObject(int depth) : BaseBlitObject(depth, Vec2i(0,0)){
}

ScrollBlitObject::~ScrollBlitObject(){
}

void ScrollBlitObject::blit(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  if (mDepth < 0){
    rend->pushMatrix();
    rend->translate((float)mPos.x, (float)mPos.y, 0);
  }
  else
    rend->popMatrix();
}

void ScrollBlitObject::render(const Vec2i& pos){
  mPos = pos;
}

RenderableBlitObject::RenderableBlitObject(int width, int height, int depth) : BlitObject(width,height,depth), mFrameBuffer(NULL){
}

void RenderableBlitObject::realize(){
  TR_USE(ADV_Render_BlitObject);
  BlitObject::realizeEmpty();
  int powx = (int)(mSize.x/mScale.x);
  int powy = (int)(mSize.y/mScale.y);

  mFrameBuffer = CGE::Engine::instance()->getRenderer()->createRenderTarget(powx, powy);
  mFrameBuffer->activate();
  mFrameBuffer->addTexture(*mTex);
  mFrameBuffer->addRenderbuffer(CGE::Texture::DEPTH);
  if (!mFrameBuffer->isComplete()){
    TR_ERROR("Unable to create framebuffer");
    //TR_BREAK();
  }
  mFrameBuffer->deactivate();
}

RenderableBlitObject::~RenderableBlitObject(){
  delete mFrameBuffer;
}

void RenderableBlitObject::bind(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->getViewport(mOldViewport);
  rend->viewport(0, 0, mSize.x, mSize.y);
  mFrameBuffer->activate();
}

void RenderableBlitObject::unbind(){
  mFrameBuffer->deactivate();
  CGE::Engine::instance()->getRenderer()->viewport(mOldViewport[0], mOldViewport[1], mOldViewport[2], mOldViewport[3]);
}

DynamicAnimation::DynamicAnimation(){

}

DynamicAnimation::~DynamicAnimation(){

}

MirrorObject::MirrorObject(int width, int height, int depth, unsigned char strength) : 
RenderableBlitObject(width, height, depth), mOpacity(strength), mIsWallMirror(false){
  mPolygon = CGE::Engine::instance()->getRenderer()->createVertexBuffer();
  mPolygon->create(VB_POSITION, 5);
  memset(mPoints, 0, 4 * sizeof(Vec2i));
}

MirrorObject::~MirrorObject(){
  delete mPolygon;
}

bool MirrorObject::update(unsigned interval){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  bind();
  //setup
  rend->enableTexturing(false);
  rend->setClearColor(CGE::Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
  rend->setClearDepth(0.0f);
  rend->clear(COLORBUFFER | ZBUFFER);
  rend->enableDepthTest(true);
  rend->depthFunc(CGE::CMP_GEQUAL);
  
  //draw mirror surface
  rend->enableColorWrite(false);
  rend->setColor(1.0f, 1.0f, 0.0f, 1.0f);
  rend->pushMatrix();
  rend->resetModelView();
  Engine::instance()->flipTexture();
  rend->pushMatrix();
  rend->translate((float)mRoom->getScrollOffset().x, (float)mRoom->getScrollOffset().y, 0.0f);
  mPolygon->activate();
  mPolygon->draw(CGE::VB_Tristrip, NULL);
  rend->popMatrix();
  rend->enableColorWrite(true);
  
  //render characters
  rend->enableDepthWrite(false);
  rend->depthFunc(CGE::CMP_EQUAL);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableTexturing(true);
  Engine::instance()->beginRendering();
  CharacterObject* self = Engine::instance()->getCharacter("self");
  if (self)
    renderCharacter(self);
  for (int i = (int)mRoom->getObjects().size()-1; i >= 0; --i){
    if (mRoom->getObjects()[i]->getType() == Object2D::CHARACTER){
      renderCharacter((CharacterObject*)mRoom->getObjects()[i]);
    }
  }
  Engine::instance()->endRendering(true);
  rend->enableDepthWrite(true);
  rend->enableDepthTest(false);

  //multiply alpha
  rend->scale((float)mSize.x,(float)mSize.y,1.0f);
  rend->blendFunc(CGE::BLEND_DST_COLOR, CGE::BLEND_ZERO);
  rend->setColor(1.0f, 1.0f, 1.0f, mOpacity / 255.0f);
  rend->enableTexturing(false);
  Engine::instance()->drawQuad();
  //GL()drawArrays(GL_TRIANGLE_STRIP, 0, 4);
  rend->popMatrix();
  rend->enableTexturing(true);

  //restore
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  Engine::instance()->restoreRenderDefaults();
  unbind();
  return true;
}

void MirrorObject::setMirrorArea(Vec2i points[4], RoomObject* room){
  mRoom = room;
  int xmin = 1000; int xmax = -1000;
  for (int i = 0; i < 4; ++i){
    if (points[i].x > xmax)
      xmax = points[i].x;
    if (points[i].x < xmin)
      xmin = points[i].x;
  }
  mMirrorCenter = (xmin + xmax) / 2;
  memcpy(mPoints, points, 4 * sizeof(Vec2i));
}

void MirrorObject::renderCharacter(CharacterObject* chr){
  if (mIsWallMirror){
    if (chr->getDepth() < mDepth)
      return;
    int oldstate = chr->getState();
    Vec2i oldpos = chr->getPosition();
    chr->updateState(true, false);
    Vec2i newpos = chr->getPosition();
    int mirrorbase = (mDepth+1)*(int)Engine::instance()->getWalkGridSize(false);
    newpos.y = mirrorbase + mMirrorOffset.y;
    int diff = newpos.y-oldpos.y;
    newpos.y += diff;
    if (mPositionDependent){
      int xdiff = mMirrorCenter-newpos.x;
      float diffadapt = float(chr->getPosition().y-mDepth*Engine::instance()->getWalkGridSize(false));
      diffadapt /= chr->getWalkGridSize();
      diffadapt = 10 - diffadapt;
      if (diffadapt <= 2)
        diffadapt = 2;
      newpos.x += (int)(xdiff/diffadapt);
    }
    else{
      newpos.x += mMirrorOffset.x;
    }
    chr->setPosition(newpos);
    chr->render();
    chr->setPosition(oldpos);
    chr->setState(oldstate);
  }
  else{
    chr->render(true);
  }
}

void MirrorObject::setWallMirror(Vec2i offset, bool positionDependent){
  mMirrorOffset = offset;
  mPositionDependent = positionDependent;
  mIsWallMirror = true;
}

void MirrorObject::realize(){
  RenderableBlitObject::realize();
  mPolygon->lockVertexPointer();
  mPolygon->setPosition(0, CGE::Vec3f((float)mPoints[1].x, (float)mPoints[1].y, 0.0f));
  mPolygon->setPosition(1, CGE::Vec3f((float)mPoints[2].x, (float)mPoints[2].y, 0.0f));
  mPolygon->setPosition(2, CGE::Vec3f((float)mPoints[0].x, (float)mPoints[0].y, 0.0f));
  mPolygon->setPosition(3, CGE::Vec3f((float)mPoints[3].x, (float)mPoints[3].y, 0.0f));
  mPolygon->setPosition(4, CGE::Vec3f((float)mPoints[0].x, (float)mPoints[0].y, 0.0f));
  mPolygon->unlockVertexPointer();
}
