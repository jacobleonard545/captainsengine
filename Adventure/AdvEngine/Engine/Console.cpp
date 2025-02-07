#include "Console.h"
#include "Engine.h"
#include <io/TraceManager.h>

using namespace adv;

TR_CHANNEL(ADV_Console);

Console::Console() : RoomObject(1,
  Vec2i(0, 0),
  Vec2i(Engine::instance()->getSettings()->resolution.x, Engine::instance()->getSettings()->resolution.y/3), "!console", Vec2i(), false),
  mActive(false), mHistoryPos(-1), mScrollPos(0){
  int font = 0;
  Color bgorig(Engine::instance()->getSettings()->backgroundcolor);
  Color bg(Engine::instance()->getSettings()->backgroundcolor);
  bg += 16;
  Color line(Engine::instance()->getSettings()->bordercolor);
  Color text(Engine::instance()->getSettings()->textcolor);
  setBackground("#console_bg", DEPTH_CONSOLE);

  Vec2i crop(Engine::instance()->getSettings()->resolution.x, 3);
  Object2D* border = new Object2D(1, Vec2i(), Vec2i(Engine::instance()->getSettings()->resolution.x, Engine::instance()->getSettings()->resolution.y / 3), "#consoleBorder");
  border->setDepth(DEPTH_CONSOLE+DEPTH_LIGHTING);
  SimpleFrames sf;
  sf.push_back("#console_bg");
  Animation* anim = new Animation(sf, 20.0f, Vec2i(), DEPTH_CONSOLE + DEPTH_LIGHTING, crop);
  border->addAnimation(anim);
  addObject(border);

  CGE::TraceManager::instance()->setTraceOutputter(this);
  char tmp[512];

  const char* script =
    "on(enter){\n"
    "  bindtext(0-1; !console)\n"
    "}\n"
    "\n"
    "on(exit){\n"
    "  bindtext(0-1; [empty])\n"
    "}\n"
    "\n"
    "on(loop2){\n"
    "  entertext([!consoleInput]; 5; %i; 0; 80; 255; 255; 255; [!consoleInput])\n"
    "}\n";
  sprintf(tmp, script,
    Engine::instance()->getSettings()->resolution.y / 3 - 3 - Engine::instance()->getFontRenderer()->getFont(0)->getLineHeight()
    );
  ExecutionContext* scr = Engine::instance()->getInterpreter()->parseProgram(tmp, PCDK_SCRIPT);
  setScript(scr);
}

Console::~Console(){
  CGE::TraceManager::instance()->removeTraceOutputter(this);
}

bool Console::init(){
  return true;
}

void Console::trace(unsigned channel, int level, const char* function, const char* message){
  if (channel == TR_GET_ID(ADV_Console))
    output(message);
}

CGE::Image* Console::getBackground(){
  int w = Engine::instance()->getSettings()->resolution.x;
  int h = Engine::instance()->getSettings()->resolution.y / 3;
  CGE::Image* img = new CGE::Image(4, w, h);
  img->allocateData();
  Color bg(57, 99, 104, 200);
  Color line(41, 76, 80, 255);
  Color black(0);
  Color* toSet = &bg;
  for (int y = 0; y < h; ++y){
    for (int x = 0; x < w; ++x){
      if (x == 0 || x == w - 1 || y == 0 || y == h - 1 ||
        ((x == 3 || x == w - 1 - 3) && (y >= 3 && y <= h - 1 - 3)) ||
        ((y == 3 || y == h - 1 - 3) && (x >= 3 && x <= w - 1 - 3)))
        toSet = &line;
      else if (x == 1 || x == 2 || x == w - 2 || x == w - 3 ||
        y == 1 || y == 2 || y == h - 2 || y == h - 3)
        toSet = &black;
      else if (y == h - 3*2 - Engine::instance()->getFontRenderer()->getFont(0)->getLineHeight())
        toSet = &line;
      else
        toSet = &bg;
      img->getData()[y*w * 4 + x * 4] = toSet->r;
      img->getData()[y*w * 4 + x * 4 + 1] = toSet->g;
      img->getData()[y*w * 4 + x * 4 + 2] = toSet->b;
      img->getData()[y*w * 4 + x * 4 + 3] = toSet->a;
    }
  }
  return img;
}

void Console::render(){
  RoomObject::render();
  FontRenderer* fren = Engine::instance()->getFontRenderer();
  int lineHeight = Engine::instance()->getFontRenderer()->getFont(0)->getLineHeight();
  int ystart = Engine::instance()->getSettings()->resolution.y / 3 - 3*2 - 2*lineHeight;
  for (size_t i = mScrollPos; i < mOutput.size(); ++i){
    std::vector<Vec2i> brinfo;
    std::string text = mOutput[i];
    fren->getTextExtent(text, 0, brinfo, Engine::instance()->getResolution().x - 5);
    fren->render(5, ystart, text, DEPTH_CONSOLE + 1, 0, brinfo, Color()/*Color(175, 191, 207, 255)*/, 0U, false, ALGN_LEFT);
    ystart -= lineHeight;
    if (ystart < -lineHeight) //no more lines visible
      break;
  }
}

void Console::show(bool doit){
  mActive = doit;
  if (doit){
    mScript->cancelFinish();
    mScript->setEvent(EVT_ENTER);
    mScrollPos = 0;
  }
  else{
    Engine::instance()->finishTextInput(false);
  }
}

void Console::print(const char* fmt, ...){
  char tmp[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(tmp, 1024, fmt, args);
  va_end(args);
  output(tmp);
}

void Console::input(const char* str){
  mHistoryPos = -1;
  mScrollPos = 0;
  String msg(str);
  msg.trim();
  if (!msg.empty()){
    mInput.push_front(msg);
    if (mInput.size() > 100)
      mInput.pop_back();
    output(msg.c_str());
    if (msg.find(' ') == msg.npos && msg.find('=') == msg.npos && msg.find(')') == msg.npos) //append call for convenience
      msg += "()";
    ExecutionContext* script = Engine::instance()->getInterpreter()->parseProgram(msg, LUA_SCRIPT);
    if (script){
      Engine::instance()->getInterpreter()->execute(script, true);
    }
  }
}

void Console::output(const char* str){
  mOutput.push_front(str);
  if (mOutput.size() > 200)
    mOutput.pop_back();
}

int Console::getDepth(){
  return DEPTH_CONSOLE;
}

void Console::historyUp(){
  if (mHistoryPos < (int)mInput.size()-1)
    ++mHistoryPos;
  if (mHistoryPos >= 0){
    std::string val = mInput[mHistoryPos];
    Engine::instance()->getInterpreter()->setVariable("!consoleInput", String(val));
  }
}

void Console::historyDown(){
  if (mHistoryPos > -1)
    --mHistoryPos;
  std::string val;
  if (mHistoryPos >= 0)
    val = mInput[mHistoryPos];
  Engine::instance()->getInterpreter()->setVariable("!consoleInput", String(val));
}

void Console::scrollUp(){
  if (mScrollPos + 2 < (int)mOutput.size())
    mScrollPos += 2;
}

void Console::scrollDown(){
  if (mScrollPos > 2)
    mScrollPos -= 2;
  else
    mScrollPos = 0;
}
