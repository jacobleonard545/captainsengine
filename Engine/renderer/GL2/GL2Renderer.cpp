#define _USE_MATH_DEFINES
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "../../window/nativeWindows.h"
#include "../../window/nativeLinux.h"
#include "../../window/nativeAndroid.h"
#include "../../system/engine.h"
#if defined QNX || defined ANDROID
#include <GLES2/gl2.h>
#define glClearDepth glClearDepthf
#else
#include <GL/glew.h>
#endif
//#include <GL/gl.h>
#include "GL2Renderer.h"
#include "GL2vertexbuffer.h"
#include "../OpenGL/OGLtexture.h"
#include "GL2indexbuffer.h"
#include "GL2Shader.h"
#include "GL2rendertarget.h"

using namespace CGE;

TR_CHANNEL(CGE_Renderer_GL2);

GL2Renderer::GL2Renderer(): Renderer(), mShader(NULL) {
  type_ = OpenGL2;
#ifdef WIN32
  hDC_ = NULL;
  hRC_ = NULL;
#endif
#if defined UNIX && !defined QNX && !defined ANDROID
  glx_ = NULL;
#endif
#if ANDROID
  m_ctx = EGL_NO_CONTEXT;
#endif
  for (int i = 0; i < 3; ++i){
    mMatrix[i] = CGE::Matrix(CGE::Matrix::Identity);
  }
  mMatrixMode = Modelview;
}

GL2Renderer::~GL2Renderer(){
#if defined(WIN32)
  wglDeleteContext(hRC_);
#endif
}

void GL2Renderer::initContext(AppWindow* win){
  TR_USE(CGE_Renderer_GL2);
  TR_INFO("Initializing OpenGL context");
  win_ = win;
#if defined(WIN32) && !defined(UNDER_CE)
  static PIXELFORMATDESCRIPTOR pfd ={
    sizeof(PIXELFORMATDESCRIPTOR),
      1,
      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
      PFD_TYPE_RGBA,
      win->getBpp(),
      0, 0, 0, 0, 0, 0,   //Color bits ignored
      0,					//No Alpha buffer
      0,					//shift bit ignored
      0,					//no accumulation buffer
      0, 0, 0, 0,			//accumulation bits ignored
      16,					//Z-buffer depth
      0,					//no stencil buffer
      0,					//no auxiliary buffer
      PFD_MAIN_PLANE,
      0,					//Reserved
      0, 0, 0				//Layer masks ignored
  };

  HWND wnd = (HWND)static_cast<WindowsWindow*>(win)->getHandle();
  if(!(hDC_ = GetDC(wnd))){
    TR_ERROR("Can't create GL device context");
    EXIT();
  }

  GLuint pixelFormat;
  if(!(pixelFormat = ChoosePixelFormat(hDC_, &pfd))){
    TR_ERROR("Can't find a suitable PixelFormat");
    EXIT();
  }

  if (!SetPixelFormat(hDC_, pixelFormat, &pfd)){
    TR_ERROR("Can't set the PixelFormat");
    EXIT();
  }

  if (!(hRC_ = wglCreateContext(hDC_))){
    TR_ERROR("Can't create GL rendering context");
    EXIT();
  }

  if (!wglMakeCurrent(hDC_, hRC_)){
    TR_ERROR("Cant't activate GL rendering context");
    EXIT();
  }

#endif
#if defined UNIX && !defined QNX && !defined ANDROID
  X11Window* x11 = static_cast<X11Window* >(win_);
  glXMakeCurrent(x11->getDisplay(), x11->getWindow(), glx_);
#endif

#if !defined QNX && !defined ANDROID
  GLenum err = glewInit();
  if (err != GLEW_OK){
    TR_ERROR("Unable to init OpenGL extensions");
    CGE::Engine::instance()->requestShutdown();
    return;
  }
  if (!GLEW_VERSION_2_0){
    TR_ERROR("OpenGL 2.0 not available");
    CGE::Engine::instance()->requestShutdown();
    return;
  }
#endif

#ifdef ANDROID
  AndroidWindow* andr = static_cast<AndroidWindow* >(win_);
  
      if (eglMakeCurrent(andr->getDisplay(), andr->getSurface(), andr->getSurface(), m_ctx) == EGL_FALSE) {
        TR_ERROR("Unable to eglMakeCurrent");
        return;
    }
    TR_INFO("Made egl context current");
#endif

#ifdef WIN32
  resizeScene(win->getWidth(), win->getHeight());
#endif
  //initRendering();  
  
}

