#include "Inventory.h"
#include "ItemObject.h"
#include "Engine.h"
#include <system/allocation.h>

using namespace adv;

Inventory::Inventory() : mCurrentInv(1), mRealized(false){

}

Inventory::~Inventory(){
  clear();
}

void Inventory::clear(){
  for (std::map<int, SingleInv>::iterator iter = mInventory.begin(); iter != mInventory.end(); ++iter){
    while (!iter->second.empty()){
      delete iter->second.front();
      iter->second.pop_front();
    }
  }
}

void Inventory::addItem(ItemObject* item, int invnumber){
  mInventory[invnumber].push_front(item);
}

void Inventory::removeItem(const std::string& item, int invnumber, ExecutionContext* del_script){
  for (SingleInv::iterator iter = mInventory[invnumber].begin(); iter != mInventory[invnumber].end(); ++iter){
    if (_stricmp((*iter)->getName().c_str(), item.c_str()) == 0){
      if (del_script == (*iter)->getScript()){
        //do not delete as it is currently running
        (*iter)->setScript(NULL);
        del_script->setOwner(NULL);
        del_script->setExecuteOnce();
      }
      delete *iter;
      mInventory[invnumber].erase(iter);
      break;
    }
  }
}

ItemObject* Inventory::getItem(const std::string& name){
  for (std::map<int, SingleInv>::iterator iter = mInventory.begin(); iter != mInventory.end(); ++iter){
    for (SingleInv::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2){
      if (_stricmp((*iter2)->getName().c_str(), name.c_str()) == 0){
        return *iter2;
      }
    }
  }
  return NULL;
}

ItemObject* Inventory::getItem(const std::string& name, int invnumber){
  for (SingleInv::iterator iter2 = mInventory[invnumber].begin(); iter2 != mInventory[invnumber].end(); ++iter2){
    if (_stricmp((*iter2)->getName().c_str(), name.c_str()) == 0){
      return *iter2;
    }
  }
  return NULL;
}

void Inventory::save(SaveStateProvider::SaveInventory& inv) const{
  for (std::map<int, SingleInv>::const_iterator iter = mInventory.begin(); iter != mInventory.end(); ++iter){
    for (SingleInv::const_reverse_iterator inviter = iter->second.rbegin(); inviter != iter->second.rend(); ++inviter){
      SaveStateProvider::SaveItem si;
      si.name = (*inviter)->getName();
      si.count = (*inviter)->getCount();
      inv.items[iter->first].push_back(si);
    }
  }
  inv.current = mCurrentInv;
}

void Inventory::load(const SaveStateProvider::SaveInventory& inventory){
  clear();
  mRealized = false;
  for (std::map<int,std::vector<SaveStateProvider::SaveItem> >::const_iterator inviter = inventory.items.begin();
    inviter != inventory.items.end(); ++inviter){
      for (unsigned i = 0; i < inviter->second.size(); ++i){
        ItemObject* item = Engine::instance()->createItem(inviter->second[i].name, inviter->second[i].count);
        addItem(item, inviter->first);
      }
  }
  setCurrent(inventory.current);
}

void Inventory::realize(){
  if (mRealized)
    return;
  for (std::map<int, SingleInv>::iterator iter = mInventory.begin(); iter != mInventory.end(); ++iter){
    for (SingleInv::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2){
      (*iter2)->realize();
    }
  }
  mRealized = true;
}

////////////////////////////////////////////

InventoryDisplay::InventoryDisplay(const Vec2i& pos, const Vec2i& size, const Vec2f& scale, int spacing, int depth)
: mPos(pos+Vec2i(30,5)), mSize(size), mScale(scale), mSpacing(spacing), mItemOffset(0), mDepth(depth){

}

InventoryDisplay::~InventoryDisplay(){

}

void InventoryDisplay::render(Inventory* inv){
  Inventory::SingleInv tmp = inv->mInventory[inv->mCurrentInv];
  Vec2i pos = mPos;
  int precount = 0;
  int count = 0;
  int invitemwidth = (int)(50*mScale.x)+mSpacing;
  int invitemheight = (int)(50*mScale.y)+mSpacing;
  for (std::list<ItemObject*>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter){
    if (precount < mItemOffset){
      ++precount;
      continue;
    }
    (*iter)->setPosition(pos);
    int olddepth = (*iter)->getAnimation()->getDepth();
    (*iter)->render();
    if (mDepth > olddepth)
      (*iter)->setDepth(mDepth);
    ++count;
    //print the item count
    if ((*iter)->getCount() > 1){
      char tmp[32];
      sprintf(tmp, "%i", (*iter)->getCount());
      std::vector<Vec2i> breakinfo;
      Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(tmp, 0, breakinfo);
      Engine::instance()->getFontRenderer()->render(pos.x+invitemwidth-breakinfo[0].y-(int)(9*mScale.x), pos.y-(int)(4*mScale.y), tmp, mDepth, 0, breakinfo, Engine::instance()->getSettings()->infotextcolor, 0, true, ALGN_LEFT);
    }
    int xpos = count % mSize.x;
    pos.x = mPos.x+xpos*invitemwidth;
    int ypos = count / mSize.x;
    pos.y = mPos.y+ypos*invitemheight;
    if (count >= mSize.x*mSize.y){
      break;
    }
  }
}

Object2D* InventoryDisplay::getObjectAt(const Vec2i& pos, Inventory* inv){
  Inventory::SingleInv tmp = inv->mInventory[inv->mCurrentInv];
  int precount = 0;
  int count = 0;
  for (std::list<ItemObject*>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter){
    if (precount < mItemOffset){
      ++precount;
      continue;
    }
    if ((*iter)->isHit(pos))
      return (*iter);
    ++count;
    if (count >= mSize.x*mSize.y)
      break;
  }
  return NULL;
}

void InventoryDisplay::setPosition(const Vec2i& pos){
  mPos = pos+Vec2i(32,8);
}

Vec2i InventoryDisplay::getPosition(){
  return mPos-Vec2i(32,8);
}

void InventoryDisplay::addScrollOffset(int offset, int maxItems){
  mItemOffset += offset;
  if (mItemOffset < 0)
    mItemOffset = 0;
  if (mItemOffset >= maxItems)
    mItemOffset -= offset;
}
