#include "SaveStateProvider.h"

#include <fstream>
#include <sstream>

#include "Engine.h"
#include "Sound.h"
#include "Particles.h"
#include "PostProcessing.h"
#include <system/allocation.h>

using namespace adv;

namespace adv{

TR_CHANNEL(ADV_SaveState);

std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveRoom& room){
  strm << room.base;
  strm << room.scrolloffset.x << " " << room.scrolloffset.y << std::endl;
  strm << room.overlaylighting << std::endl;
  strm << room.objects.size() << std::endl;
  for (std::map<String,SaveStateProvider::SaveObject*>::const_iterator iter = room.objects.begin(); iter != room.objects.end(); ++iter){
    strm << iter->first << std::endl << *iter->second;
  }
  strm << room.characters.size() << std::endl;
  for (std::map<String,SaveStateProvider::CharSaveObject*>::const_iterator iter = room.characters.begin(); iter != room.characters.end(); ++iter){
    strm << iter->first << std::endl << *iter->second;
  }
  strm << room.walkmap.size() << std::endl;
  for (std::map<Vec2i, bool>::const_iterator iter = room.walkmap.begin(); iter != room.walkmap.end(); ++iter){
    strm << iter->first.x << " " << iter->first.y << " " << iter->second << std::endl;
  }
  return strm;
}

std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveRoom& room){
  strm >> room.base;
  strm >> room.scrolloffset.x >> room.scrolloffset.y;
  strm >> room.overlaylighting;
  int number;
  strm >> number;
  String name;
  for (int i = 0; i < number; ++i){
    strm >> name;
    SaveStateProvider::SaveObject* obj = new SaveStateProvider::SaveObject;
    strm >> *obj;
    room.objects[name] = obj;
  }
  strm >> number;
  for (int i = 0; i < number; ++i){
    strm >> name;
    SaveStateProvider::CharSaveObject* chr = new SaveStateProvider::CharSaveObject;
    strm >> *chr;
    room.characters[name] = chr;
  }
  strm >> number;
  for (int i = 0; i < number; ++i){
    Vec2i pos;
    strm >> pos.x >> pos.y;
    bool walkable;
    strm >> walkable;
    room.walkmap[pos] = walkable;
  }
  return strm;
}

std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveObject& object){
  strm << object.position.x << " " << object.position.y << " " << object.state << " " << object.name << std::endl;
  strm << object.lighting << std::endl;
  return strm;
}

std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveObject& object){
  strm >> object.position.x >> object.position.y >> object.state >> object.name;
  strm >> object.lighting;
  return strm;
}

std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::CharSaveObject& chr){
  strm << chr.base;
  strm << chr.mirrored << std::endl;
  strm << chr.inventory;
  strm << chr.fontid << " " << chr.scale << " " << chr.nozooming << std::endl;
  if (chr.walksound.empty())
    strm << String("none");
  else
    strm << chr.walksound;
  strm << std::endl;
  if (chr.linkedObject.empty())
    strm << String("none");
  else
    strm << chr.linkedObject;
  strm << std::endl;
  return strm;
}

std::istream& operator>>(std::istream& strm, SaveStateProvider::CharSaveObject& chr){
  strm >> chr.base >> chr.mirrored;
  strm >> chr.inventory;
  strm >> chr.fontid >> chr.scale >> chr.nozooming;
  strm >> chr.walksound;
  if (chr.walksound == "none")
    chr.walksound = "";
  strm >> chr.linkedObject;
  if (chr.linkedObject == "none")
    chr.linkedObject = "";
  return strm;
}

std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveInventory& inv){
  strm << inv.items.size() << std::endl;
  for (std::map<int, std::vector<SaveStateProvider::SaveItem> >::const_iterator iter = inv.items.begin(); iter != inv.items.end(); ++iter){
    strm << iter->first << " ";
    strm << iter->second.size();
    for (unsigned i = 0; i < iter->second.size(); ++i){
      strm << " " << iter->second[i].name << " " << iter->second[i].count;
    }
    strm << std::endl;
  }
  strm << inv.current << std::endl;
  return strm;
}