void GL2Renderer::killContext(){
  TR_USE(CGE_Renderer_GL2);
  delete mRT;
  mShader->deactivate();
  delete mShader;
  delete mLightShader;
#if defined(WIN32) && !defined(UNDER_CE)
  if (hRC_){
    if (!wglMakeCurrent(NULL,NULL)){
      TR_ERROR("Release of GL context failed");
    }
    if (!wglDeleteContext(hRC_)){
      TR_ERROR("Release of rendering context failed");
    }
    hRC_ = NULL;
  }

  HWND wnd = (HWND)static_cast<WindowsWindow*>(win_)->getHandle();
  if (hDC_ && !ReleaseDC(wnd,hDC_)){
    TR_ERROR("Release of device context failed");
    hDC_ = NULL;
  }
#endif
#if defined UNIX && !defined QNX &&  !defined ANDROID
  X11Window* x11 = static_cast<X11Window* >(win_);
  if (glx_){
    if (!glXMakeCurrent(x11->getDisplay(), None, NULL)){
      TR_ERROR("Release of GL context failed");
    }
    glXDestroyContext(x11->getDisplay(), glx_);
    glx_ = NULL;
  }
#endif
#ifdef ANDROID
  AndroidWindow* andr = static_cast<AndroidWindow* >(win_);
  
  eglMakeCurrent(andr->getDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (m_ctx != EGL_NO_CONTEXT) {
            eglDestroyContext(andr->getDisplay(), m_ctx);
        }
   m_ctx = EGL_NO_CONTEXT; 
#endif
}

static char const * vs_src_unlit =
"attribute vec3 pos;\n"
"attribute vec4 color;\n"
"attribute vec2 texcoord;\n"
"attribute vec3 normal;\n"
"\n"
"uniform mat4 mvp;\n"
"uniform mat4 texmat;\n"
"\n"
"varying vec2 uvcoord;\n"
"varying vec4 vcolor;\n"
"\n"
"void main(){\n"
"  uvcoord = (texmat*vec4(texcoord,1,1)).xy;\n"
"  vcolor = color;\n"
"  gl_Position = mvp*vec4(pos, 1.0);\n"
"}\n"
"";

static char const * fs_src_unlit =
#ifdef RENDER_EMBEDDED
"precision mediump float;\n"
#endif
"uniform sampler2D texture;\n"
"uniform bool textureEnabled;\n"
"\n"
"varying vec2 uvcoord;\n"
"varying vec4 vcolor;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  if (textureEnabled)\n"
"     color = texture2D(texture, uvcoord);\n"
"  gl_FragColor = color*vcolor;\n"
"}\n"
"";

namespace CGE{
  class LightGL2Shader : public GL2Shader{
    virtual bool linkShaders(){
      bool ret = GL2Shader::linkShaders();
      mTexEnabledLoc = getUniformLocation(FRAGMENT_SHADER, "textureEnabled");
      mColorLoc = getUniformLocation(FRAGMENT_SHADER, "matDiffuse");
      mAmbientLoc = getUniformLocation(FRAGMENT_SHADER, "matAmbient");
      mShininessLoc = getUniformLocation(FRAGMENT_SHADER, "matShininess");
      mSpecularLoc = getUniformLocation(FRAGMENT_SHADER, "matSpecular");
      mLightPosLoc = getUniformLocation(FRAGMENT_SHADER, "lightPos");
      mLightDirLoc = getUniformLocation(FRAGMENT_SHADER, "lightDir");
      mLightCutoffLoc = getUniformLocation(FRAGMENT_SHADER, "lightCutoff");
      mLightColorLoc = getUniformLocation(FRAGMENT_SHADER, "lightColor");
      mLightAttenuationLoc = getUniformLocation(FRAGMENT_SHADER, "lightAttenuation");
      return ret;
    }
    virtual bool applyMaterial(Material const& mat){
      lockUniforms(FRAGMENT_SHADER);
      Color diff = mat.getDiffuse();
      diff.a = mat.getOpacity();
      uniform(mColorLoc, diff);
      uniform(mAmbientLoc, mat.getAmbient());
      uniform(mShininessLoc, mat.getPower());
      uniform(mSpecularLoc, mat.getSpecular());
      Texture const* tex = mat.getDiffuseTex();
      if (!tex)
        tex = mat.getOpacityTex();
      if (tex){
        uniform(mTexEnabledLoc, 1);
        tex->activate();
      }
      else
        uniform(mTexEnabledLoc, 0);
      unlockUniforms(FRAGMENT_SHADER);
      return true;
    }
    virtual void applyLight(int number, Light const& light){
      lockUniforms(FRAGMENT_SHADER);
      Vec4f pos = light.getPosition();
      uniform(mLightPosLoc + number, pos.x, pos.y, pos.z, pos.w);
      uniform(mLightDirLoc + number, light.getDirection());
      uniform(mLightCutoffLoc + number, light.getCutoff() / 180.0f*(float)M_PI);
      uniform(mLightAttenuationLoc + number, light.getAttenuation());
      uniform(mLightColorLoc + number, light.getColor());
      unlockUniforms(FRAGMENT_SHADER);
    }

    int mColorLoc;
    int mAmbientLoc;
    int mSpecularLoc;
    int mTexEnabledLoc;
    int mShininessLoc;
    
    int mLightPosLoc;
    int mLightDirLoc;
    int mLightColorLoc;
    int mLightCutoffLoc;
    int mLightAttenuationLoc;
  };
}

static char const * vs_src_light =
"attribute vec3 pos;\n"
"attribute vec4 color;\n"
"attribute vec2 texcoord;\n"
"attribute vec3 normal;\n"
"\n"
"uniform mat4 mvp;\n"
"uniform mat4 texmat;\n"
"uniform mat4 mvmat;\n"
"uniform mat4 normalmat;\n"
"\n"
"varying vec2 uvcoord;\n"

"varying vec3 vnormal;\n"
"varying vec3 vpos;\n"
"\n"
"void main(){\n"
"  uvcoord = (texmat*vec4(texcoord,1,1)).xy;\n"
"\n"
"  vnormal = normalize((normalmat * vec4(normal, 0.0)).xyz);\n"
"  vpos = (mvmat*vec4(pos, 1.0)).xyz;\n"
"\n"
"  gl_Position = mvp*vec4(pos, 1.0);\n"
"}\n"
"";

static char const * fs_src_light =
#ifdef RENDER_EMBEDDED
"precision mediump float;\n"
#endif
"#define NUM_LIGHTS 8\n"
"uniform sampler2D texture;\n"
"uniform bool textureEnabled;\n"
"\n"
"varying vec2 uvcoord;\n"
"\n"
"varying vec3 vnormal;\n"
"varying vec3 vpos;\n"
"\n"
"uniform int numLights;\n"
"uniform vec4 lightPos[NUM_LIGHTS];\n"
"uniform vec3 lightDir[NUM_LIGHTS];\n"
"uniform vec4 lightColor[NUM_LIGHTS];\n"
"uniform float lightCutoff[NUM_LIGHTS];\n"
"uniform float lightAttenuation[NUM_LIGHTS];\n"
"\n"
"uniform vec4 matAmbient;\n"
"uniform vec4 matDiffuse;\n"
"uniform vec4 matSpecular;\n"
"uniform float matShininess;\n"
"\n"
"void main(){\n"
"  vec4 color = matDiffuse;\n"
"  if (textureEnabled)\n"
"     color *= texture2D(texture, uvcoord);\n"
"\n"
"  vec3 normal = normalize(vnormal);\n"
"  vec3 eye = normalize(-vpos);\n"
"  vec3 diffuse = vec3(0.0,0.0,0.0);\n"
"  vec3 specular = vec3(0.0,0.0,0.0);\n"
"  vec3 ambient = vec3(0.0,0.0,0.0);\n"
#ifdef RENDER_EMBEDDED
"  for (int i = 0; i <= NUM_LIGHTS; ++i){\n"
"    if (i < numLights){\n"
#else
"  for (int i = 0; i < numLights; ++i){\n"
#endif
"    vec3 lightvec;\n"
"    float att = 1.0;\n"
"    if (lightPos[i].w == 0.0)\n"
"     lightvec = normalize(lightPos[i].xyz);\n"
"    else{\n"
"      lightvec = normalize(lightPos[i].xyz - vpos);\n"
"      float lightAngle = acos(dot(-lightvec, normalize(lightDir[i])));\n"
"      if (lightAngle > lightCutoff[i]){\n"
"        att = 0.0;\n"
"      }\n"
"      else{\n"
"        float lightDist = length(lightPos[i].xyz-vpos);\n"
"        att = 1.0/(1.0+lightAttenuation[i]*pow(lightDist, 2.0));\n"
"      }\n"
"    }\n"
"    vec3 refl = normalize(reflect(-lightvec, normal));\n"
"    float NL = max(dot(normal,lightvec), 0.0);\n"
"    float spec = 0.0;\n"
"    if (NL > 0.0)\n"
"      spec = pow(max(dot(refl, eye), 0.0), matShininess);\n"
"    \n"
"    diffuse += lightColor[i].rgb*NL*att;\n"
"    specular += lightColor[i].rgb*spec*att;\n"
"    ambient += lightColor[i].rgb*matAmbient.rgb;\n"
#ifdef RENDER_EMBEDDED
"    }\n"
#endif
"  }\n"
"  vec4 finalColor = vec4(color.rgb*(ambient + diffuse) + specular*matSpecular.rgb, color.a);\n"
"  gl_FragColor = finalColor;\n"
//"  gl_FragColor = vec4(color.rgb*diffuse,1);\n"
//"  gl_FragColor = vec4(-lightDir,1);\n"
//"  gl_FragColor = vec4(lightAngle, lightAngle, lightAngle, 1.0);\n"
"}\n"
"";

void GL2Renderer::initRendering(){
  TR_USE(CGE_Renderer_GL2);
  TR_INFO("Initializing Scene");

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //depth buffer
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  mRT = new GL2RenderTarget();
  RenderTarget::mCurrTarget = mRT;

  mLightShader = new CGE::LightGL2Shader();
  mLightShader->addShader(Shader::VERTEX_SHADER, vs_src_light);
  mLightShader->addShader(Shader::FRAGMENT_SHADER, fs_src_light);
  mLightShader->linkShaders();
  mLightShader->syncMatrix("mvp", CGE::MVP);
  mLightShader->syncMatrix("texmat", CGE::MatTexture);
  mLightShader->syncMatrix("mvmat", CGE::Modelview);
  mLightShader->syncMatrix("normalmat", CGE::MatNormal);
  mLightShader->activate();
  Material tmp("init");
  mLightShader->applyMaterial(tmp);
  int tex = mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "texture");
  mLightShader->uniform(tex, 0);//texture (uniform 32) at stage 0
  mLightShader->deactivate();

  mShader = new CGE::GL2Shader();
  mShader->addShader(Shader::VERTEX_SHADER, vs_src_unlit);
  mShader->addShader(Shader::FRAGMENT_SHADER, fs_src_unlit);
  mShader->linkShaders();
  mShader->syncMatrix("mvp", CGE::MVP);
  mShader->syncMatrix("texmat", CGE::MatTexture);
  mShader->activate();
  tex = mShader->getUniformLocation(Shader::FRAGMENT_SHADER, "texture");
  mShader->uniform(tex, 0);//texture (uniform 32) at stage 0

  Renderer::initRendering();
}

