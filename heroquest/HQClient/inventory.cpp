#include <iostream>
#include "input/mouse.h"

#include "textureManager.h"
#include "script.h"
#include "inventory.h"
#ifdef _CLIENT_
#include "player.h"
#include "renderer.h"
#include "renderer/forms.h"
#endif

using std::cerr;
using CGE::Vector2D;
using Input::Mouse;

Inventory::Inventory(){
  items_.push_back(Item());
  left_hand_ = 0;
  right_hand_ = 0;
  head_ = 0;
  body_ = 0;
  belt_ = 0;
  breast_ = 0;
#ifdef _CLIENT_
  fnt_ = CGE::Engine::instance()->getFont(0);
  //fnt_.setBase(text.getBase());
  //fnt_.setColor(0,1,1);
  page_ = 1;
  chosenItem_ = NULL;
  visible_ = true;
#endif
}

Inventory::Inventory(const Inventory& i){
  items_ = vector<Item>(i.items_);
  lookup_ = map<string,int>(i.lookup_);
  left_hand_ = i.left_hand_;
  right_hand_ = i.right_hand_;
  head_ = i.head_;
  body_ = i.body_;
  belt_ = i.belt_;
  breast_ = i.breast_;
#ifdef _CLIENT_
  fnt_ = i.fnt_;
  page_ = i.page_;
  chosenItem_ = i.chosenItem_;
  visible_ = i.visible_;
#endif
}

void Inventory::addItem(Item it){
  int idx = lookup_[it.getName()];
  //no such item stored
  if(idx == 0){
    items_.push_back(it);
    lookup_[it.getName()] = (int)items_.size()-1;
  }
  else{
    for (int i = 0; i < it.getNumber(); i++)
      items_[idx].increase();
  }
}

bool Inventory::deleteItem(string name){
  int idx = lookup_[name];
  if (idx != 0){
    //last item deleted
    if (!items_[idx].decrease()){
      items_[idx].reset();
      return false;
    }
    return true;
  }
  return false;
}


bool Inventory::deleteItemSavely(const Vector2D& pos, string name){
  int idx = lookup_[name];
  if (idx != 0){
    //is it wearable
    if (items_[idx].getType() == Item::Armory){
      int id = items_[idx].getId();
      if (idx == left_hand_ || idx == right_hand_ || idx == head_ || idx == body_ ||
          idx == belt_ || idx == breast_){
        scr.armoryOff(pos, id);
      }
    }
    //last item deleted
    if (!items_[idx].decrease()){
      items_[idx].reset();
      return false;
    }
    return true;
  }
  return false;
}


// get item from inventory
Item Inventory::getItem(const string name){
  int idx = lookup_[name];
  return items_[idx];
}

bool Inventory::useItem(string name, Vector2D pos, Vector2D target){
  int idx = lookup_[name];
  if (idx == 0)
    return false;
  if(!items_[idx].isValid())
    return false;
  bool success = items_[idx].use(pos, target);
  return success;
}
    
//! writes item data
void Inventory::write(ofstream& out)const{
  short length = (short)items_.size()-1;
  out.write((char*)&length, sizeof(length));
  for (int i = 1; i < length+1; i++){
    items_[i].write(out);
  }
  out.write((char*)&left_hand_, sizeof(left_hand_));
  out.write((char*)&right_hand_, sizeof(right_hand_));
  out.write((char*)&head_, sizeof(head_));
  out.write((char*)&body_, sizeof(body_));
  out.write((char*)&belt_, sizeof(belt_));
  out.write((char*)&breast_, sizeof(breast_));
}

//! reads item data
void Inventory::read(ifstream& in){
  short length;
  in.read((char*)&length, sizeof(length));
  for (int i = 0; i < length; i++){
    Item it;
    it.read(in);
    items_.push_back(it);
    lookup_[it.getName()] = (int)items_.size()-1;
  }
  in.read((char*)&left_hand_, sizeof(left_hand_));
  in.read((char*)&right_hand_, sizeof(right_hand_));
  in.read((char*)&head_, sizeof(head_));
  in.read((char*)&body_, sizeof(body_));
  in.read((char*)&belt_, sizeof(belt_));
  in.read((char*)&breast_, sizeof(breast_));
}

string Inventory::toString(){
  string ret = "";
  for (unsigned i = 0; i < items_.size(); i++){
    ret += items_[i].toString()+"!";
  }
  ret += toStr(left_hand_)+"$";
  ret += toStr(right_hand_)+"$";
  ret += toStr(head_)+"$";
  ret += toStr(body_)+"$";
  ret += toStr(belt_)+"$";
  ret += toStr(breast_)+"$";
  return ret;
}

