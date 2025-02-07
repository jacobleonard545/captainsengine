#ifndef CGE_GL2_RENDERTARGET_H
#define CGE_GL2_RENDERTARGET_H

#include <renderer/rendertarget.h>
#if !defined QNX && !defined ANDROID
#include <GL/glew.h>
#else
#include <GLES2/gl2.h>
#endif
#include <vector>

namespace CGE{

class GL2RenderTarget : public RenderTarget{
public:
  GL2RenderTarget();
  GL2RenderTarget(unsigned width, unsigned height);
  ~GL2RenderTarget();
  virtual void addTexture(Texture::Format fmt);
  virtual void addTexture(const CGE::Texture& tex);
  virtual void addRenderbuffer(Texture::Format fmt);
  virtual bool isComplete();
  virtual Texture* getTexture(unsigned idx);
  virtual unsigned getNumTextures() { return (unsigned)mTextures.size(); }
protected:
  virtual void bind();
private:
  unsigned mWidth;
  unsigned mHeight;
  GLuint mFBO;
  std::vector<Texture*> mTextures;
  std::vector<GLuint> mRenderbuffers;
  unsigned mCTCount;
};

}

#endif