#ifndef FONT_H
#define FONT_H

#include <queue>
#include <list>
#include "EngineObjects.h"

namespace adv{

class Textout;
class FontBlitObject;

class FontRenderer{
public:
  class String{
  public:
    String(const Vec2i& pos, unsigned displayTime, bool keepOnScreen, unsigned fading);
    //String(const String& str);
    ~String();
    void append(FontBlitObject* obj) {mString.push_back(obj);}
    void clear();
    void render(unsigned interval);
    int getTime() {return mDisplayTime;}
    void setSuspensionScript(ExecutionContext* ctx);
    void setSpeaker(CharacterObject* chr) {mSpeaker = chr;}
    CharacterObject* getSpeaker() {return mSpeaker;}
    void setExtent(const Vec2i& extent);
    void setBoundRoom(RoomObject* room) {mBoundRoom = room;}
    RoomObject* getRoom() {return mBoundRoom;}
    void remove();
    void endDisplaying(bool immediate);
    void setOpacity(unsigned char opacity);
  protected:
    Vec2i mPos;
    Vec2i mCenterOffset;
    int mDisplayTime;
    std::vector<FontBlitObject*> mString;
    ExecutionContext* mSuspensionScript;
    CharacterObject* mSpeaker;
    bool mUnregisterSpeaker;
    bool mKeepOnScreen;
    RoomObject* mBoundRoom;
    int mFadingTime;
    int mTimeShown;
  };

  class Font{
  public:
    Font(const FontData& data, int fading, bool unified);
    ~Font();
    String* render(int x, int y, const std::string& text, int depth, const Color& color, unsigned displayTime, const std::vector<Vec2i>& breakinfo, bool keepOnScreen, Alignment align);
    Vec2i getTextExtent(const std::string& text, std::vector<Vec2i>& breakinfo, unsigned maxStringWidth);
    void blit(unsigned interval,  RoomObject* mainroom, bool renderBoundText);
    void removeText(CharacterObject* chr, bool immediately);
    void removeText(String* str);
    int getFading() {return mFading;}
    int getLineHeight() { return mFontsize.y; }
  protected:
    void buildBlitObject();
    Vec2i mFontsize;
    Vec2i mNumChars;
    Vec2i mTexSize;
    Vec2f mScale;
    std::vector<CGE::Texture*> mImages;
    std::vector<FontData::Glyph> mGlyphMap;
    std::list<String*> mRenderQueue;
    int mFading;
  };
public:
  FontRenderer(AdvDocument* data);
  ~FontRenderer();
  bool loadFont(unsigned id);
  void unloadFont(unsigned id);
  String* render(int x, int y, const std::string& text, int depth, int fontid, const std::vector<Vec2i>& breakinfo, const Color& col, unsigned displayTime, bool keepOnScreen, Alignment align);
  Vec2i getTextExtent(const std::string& text, int fontid, std::vector<Vec2i>& breakinfo, int maxStringWidth=300);
  void prepareTextouts(unsigned interval);
  void prepareBlit(unsigned interval, RoomObject* mainroom, bool renderBoundText);
  void removeText(CharacterObject* chr, bool immediately);
  void removeText(String* str);
  Textout* getTextout(const std::string& id);
  void clearTextouts();
  void enableTextouts(bool enable);
  void enableBoundTextouts(RoomObject* room, bool enable);
  void save(std::ostream& out);
  void load(std::istream& in, int version);
  Font* getFont(unsigned id) {return mFonts[id];}
protected:
  AdvDocument* mData;
  std::vector<Font*> mFonts;
  std::map<std::string, Textout*> mTextouts;
};

}

#endif