std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveInventory& inv){
  int numInvs, invnum, number;
  strm >> numInvs;
  for (int i = 0; i < numInvs; ++i){
    strm >> invnum >> number;
    inv.items[invnum].resize(number);
    for (int j = 0; j < number; ++j){
      SaveStateProvider::SaveItem si;
      strm >> si.name >> si.count;
      inv.items[invnum][j] = si;
    }
  }
  strm >> inv.current;
  return strm;
}

std::ostream& operator<<(std::ostream& strm, const Color& color){
  strm << (int)color.r << " " << (int)color.g << " " << (int)color.b << " " << (int)color.a;
  return strm;
}

std::istream& operator>>(std::istream& strm, Color& color){
  int tmp;
  strm >> tmp;
  color.r = (char)tmp;
  strm >> tmp;
  color.g = (char)tmp;
  strm >> tmp;
  color.b = (char)tmp;
  strm >> tmp;
  color.a = (char)tmp;
  return strm;
}

}

SaveStateProvider::SaveStateProvider(AdvDocument* data) : mData(data), mNoWrites(0), mMuty(true) {

}

SaveStateProvider::~SaveStateProvider(){
  clear();
}

SaveStateProvider::SaveRoom* SaveStateProvider::getRoom(const std::string name){
  mMuty.lock();
  String idxname = toLower(name);
  std::map<String,SaveRoom*>::iterator iter = mRooms.find(idxname);
  if (mRooms.empty() || iter == mRooms.end()){
    //let's see if we find the room in the original
    Room* orig = mData->getRoom(name);
    if (!orig){
      mMuty.unlock();
      return NULL;
    }
    SaveRoom* save = new SaveRoom();
    save->base.position = Vec2i();
    save->base.lighting = orig->lighting;
    save->base.name = orig->name;
    save->scrolloffset = orig->scrolloffset*-1;
    save->doublewalkmap = orig->doublewalkmap;
    for (unsigned i = 0; i < orig->objects.size(); ++i){
      SaveObject* object = new SaveObject();
      object->lighting = Color();
      object->state = orig->objects[i].state;
      object->position = orig->objects[i].position;
      object->name = orig->objects[i].name;
      save->objects[orig->objects[i].name] = object;
    }
    for (unsigned i = 0; i < mData->getRoomCharacters().size(); ++i){
      if (idxname == toLower(mData->getRoomCharacters()[i].room)){
        bool mirror;
        int state = CharacterObject::calculateState(mData->getRoomCharacters()[i].dir, mirror);
        state = CharacterObject::calculateState(state, false, false, false);
        CharSaveObject* chr = new CharSaveObject();
        chr->base.lighting = Color();
        chr->base.state = state;
        chr->base.position = mData->getRoomCharacters()[i].position;
        chr->base.name = mData->getRoomCharacters()[i].name;
        chr->mirrored = mirror;
        Character* chbase = mData->getCharacter(mData->getRoomCharacters()[i].character);
        int fontid = 0;
        if (chbase != NULL){
          fontid = chbase->fontid;
          chr->scale = chbase->zoom/100.0f;
          chr->nozooming = chbase->notzoom;
          chr->walksound = chbase->walksound;
        }
        else{
          chr->scale = 1.0f;
          chr->nozooming = false;
          chr->walksound = "";
        }
        if (fontid == 0)
          fontid = 1;
        chr->fontid = fontid;
        save->characters[mData->getRoomCharacters()[i].name] = chr;
      }
    }
    mRooms[idxname] = save;
    mMuty.unlock();
    return save;
  }
  mMuty.unlock();
  return iter->second;
}

SaveStateProvider::SaveObject* SaveStateProvider::getObject(SaveStateProvider::SaveRoom* room, const std::string& name){
  mMuty.lock();
  std::map<String,SaveObject*>::iterator iter = room->objects.find(name);
  if (room->objects.empty() || iter == room->objects.end()){
    mMuty.unlock();
    return NULL;
  }
  mMuty.unlock();
  return iter->second;
}

