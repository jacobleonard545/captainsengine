#if defined WIN32 && !defined _NODIRECTX

#include "../../system/engine.h"
#include <d3d11.h>
#include "DXrenderer.h"
#include "DXtexture.h"
#include "../../image/loader.h"
#include "../../image/image.h"

using namespace CGE; 

DXTexture::DXTexture() : Texture(AUTO), mTexture(NULL), mTex(NULL), mState(NULL){
  mDevice = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();
}

DXTexture::~DXTexture(){
  SAFE_RELEASE(mTexture);
  SAFE_RELEASE(mTex);
  SAFE_RELEASE(mState);
}

bool DXTexture::createFromImage(Image const* img, Format fmt){
  if (fmt == AUTO)
    fmt = (Format)img->getNumChannels();
  mFormat = fmt;
  mWidth = img->getWidth();
  mHeight = img->getHeight();

  CGE::Image* converted = NULL;
  const void* imgdata;
  UINT imgsize;
  UINT imgpitch;

  if (img->getNumChannels() == 3){
    converted = img->convertImage(4);
    imgdata = converted->getData();
    imgsize = converted->getImageSize();
    imgpitch = converted->getRowSpan();
  }
  else{
    imgdata = img->getData();
    imgsize = img->getImageSize();
    imgpitch = img->getRowSpan();
  }

  SAFE_RELEASE(mTex);
  SAFE_RELEASE(mState);

  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = img->getWidth();
  desc.Height = img->getHeight();
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = dxformat(fmt);
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  //desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = imgdata;
  data.SysMemPitch = imgpitch;
  data.SysMemSlicePitch = imgsize;

  HRESULT res = mDevice->CreateTexture2D(&desc, &data, &mTexture);
  if (!SUCCEEDED(res))
    return false;
  mDevice->CreateShaderResourceView(mTexture, NULL, &mTex);

  D3D11_SAMPLER_DESC samp;
  ZeroMemory(&samp, sizeof(samp));
  samp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samp.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samp.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samp.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samp.MaxLOD = D3D11_FLOAT32_MAX;
  mDevice->CreateSamplerState(&samp, &mState);

  delete converted;

  return true;
}

bool DXTexture::createEmpty(unsigned width, unsigned height, Format fmt){
  if (fmt == AUTO)
    return false;
  mFormat = fmt;
  mWidth = width;
  mHeight = height;

  SAFE_RELEASE(mTex);
  SAFE_RELEASE(mState);

  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = fmt == DEPTH ? DXGI_FORMAT_R24G8_TYPELESS : dxformat(fmt);
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  if (fmt == Texture::DEPTH)
    desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
  else
    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  HRESULT res = mDevice->CreateTexture2D(&desc, NULL, &mTexture);
  if (!SUCCEEDED(res))
    return false;

  D3D11_SHADER_RESOURCE_VIEW_DESC rvdesc;
  ZeroMemory(&rvdesc, sizeof(rvdesc));
  rvdesc.Format = fmt == DEPTH ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : dxformat(fmt);
  rvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  rvdesc.Texture2D.MipLevels = -1;
  rvdesc.Texture2D.MostDetailedMip = 0;
  mDevice->CreateShaderResourceView(mTexture, &rvdesc, &mTex);

  D3D11_SAMPLER_DESC samp;
  ZeroMemory(&samp, sizeof(samp));
  samp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samp.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samp.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samp.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samp.MaxLOD = D3D11_FLOAT32_MAX;
  mDevice->CreateSamplerState(&samp, &mState);

  return true;
}

void DXTexture::activate(unsigned stage)const{
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->PSSetShaderResources(stage, 1, &mTex);
  ctx->PSSetSamplers(stage, 1, &mState);
}

void DXTexture::deactivate(unsigned stage)const{
  ID3D11ShaderResourceView* view = NULL;
  //ID3D11SamplerState* state = NULL;
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->PSSetShaderResources(stage, 1, &view);
  //ctx->PSSetSamplers(stage, 1, &state);
}

DXGI_FORMAT DXTexture::dxformat(Format fmt){
  switch (fmt){
  case GRAY:
    return DXGI_FORMAT_R8_UNORM;
  case RGB:
    return DXGI_FORMAT_R8G8B8A8_UNORM;
  case RGBA:
    return DXGI_FORMAT_R8G8B8A8_UNORM;
  case DEPTH:
    return DXGI_FORMAT_D24_UNORM_S8_UINT;
  case ALPHA:
    return DXGI_FORMAT_A8_UNORM;
  case FLOAT:
    return DXGI_FORMAT_R32G32B32A32_FLOAT;
  }
  return DXGI_FORMAT_UNKNOWN;
}

Texture* DXTexture::copy(){
  DXTexture* ret = new DXTexture();
  ret->createEmpty(mWidth, mHeight, mFormat);
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->CopyResource(ret->mTexture, mTexture);
  return ret;
}

void DXTexture::update(int x, int y, unsigned width, unsigned height, void* data){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  int bpp = (int)mFormat;
  D3D11_BOX box;
  box.top = y;
  box.left = x;
  box.bottom = y + height;
  box.right = x + width;
  box.front = 0;
  box.back = 1;
  if (bpp == 3){
    Image img(3, width, height, (unsigned char*)data);
    img.convertFormat(4);
    ctx->UpdateSubresource(mTexture, 0, &box, img.getData(), width*4, 0);
  }
  else
    ctx->UpdateSubresource(mTexture, 0, &box, data, width*bpp, 0);
}

#endif