void GL2Renderer::renderScene(){
  Renderer::renderScene();
}

void GL2Renderer::resizeScene(int width, int height){
  TR_USE(CGE_Renderer_GL2);
#ifdef WIN32
  if (hRC_ == NULL)
    return;
#endif
  TR_INFO("Resizing Scene");
  if (height == 0){
    height = 1;
  }
  win_->setWidth(width);
  win_->setHeight(height);

  glViewport(0, 0, width, height);

  Renderer::resizeScene(width, height);
}

//! clear scene
void GL2Renderer::setClearColor(const Vec4f& color){
  glClearColor(color.x, color.y, color.z, color.w);
}

void GL2Renderer::setClearDepth(float depth){
  glClearDepth(depth);
}

void GL2Renderer::clear(long flags){
  long glflags = 0;
  if (flags & ZBUFFER)
    glflags |= GL_DEPTH_BUFFER_BIT;
  if (flags & COLORBUFFER)
    glflags |= GL_COLOR_BUFFER_BIT;
  glClear(glflags);
}

//! get a vertex buffer
VertexBuffer* GL2Renderer::createVertexBuffer(){
  return new GL2VertexBuffer();
}

Texture* GL2Renderer::createTexture(){
  OGLTexture* tex = new OGLTexture();
  return tex;
}

