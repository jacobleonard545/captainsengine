#if defined(LINUX) && !defined(NO_X11)
#include "../renderer/GL2/GL2Renderer.h"
#include <GL/glx.h>
#include "../system/engine.h"
#include "../renderer/OpenGL/OGLrenderer.h"
//#include "../input/keyboard.h"
#include "nativeLinux.h"

TR_CHANNEL_EXT(CGE_Window);

using namespace CGE;

X11Window::X11Window(CGE::Renderer* renderer) : AppWindow(renderer){
  disp_ = NULL;
  win_ = 0;
}

void X11Window::init(const std::string& name){
  TR_USE(CGE_Window);
  TR_INFO("Initializing window");

  disp_ = XOpenDisplay(0);
  screen_ = DefaultScreen(disp_);

  int attribs[] = {GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 4,
    GLX_GREEN_SIZE, 4,
    GLX_BLUE_SIZE, 4,
    GLX_DEPTH_SIZE, 16,
    None};
  XVisualInfo* vi = glXChooseVisual(disp_, screen_, attribs);
  if (vi == NULL)
    EXIT2("Got no doublebuffered window");

  int glxMajor;
  int glxMinor;
  glXQueryVersion(disp_, &glxMajor, &glxMinor);
  
  XSetWindowAttributes attr;
  GLXContext glx = glXCreateContext(disp_, vi, 0, GL_TRUE);
  if (renderer_->getRenderType() == CGE::OpenGL)
    static_cast< CGE::OGLRenderer* >(renderer_)->setGLX(glx);
  else if (renderer_->getRenderType() == CGE::OpenGL2)
    static_cast< CGE::GL2Renderer* >(renderer_)->setGLX(glx);
  else
    TR_ERROR("Unsupported renderer");
  Colormap color = XCreateColormap(disp_, RootWindow(disp_, vi->screen),
      vi->visual, AllocNone);
  attr.colormap = color;
  attr.border_pixel = 0;
  
  if (shown_)
    applyResolution(); 
  
  if (fullscreen_){
    attr.override_redirect = True;
    attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;
    win_ = XCreateWindow(disp_, RootWindow(disp_, vi->screen), 0, 0,
        width_, height_, 0, vi->depth, InputOutput, vi->visual,
        CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect,
        &attr);
    XWarpPointer(disp_, None, win_, 0, 0, 0, 0, 0, 0);
    XMapRaised(disp_, win_);
    XGrabKeyboard(disp_, win_, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    XGrabPointer(disp_, win_, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync,
        GrabModeAsync, win_, None, CurrentTime);
  }
  else{
    attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;
    win_ = XCreateWindow(disp_, RootWindow(disp_, vi->screen), 0, 0,
        width_, height_, 0, vi->depth, InputOutput, vi->visual,
        CWBorderPixel | CWColormap | CWEventMask, &attr);
    Atom wmDelete = XInternAtom(disp_, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(disp_, win_, &wmDelete, 1);
    XSetStandardProperties(disp_, win_, name.c_str(), name.c_str(), 
        None, NULL, 0, NULL);
    if (shown_)
      show(true);
  }

  glx = NULL;
  renderer_->initContext(this);
  renderer_->resizeScene(width_, height_);
}

void X11Window::applyResolution(){
  if (fullscreen_){
    int videoMajor;
    int videoMinor;
    XF86VidModeQueryVersion(disp_, &videoMajor, &videoMinor);

    int numModes;
    XF86VidModeModeInfo** modes;
    XF86VidModeGetAllModeLines(disp_, screen_, &numModes, &modes);
    static bool first = true;
    if (first){
      prevDesktop_ = *modes[0];
      first = false;
    }
  
    //search for best resolution
    int bestMode = 0;
    for (int i = 0; i < numModes; i++){
      if (modes[i]->hdisplay == width_ && modes[i]->vdisplay == height_){
        bestMode = i;
      }
    }

    //Change video mode
    XF86VidModeSwitchToMode(disp_, screen_, modes[bestMode]);
    XF86VidModeSetViewPort(disp_, screen_, 0, 0);
    width_ = modes[bestMode]->hdisplay;
    height_ = modes[bestMode]->vdisplay;
    XFree(modes);
 
  }
}

void X11Window::kill(){
  TR_USE(CGE_Window);
  TR_INFO("Killing window");

  renderer_->killContext();

  if (fullscreen_){
    XF86VidModeSwitchToMode(disp_, screen_, &prevDesktop_);
    XF86VidModeSetViewPort(disp_, screen_, 0, 0);
  }
  
  XCloseDisplay(disp_);
}


void X11Window::changeSize(int width, int height){
  AppWindow::changeSize(width, height);
  XResizeWindow(disp_, win_, width, height);
  applyResolution();
}

void X11Window::show(bool doit){
  if (doit){
    XMapRaised(disp_, win_);
  }
  else
    XUnmapWindow(disp_, win_);
  AppWindow::show(doit);
}


#endif
