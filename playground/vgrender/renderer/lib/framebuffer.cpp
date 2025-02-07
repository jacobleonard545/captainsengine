#include "framebuffer.h"
#include "state.h"

VRSurface::VRSurface(int width, int height) : mBuffer(4, width, height){

}

void VRSurface::clear(unsigned char r, unsigned char g, unsigned char b, unsigned char a){
  unsigned char* data = mBuffer.getData();
  for (unsigned j = 0; j < mBuffer.getHeight(); ++j){
    for (unsigned i = 0; i < mBuffer.getWidth(); ++i){
      data[0] = r;
      data[1] = g;
      data[2] = b;
      data[3] = a;
      data += 4;
    }
  }
}

void VRSurface::clear(){
  unsigned char* color = VR_CONTEXT->getClearColor();
  clear(color[0], color[1], color[2], color[3]);
}