SaveStateProvider::SaveObject* SaveStateProvider::getOrAddObject(SaveStateProvider::SaveRoom* room, const std::string& name){
  mMuty.lock();
  std::map<String,SaveObject*>::iterator iter = room->objects.find(name);
  if (room->objects.empty() || iter == room->objects.end()){
    SaveObject* so = new SaveObject;
    room->objects.insert(std::make_pair(name,so));
    mMuty.unlock();
    return so;
  }
  mMuty.unlock();
  return iter->second;
}

void SaveStateProvider::removeObject(SaveStateProvider::SaveRoom* room, const std::string& name){
  mMuty.lock();
  delete room->objects[name];
  room->objects.erase(name);
  mMuty.unlock();
}

SaveStateProvider::CharSaveObject* SaveStateProvider::getCharacter(SaveStateProvider::SaveRoom* room, const std::string& name){
  mMuty.lock();
  std::map<String,CharSaveObject*>::iterator iter = room->characters.find(name);
  if (room->characters.empty() || iter == room->characters.end()){
    mMuty.unlock();
    return NULL;
  }
  mMuty.unlock();
  return iter->second;
}

SaveStateProvider::CharSaveObject* SaveStateProvider::getOrAddCharacter(SaveStateProvider::SaveRoom* room, const std::string& name){
  mMuty.lock();
  std::map<String,CharSaveObject*>::iterator iter = room->characters.find(name);
  if (room->characters.empty() || iter == room->characters.end()){
    CharSaveObject* cso = new CharSaveObject;
    room->characters.insert(std::make_pair(name,cso));
    mMuty.unlock();
    return cso;
  }
  mMuty.unlock();
  return iter->second;
}

void SaveStateProvider::removeCharacter(SaveStateProvider::SaveRoom* room, const std::string& name){
  mMuty.lock();
  delete room->characters[name];
  room->characters.erase(name);
  mMuty.unlock();
}

void SaveStateProvider::clear(){
  for (std::map<String,SaveRoom*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    for (std::map<String,SaveObject*>::iterator objiter = iter->second->objects.begin(); objiter != iter->second->objects.end(); ++objiter){
      delete objiter->second;
    }
    for (std::map<String,CharSaveObject*>::iterator chriter = iter->second->characters.begin(); chriter != iter->second->characters.end(); ++chriter){
      delete chriter->second;
    }
    delete iter->second;
  }
  mRooms.clear();
}

void SaveStateProvider::save(const std::string& name){
  if (Engine::instance()->mMenuShown){
    Engine::instance()->unloadRoom(NULL, false, true, NULL);
    Engine::instance()->mMenuShown = false;
  }
  std::string focussedcharname;
  if (Engine::instance()->mFocussedChar){
    SaveRoom* sr = getRoom(Engine::instance()->mRooms.back()->getName());
    focussedcharname = Engine::instance()->mFocussedChar->getName();
    Engine::instance()->mFocussedChar->save(sr);
  }
  for (std::list<RoomObject*>::iterator iter = Engine::instance()->mRooms.begin(); iter != Engine::instance()->mRooms.end(); ++iter){
    (*iter)->save(NULL);
  }
  std::ofstream out(name.c_str(), std::ios::binary);
  int version = 2;
  out << version << std::endl;
  out << Engine::instance()->getInterpreter()->getLanguage() << std::endl;
  //save room data
  out << mRooms.size() << std::endl;
  for (std::map<String,SaveRoom*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    String rname = iter->first;
    out << rname;
    out << std::endl << *iter->second;
  }
  //save loaded rooms
  int roomsToSave = (int)Engine::instance()->mRooms.size();
  if (Engine::instance()->mSubRoomLoaded)
    --roomsToSave;//do not save active subroom
  out << roomsToSave;
  for (std::list<RoomObject*>::reverse_iterator iter = Engine::instance()->mRooms.rbegin(); iter != Engine::instance()->mRooms.rend(); ++iter){
    out << " ";
    String rname = (*iter)->getName();
    out << rname;
    --roomsToSave;
    if (roomsToSave <= 0)
      break;
  }
  out << std::endl;
  //focussed char
  CharacterObject* chr = Engine::instance()->getCharacter("self");
  if (chr != NULL)
    out << chr->getName() << std::endl;
  else
    out << "none" << std::endl;
  if (!Engine::instance()->mLastFocussedChar.empty())
    out << Engine::instance()->mLastFocussedChar << std::endl;
  else
    out << "none" << std::endl;
  if (Engine::instance()->mFocussedChar){
    SaveStateProvider::SaveRoom* sr = getRoom(Engine::instance()->mFocussedChar->getRoom());
    removeCharacter(sr, focussedcharname);
  }
  out << Engine::instance()->mMainRoomLoaded << " " << Engine::instance()->mSubRoomLoaded << std::endl;
  out << Engine::instance()->mShowTaskbar << " " << Engine::instance()->mScreenChange << std::endl;
  out << Engine::instance()->mTextEnabled << " " << Engine::instance()->mFontID << std::endl;
  out << Engine::instance()->mMouseShown << " " << Engine::instance()->mMouseEnabled << " " << Engine::instance()->mTimeFactor << " " << Engine::instance()->mTimeFactorFaded << 
    " " << Engine::instance()->mMenuEnabled << std::endl;
  out << Engine::instance()->mFxShapesEnabled << " " << Engine::instance()->mScrollSpeed << " " << Engine::instance()->mCamFollowChar <<std::endl;
  out << Engine::instance()->getCursor()->getRightClickStyle() << std::endl;
  //scripts
  Engine::instance()->getInterpreter()->save(out);
  //sounds
  SoundEngine::instance()->save(out);
  //particles
  Engine::instance()->getParticleEngine()->save(out);
  //textscenes
  Engine::instance()->getFontRenderer()->save(out);
  //postproc
  Engine::instance()->getPostProcessor()->save(out);
  out.close();
  mData->fileChanged(name);
}

