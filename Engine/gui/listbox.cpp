//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |                 The Open Source game engine                      |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: listbox.cpp                                                |
//  ==================================================================

#include "listbox.h"

#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/forms.h"

using namespace Gui;
using CGE::Color;
using CGE::Renderer;
using namespace CGE;
using std::max;

//! Constructor
ListBox::ListBox(){
  type_ = ListBoxT;
  pos_ = Vector2D(112, 84);
  span_ = Vector2D(800,600);
  bgColor_ = Color(0.0,0.0,0.0,0.8f);
  selected_ = -1;
  selColor_ = Vector3D(0.0,1.0,0.0);
  handleClicks_ = NULL;
  scroll_ = 0;
}
  
//! Destructor
ListBox::~ListBox(){
  entries_.clear();
}

//! The render function
void ListBox::render(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableTexturing(false);
  //draw background
  rend->setColor(&bgColor_);
  CGE::Engine::instance()->getForms()->activateQuad();
  CGE::Engine::instance()->getForms()->drawQuad(pos_, span_);

  //draw text
  rend->enableTexturing(true);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE);
  int y = pos_.y + span_.y - 20;
  int maxy = span_.y / 20;
  int count = 0;
  vector<std::string>::iterator iter;
  for (iter = entries_.begin(); iter != entries_.end() && count < maxy+scroll_; iter++){
    //scrolled away
    if (count < scroll_){
      ++count;
      continue;
    }
    if (count == selected_){
      CGE::Engine::instance()->getFont(1)->setColor(selColor_.x, selColor_.y, selColor_.z);
    }
    else{
      CGE::Engine::instance()->getFont(1)->setColor(fgColor_.x, fgColor_.y, fgColor_.z);
    }
    CGE::Engine::instance()->getFont(1)->print(pos_.x, y, (*iter).c_str(), 0, 0.0);
    y -= 20;
    ++count;
  }
}

//! Is the dialog clicked
bool ListBox::isClicked(const ::CGE::Vector2D& pos){
  //out of bounds?
  if (pos.x < pos_.x || pos.x > pos_.x+span_.x)
    return false;
  if (pos.y < pos_.y || pos.y > pos_.y+span_.y)
    return false;
  //scroll down
  if (pos.x < pos_.x+span_.x && pos.x > pos_.x+span_.x-20
      && pos.y < pos_.y+span_.y/2){
    //is scrolling neccessary
    int maxy = span_.y / 20;
    if (scroll_+maxy >= (int)entries_.size())
      return false;
    scroll_++;
    return false;
  }
  //scroll up
  if (pos.x < pos_.x+span_.x && pos.x > pos_.x+span_.x-20
      && pos.y > pos_.y+span_.y/2){
    scroll_ = max(0,scroll_-1);
    return false;
  }
  //calculate the selection index
  int diff = pos_.y+span_.y-pos.y;
  diff /= 20;
  //a valid entry?
  if (diff <= (int)entries_.size()){
    selected_ = diff+scroll_;
  }
  return true;
}

//! Process an event
void ListBox::process(){
  if (handleClicks_)
    (*handleClicks_)(this);
}

//! Add an entry
void ListBox::addEntry(std::string entry){
  entries_.push_back(entry);
}

//! Add multiple entries
void ListBox::addEntries(const std::vector<std::string>& entries){
  for (unsigned i = 0; i < entries.size(); i++){
    addEntry(entries[i]);
  }
}