void Inventory::fromString(string it){
  size_t pos = 0;
  size_t found;
  items_.clear();
  while ((found = it.find('!',pos)) != string::npos){
    string tmp = it.substr(pos,found-pos);
    pos = found+1;
    Item ite;
    ite.fromString(tmp);
    items_.push_back(ite);
    lookup_[ite.getName()] = (int)items_.size()-1;
  }
  //find the armory mount points
  string tmp;
  found = it.find('$', pos);
  left_hand_ = toInt(it.substr(pos,found-pos));
  pos = found+1;
  found = it.find('$', pos);
  right_hand_ = toInt(it.substr(pos,found-pos));
  pos = found+1;
  found = it.find('$', pos);
  head_ = toInt(it.substr(pos,found-pos));
  pos = found+1;
  found = it.find('$', pos);
  body_ = toInt(it.substr(pos,found-pos));
  pos = found+1;
  found = it.find('$', pos);
  belt_ = toInt(it.substr(pos,found-pos));
  pos = found+1;
  found = it.find('$', pos);
  breast_ = toInt(it.substr(pos,found-pos));
  pos = found+1;

}


bool Inventory::wearArmory(string what, string where1, string where2){
  int idx = lookup_[what];
  if (idx == 0)
    return false;
  if(!items_[idx].isValid())
    return false;
  if (where1 == "left hand" || where2 == "left hand"){
    if (left_hand_ != 0)
      return false;
    left_hand_ = idx;
  }
  if (where1 == "right hand" || where2 == "right hand"){
    if (right_hand_ != 0)
      return false; 
    right_hand_ = idx;
  }
  if (where1 == "head" || where2 == "head"){
    if (head_ != 0)
      return false;
    head_ = idx;
  }
  if (where1 == "body" || where2 == "body"){
    if (body_ != 0)
      return false;
    body_ = idx;
  }
  if (where1 == "belt" || where2 == "belt"){
    if (belt_ != 0)
      return false;
    belt_ = idx;
  }
  if (where1 == "breast" || where2 == "breast"){
    if (breast_ != 0)
      return false;
    breast_ = idx;
  }
  return true;
}


// get worn item
Item& Inventory::getArmory(string where){
  if (where == "left hand")
    return items_[left_hand_];
  if (where == "right hand")
    return items_[right_hand_];
  if (where == "head")
    return items_[head_];
  if (where == "body")
    return items_[body_];
  if (where == "belt")
    return items_[belt_];
  if (where == "breast")
    return items_[breast_];
  return items_[0];
}
    
// remove worn armory
void Inventory::removeArmory(string what){
  int idx = lookup_[what];
  if (idx == 0 || !items_[idx].isValid())
    return;
  if (left_hand_ == idx)
    left_hand_ = 0;
  if (right_hand_ == idx)
    right_hand_ = 0;
  if (head_ == idx)
    head_ = 0;
  if (body_ == idx)
    body_ = 0;
  if (belt_ == idx)
    belt_ = 0;
  if (breast_ == idx)
    breast_ = 0;
}


//delete used items and restructure data
void Inventory::compactify(){
  vector<Item>::iterator iter = items_.begin();
  iter++;
  int position = 1;
  int offset = 0;
  while (iter != items_.end()){
    Item& it = *iter;
    string name = it.getName();
    lookup_[name] = position-offset;
    if (!it.isValid()){
      offset++;
      iter = items_.erase(iter);
    }
    else{
      iter++;
    }
    position++;
  }
}


// give back used spells
void Inventory::restoreSpells(int classes){
  vector<Item>::iterator iter = items_.begin();
  iter++;
  string lastSpell = "blubb";
  int currClass = -1;
  while (iter != items_.end()){
    Item& it = *iter;
    if (it.getType() == Item::Spell){
      //update class
      if (lastSpell != it.getAdditional()){
        lastSpell = it.getAdditional();
        currClass++;
      }
      //is it allowed to have and invalid
      if (currClass < classes && !it.isValid()){
        //looks like a used spell, so give one more
        it.increase();
      }
      else if (currClass >= classes){
        //not allowed to possess
        it.decrease();
      }
    }
    iter++;
  }
}