void SaveStateProvider::load(const std::string& name, bool fromScript){
  std::ifstream in(name.c_str(), std::ios::binary);
  if (!in)
    return;
  //clear the current save state;
  Engine::instance()->setFocus("none", NULL);
  clear();
  Engine::instance()->getInterpreter()->clearState();
  Engine::instance()->getInterpreter()->executeCutscene(NULL, false);
  mNoWrites = Engine::instance()->unloadRooms(false);
  if (Engine::instance()->mMenuShown){
    Engine::instance()->unloadRoom(NULL, false, false, NULL);
    Engine::instance()->mMenuShown = false;
  }
  //load room data
  std::string language;
  int version;
  in >> version;
  in >> language;
  Engine::instance()->getInterpreter()->setLanguage(language);
  int numRooms;
  in >> numRooms;
  String roomname;
  for (int i = 0; i < numRooms; ++i){
    in >> roomname;
    SaveRoom* room = new SaveRoom;
    in >> *room;
    Room* rm = mData->getRoom(roomname);
    room->doublewalkmap = rm->doublewalkmap;
    mRooms[roomname] = room;
  }
  //load rooms to load
  in >> numRooms;
  for (int i = 0; i < numRooms; ++i){
    in >> roomname;
    if (i == 0)
      Engine::instance()->loadMainRoom(roomname, NULL, Engine::instance()->getScreenChange(), false);
    else
      Engine::instance()->loadSubRoom(roomname, NULL, 0, false);
  }
  if (fromScript)
    Engine::instance()->getInterpreter()->getExecMutex().unlock();
  Engine::instance()->mLoader.waitUntilFinished();//TODO quick and dirty, persistence access to avoid race conditions
  if (fromScript)
    Engine::instance()->getInterpreter()->getExecMutex().lock();
  //focussed char
  std::string focussedchar;
  in >> focussedchar;
  in >> Engine::instance()->mLastFocussedChar;
  if (Engine::instance()->mLastFocussedChar == "none"){
    Engine::instance()->mLastFocussedChar = "none";
  }
  Engine::instance()->setFocus(focussedchar, NULL);
  //data
  int tmp;
  in >> Engine::instance()->mMainRoomLoaded >> Engine::instance()->mSubRoomLoaded;
  in >> Engine::instance()->mShowTaskbar >> tmp;
  Engine::instance()->mScreenChange = (ScreenChange)tmp;
  in >> Engine::instance()->mTextEnabled >> Engine::instance()->mFontID;
  Engine::instance()->getFontRenderer()->loadFont(Engine::instance()->mFontID);
  in >> Engine::instance()->mMouseShown >> Engine::instance()->mMouseEnabled;
  in >> Engine::instance()->mTimeFactor >> Engine::instance()->mTimeFactorFaded;
  in >> Engine::instance()->mMenuEnabled;
  in >> Engine::instance()->mFxShapesEnabled >> Engine::instance()->mScrollSpeed >> Engine::instance()->mCamFollowChar;
  if (version >= 2){
    in >> tmp;
    Engine::instance()->getCursor()->setRightClickStyle(tmp);
  }
  Engine::instance()->getInterpreter()->load(in, version);
  SoundEngine::instance()->load(in, version);
  Engine::instance()->getParticleEngine()->load(in);
  Engine::instance()->getFontRenderer()->load(in, version);
  Engine::instance()->getPostProcessor()->load(in);
  in.close();
}

