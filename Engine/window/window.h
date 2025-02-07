#ifndef WINDOW_H
#define WINDOW_H

#include <string>

namespace CGE{
class Renderer;
}

namespace CGE{

#define WINDOW_NAME "Captains Game Engine"

class AppWindow{
public:
  AppWindow(CGE::Renderer* render);
  virtual ~AppWindow(){};
  virtual void init(const std::string& name){};
  virtual void kill(){};
  //! get Bits per pixel
  inline const int getBpp() const {return bpp_;}
  //! is window fullscreen?
  inline const bool isFullscreen() const {return fullscreen_;}
  //! get window width
  inline const int getWidth() const {return width_;}
  //! set window width
  inline void setWidth(int width) {width_ = width;}
  //! get window height
  inline const int getHeight() const {return height_;}
  //! set window height
  inline void setHeight(int height) {height_ = height;}
  //! set focussed
  void focus(bool hasFocus);
	//! get a native window handle
	virtual void* getHandle() {return NULL;}

  virtual void changeSize(int width, int height){
    width_ = width;
    height_ = height;
  }

  virtual void messageBox(const std::string& title, const std::string& message);

  virtual void show(bool doit){
    shown_ = doit;
  }

  virtual void setTitle(const std::string& title) {}
protected:
  CGE::Renderer* renderer_;
  bool fullscreen_;
  int width_;
  int height_;
  int bpp_;
  bool shown_;
};

}

#endif