IndexBuffer* GL2Renderer::createIndexBuffer(IndexBuffer::Type t, uint32 size){
  return new GL2IndexBuffer(t,size);
}

RenderTarget* GL2Renderer::createRenderTarget(unsigned width, unsigned height){
  GL2RenderTarget* rt = new GL2RenderTarget(width, height);
  return rt;
}

Shader* GL2Renderer::createShader(){
  GL2Shader* sh = new GL2Shader();
  return sh;
}

void GL2Renderer::lookAt(const Vector3D& position, const Vector3D& look, const Vector3D& up){
  Vec3f forward = look - position;
  forward.normalize();
  Vec3f side = forward.cross(up).normalized();
  Vec3f up_new = side.cross(forward);
  Matrix mat = Matrix(side, up_new, forward*-1, Vec3f()/*eye*1*/)*Matrix(Matrix::Translation,position*-1);
  multiplyMatrix(mat);
}

//! set projection
void GL2Renderer::projection(float angle, float aspect, float nearplane, float farplane){ 
  GLfloat ymax = nearplane * (GLfloat)tan(angle * 3.1415962f / 360.0);
  GLfloat ymin = -ymax;
  GLfloat xmin = ymin * aspect;
  GLfloat xmax = ymax * aspect;
  mMatrix[Projection] = CGE::Matrix(CGE::Matrix::Perspective, xmin, xmax, ymin, ymax, nearplane, farplane);
}