std::string SaveStateProvider::saveSlotToPath(int slot){
  std::ostringstream path;
  path << Engine::instance()->getSettings()->savedir;
  path << "/save" << slot << ".sav";
  return path.str();
}

SaveStateProvider::CharSaveObject* SaveStateProvider::findCharacter(const std::string& name, std::string& room, std::string& realName){
  mMuty.lock();
  //check if already present
  for (std::map<String,SaveRoom*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    for (std::map<String,CharSaveObject*>::iterator chriter = iter->second->characters.begin(); chriter != iter->second->characters.end(); ++chriter){
      if (_stricmp(chriter->first.c_str(), name.c_str()) == 0){
        room = iter->second->base.name;
        realName = chriter->first.c_str();
        mMuty.unlock();
        return chriter->second;
      }
    }
  }
  for (unsigned i = 0; i < mData->getRoomCharacters().size(); ++i){
    if (_stricmp(mData->getRoomCharacters()[i].name.c_str(), name.c_str()) == 0){
      //load the room with the character into saving
      room = mData->getRoomCharacters()[i].room;
      realName = mData->getRoomCharacters()[i].name;
      SaveRoom* saveroom = getRoom(room);
      std::map<String,CharSaveObject*>::iterator iter = saveroom->characters.find(realName);
      if (iter != saveroom->characters.end()){
        mMuty.unlock();
        return iter->second;
      }
      for (std::map<String,CharSaveObject*>::iterator iter = saveroom->characters.begin(); iter != saveroom->characters.end(); ++iter){
        if (_stricmp(iter->first.c_str(), name.c_str()) == 0){
          mMuty.unlock();
          return iter->second;
        }
      }
    }
  }
  mMuty.unlock();
  return NULL;
}

SaveStateProvider::SaveObject* SaveStateProvider::findObject(const std::string& name, std::string& room){
  TR_USE(ADV_SaveState);
  mMuty.lock();
  //check if already present
  for (std::map<String,SaveRoom*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    for (std::map<String,SaveObject*>::iterator objiter = iter->second->objects.begin(); objiter != iter->second->objects.end(); ++objiter){
      if (_stricmp(objiter->first.c_str(), name.c_str()) == 0){
        room = iter->second->base.name;
        mMuty.unlock();
        return objiter->second;
      }
    }
  }
  //load the room with the object into saving
  Room* rm = NULL;
  Roomobject* obj = mData->findRoomObject(name, rm);
  if (obj == NULL){
    mMuty.unlock();
    TR_ERROR("Object %s not found.", name.c_str());
    return NULL;
  }
  room = rm->name;
  SaveRoom* saveroom = getRoom(room);
  std::map<String,SaveObject*>::iterator iter = saveroom->objects.find(obj->name);
  if (iter != saveroom->objects.end()){
    mMuty.unlock();
    return iter->second;
  }
  TR_BREAK("Object %s not found in %s", name.c_str(), room.c_str());
  mMuty.unlock();
  return NULL;
}

float SaveStateProvider::SaveRoom::getWalkGridSize(){
  return Engine::instance()->getWalkGridSize(doublewalkmap);
}