void Inventory::render(){
#ifdef _CLIENT_
  if (!visible_)
    return;
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  //background
  rend->enableBlend(false);
  rend->setColor(1.0, 1.0, 1.0, 1.0);
  TextureManager::instance()->otherTex[1]->activate();
  //glBindTexture(GL_TEXTURE_2D, tex.otherTex[1]);
  CGE::Forms* forms = CGE::Engine::instance()->getForms();
  forms->activateQuad();
  forms->drawQuad(Vec2f(), Vec2f(SCREENWIDTH, SCREENHEIGHT));
  rend->enableBlend(true);

  //items
  Vector2D pos(50, 485);
  short count = 0;
  fnt_->setColor(0,1,1);
  for (unsigned i = 9*(page_-1)+1; i < items_.size() && i < (unsigned)(9*page_+1); i++){
    string name = items_[i].getName();
    if (name.size() > 11)
      name.erase(11);
    int fill = (int)(11 - name.size())/2;
    name.insert(name.begin(), fill, ' ');
    fnt_->print(pos.x, pos.y, name.c_str(), 1);
    fnt_->print(pos.x+48, pos.y-18, toStr(items_[i].getNumber()).c_str(), 1);
    pos.x += 148;
    count++;
    if (count == 3){
      pos.x = 50;
      count = 0;
      pos.y -= 193;
    }
  }
  fnt_->print(177, 50, ("<<  Page "+toStr(page_)+"  >>").c_str(), 0);

  //stats
  Creature* c = plyr.getCreature();
  fnt_->setColor(0,1,1);
  fnt_->print(600, 700, ("Name: "+c->getName()).c_str(), 0);
  fnt_->print(800, 700, ("Race: "+c->getType()).c_str(), 0);
  fnt_->print(600, 680, ("Attack: "+toStr(c->getAttack())).c_str(), 0);
  fnt_->print(800, 680, ("Defense: "+toStr(c->getDefence())).c_str(), 0);
  fnt_->print(600, 660, ("Body: "+toStr(c->getBody())+"/"+toStr(c->getMaxBody())).c_str(), 0);
  fnt_->print(800, 660, ("Mind: "+toStr(c->getMind())+"/"+toStr(c->getMaxMind())).c_str(), 0);
  Hero* h = dynamic_cast<Hero*>(c);
  if (h){
    fnt_->print(600, 620, ("Money: "+toStr(h->getMoney())).c_str(), 0);
  }

  //worn items
  Item ite = getArmory("right hand");
  if (ite.isValid()){
    fnt_->print(600, 580, (ite.getName()+" is held in the right hand").c_str(), 0);
  }
  ite = getArmory("left hand");
  if (ite.isValid()){
    fnt_->print(600, 560, (ite.getName()+" is held in the left hand").c_str(), 0);
  }
  ite = getArmory("head");
  if (ite.isValid()){
    fnt_->print(600, 540, (ite.getName()+" is worn on the head").c_str(), 0);
  }
  ite = getArmory("body");
  if (ite.isValid()){
    fnt_->print(600, 520, (ite.getName()+" is worn on the body").c_str(), 0);
  }
  ite = getArmory("belt");
  if (ite.isValid()){
    fnt_->print(600, 500, (ite.getName()+" is worn on the belt").c_str(), 0);
  }
  ite = getArmory("breast");
  if (ite.isValid()){
    fnt_->print(600, 480, (ite.getName()+" is worn on the breast").c_str(), 0);
  }

  //Vector2D click = gl->getMousePos();
  //fnt_->glPrint(600, 460, (toStr(click.x)+"/"+toStr(click.y)).c_str(), 0);

  if (chosenItem_ != NULL){
    fnt_->print(600, 440, chosenItem_->getName().c_str(), 0);
  }

  //fnt_.render();
#endif
}

void Inventory::update(){
#ifdef _CLIENT_
  if (!visible_)
    return;
  Vector2D click = Input::Mouse::instance()->getClickPos();
  //forward button
  if (click.x >= 300 && click.x <= 325){
    if (click.y >= 700 && click.y <= 720){
      Mouse::instance()->resetMousePos();
      page_++;
    }
  }
  //backward button
  if (click.x >= 180 && click.x <= 200){
    if (click.y >= 700 && click.y <= 720){
      if (page_ > 1){
        Mouse::instance()->resetMousePos();
        page_--;
      }
    }
  }
  //first item column
  if (click.x >= 65 && click.x <= 155){
    if (click.y >= 125 && click.y <= 280){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(page_-1)+1;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 315 && click.y <= 475){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(page_-1)+4;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 510 && click.y <= 670){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(page_-1)+7;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
  }
  //second item column
  if (click.x >= 210 && click.x <= 300){
    if (click.y >= 125 && click.y <= 280){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(page_-1)+2;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 315 && click.y <= 475){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(page_-1)+5;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 510 && click.y <= 670){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(page_-1)+8;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
  }
  //third item column
  if (click.x >= 355 && click.x <= 450){
    if (click.y >= 125 && click.y <= 280){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(page_-1)+3;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 315 && click.y <= 475){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(page_-1)+6;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 510 && click.y <= 670){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(page_-1)+9;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
  }
#endif
}