void GL2Renderer::ortho(float left, float right, float bottom, float top, float nearp, float farp){
  mMatrix[Projection] = CGE::Matrix(CGE::Matrix::Ortho, left, right, bottom, top, nearp, farp);
}

//! reset modelview matrix
void GL2Renderer::resetModelView(){
  mMatrix[mMatrixMode] = Matrix(Matrix::Identity);
}

//! translate
void GL2Renderer::translate(float x, float y, float z){
  mMatrix[mMatrixMode] *= Matrix(Matrix::Translation, Vec3f(x, y, z));
}

//! scale
void GL2Renderer::scale(float x, float y, float z){
  mMatrix[mMatrixMode] *= Matrix(Matrix::Scale, Vec3f(x, y, z));
}

//! rotate
void GL2Renderer::rotate(float angle, float x, float y, float z){
  mMatrix[mMatrixMode] *= Matrix(Matrix::Rotation, Vec3f(x, y, z), angle/180*(float)M_PI);
}

//! set rendermode
void GL2Renderer::renderMode(RendMode rm){
  /*if (rm == Filled){
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  else if(rm == Wireframe){
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }*/
}

//! set blending mode
void GL2Renderer::blendFunc(BlendType src, BlendType dest){
  int srcfl = 0;
  int destfl = 0;
  switch(src){
    case BLEND_ONE:
      srcfl = GL_ONE;
      break;
    case BLEND_SRC_ALPHA:
      srcfl = GL_SRC_ALPHA;
      break;
    case BLEND_DST_COLOR:
      srcfl = GL_DST_COLOR;
      break;
    default:
      break;
  }
  switch(dest){
    case BLEND_ONE:
      destfl = GL_ONE;
      break;
    case BLEND_ZERO:
      destfl = GL_ZERO;
      break;
    case BLEND_ONE_MINUS_SRC_ALPHA:
      destfl = GL_ONE_MINUS_SRC_ALPHA;
      break;
    default:
      break;
  }
  glBlendFunc(srcfl, destfl);
}

//! enable blending
void GL2Renderer::enableBlend(const bool flag){
  if (flag)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
}

void GL2Renderer::enableBackFaceCulling(const bool flag){
  if (flag)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
}

void GL2Renderer::enableColorWrite(bool flag){
  if (flag)
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  else
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

//! enable texturing
void GL2Renderer::enableTexturing(const bool flag){
  int texEn = ((GL2Shader*)Shader::getCurrentShader())->getUniformLocation(Shader::FRAGMENT_SHADER, "textureEnabled");
  if (flag)
    ((GL2Shader*)Shader::getCurrentShader())->uniform(texEn, 1);
  else
    ((GL2Shader*)Shader::getCurrentShader())->uniform(texEn, 0);
}

// enable lighting
void GL2Renderer::enableLighting(const bool flag){
  if (flag && Shader::getCurrentShader() != mLightShader){
    Shader::getCurrentShader()->deactivate();
    mLightShader->activate();
  }
  else if (!flag && Shader::getCurrentShader() != mShader){
    Shader::getCurrentShader()->deactivate();
    mShader->activate();
  }
}

//! enable depth test
void GL2Renderer::enableDepthTest(const bool flag){
  if (flag)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
}

//! enable depth write
void GL2Renderer::enableDepthWrite(bool flag){
  glDepthMask(flag ? GL_TRUE : GL_FALSE);
}

void GL2Renderer::depthFunc(CompareFunc func){
  GLenum dfunc = GL_INVALID_ENUM;
  switch (func){
  case CMP_EQUAL:
    dfunc = GL_EQUAL;
    break;
  case CMP_GEQUAL:
    dfunc = GL_GEQUAL;
    break;
  }
  glDepthFunc(dfunc);
}

//! set color
void GL2Renderer::setColor(float r, float g, float b, float a){
  if (Shader::getCurrentShader() == mLightShader){
    Material tmp("setColor");
    tmp.setDiffuse(Color(r,g,b,a));
    mLightShader->applyMaterial(tmp);
  }
  else
    glVertexAttrib4f(1, r, g, b, a);
}

//! set color
void GL2Renderer::setColor(const Color* c){
  if (Shader::getCurrentShader() == mLightShader){
    Material tmp("setColor");
    tmp.setDiffuse(*c);
    mLightShader->applyMaterial(tmp);
  }
  else
    glVertexAttrib4fv(1, c->array);
}

//! push matrix
void GL2Renderer::pushMatrix(){
  mMatrixStack[mMatrixMode].push(mMatrix[mMatrixMode]);
}

//! pop matrix
void GL2Renderer::popMatrix(){
  mMatrix[mMatrixMode] = mMatrixStack[mMatrixMode].top();
  mMatrixStack[mMatrixMode].pop();
}

//! multiply matrix
void GL2Renderer::multiplyMatrix(const CGE::Matrix& mat){
  mMatrix[mMatrixMode] *= mat;
}

//! set material
/*void GL2Renderer::setMaterial(const Material& mat){
  Color diff = mat.getDiffuse();
  diff.a = mat.getOpacity();
  if (mat.getOpacity() != 1.0f)
    enableBlend(true);
  else
    enableBlend(false);
  setColor(&diff);
  Texture const* tex = mat.getDiffuseTex();
  if (tex){
    enableTexturing(true);
    tex->activate();
  }
  else
    enableTexturing(false);
}*/

//! set the viewport
void GL2Renderer::viewport(int x, int y, int w, int h){
  glViewport(x, y, w, h);
}

//! get the viewport
void GL2Renderer::getViewport(int view[4]){
  glGetIntegerv(GL_VIEWPORT, view);
}

//! get a matrix
Matrix GL2Renderer::getMatrix(MatrixType mt){
  if (mt == MVP){
    return mMatrix[Projection] * mMatrix[Modelview];
  }
  else if (mt == MatNormal){
    return mMatrix[Modelview].inverse().transpose();
  }
  return mMatrix[mt];
}

void GL2Renderer::swapBuffers(){
#ifdef WIN32
  SwapBuffers(hDC_);
#endif
#if defined UNIX && !defined QNX && !defined ANDROID
  X11Window* win = static_cast<X11Window*>(win_);
  glXSwapBuffers(win->getDisplay(), win->getWindow());
#endif
#ifdef ANDROID
  AndroidWindow* win = static_cast<AndroidWindow*>(win_);
  eglSwapBuffers(win->getDisplay(), win->getSurface());
#endif
}

void GL2Renderer::switchMatrixStack(MatrixType type){
  mMatrixMode = type;
}


void GL2Renderer::scissor(int x, int y, unsigned w, unsigned h){
  glScissor(x, y, w, h);
}

void GL2Renderer::enableScissorTest(bool flag){
  if (flag)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);
}