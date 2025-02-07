#include "Sound.h"

#include <iostream>
#include <system/allocation.h>

#ifdef UNIX
#define UINT64_C(val) (uint64_t)val
#endif
#ifndef UINT64_C
#define UINT64_C(val) val##ui64
#endif
#ifndef INT64_C
#define INT64_C(val) val##i64
#endif

#ifdef WIN32
#pragma warning( disable : 4244 )
#endif

#ifndef DISABLE_SOUND
#include <AL/alut.h>
#ifndef DISABLE_EFX
#ifdef UNIX
#include <AL/efx.h>
#else
#include <AL/efx.h>
#include <AL/efx-creative.h>
//#include <EFX-Util.h>
#endif
#include "EFXeffects.h"
#ifndef AL_LOWPASS_MAX_GAIN
#define AL_LOWPASS_MAX_GAIN 1.0f
#endif
#endif
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
};
#endif
#include "AdvDoc.h"
#include "Engine.h"
#include "SwfPlayer.h"

using namespace adv;

TR_CHANNEL(ADV_SOUND_ENGINE);

SoundEngine* SoundEngine::mInstance = NULL;

#if !defined(DISABLE_SOUND) && !defined(DISABLE_EFX)
static LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = NULL;
static LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = NULL;
static LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = NULL;
static LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = NULL;
static LPALGENEFFECTS alGenEffects = NULL;
static LPALDELETEEFFECTS alDeleteEffects = NULL;
static LPALEFFECTF alEffectf = NULL;
static LPALEFFECTFV alEffectfv = NULL;
static LPALEFFECTI alEffecti = NULL;
static LPALGENFILTERS alGenFilters = NULL;
static LPALDELETEFILTERS alDeleteFilters = NULL;
static LPALFILTERI alFilteri = NULL;
static LPALFILTERF alFilterf = NULL;
#endif

#ifdef FFMPEG_ANCIENT_API
#define AVMEDIA_TYPE_AUDIO CODEC_TYPE_AUDIO
#define AVMEDIA_TYPE_VIDEO CODEC_TYPE_VIDEO
#define AV_SAMPLE_FMT_

int avcodec_decode_audio3(AVCodecContext* avctx, int16_t* samples, int* frame_size_ptr, AVPacket* avpkt){
  return avcodec_decode_audio2(avctx, samples, frame_size_ptr, avpkt->data, avpkt->size);
}

int avcodec_decode_video2(AVCodecContext* avctx, AVFrame* picture, int* got_picture_ptr, AVPacket* avpkt){
  return avcodec_decode_video(avctx, picture, got_picture_ptr, avpkt->data, avpkt->size);
}

#endif

SoundEngine::SoundEngine() : mData(NULL), mActiveMusic(NULL), mActiveVideo(NULL), mMusicVolume(1.0f), mSpeechVolume(1.0f), mCurrentEffect("none"), mIsDSPEffect(false), mFadingTime(300), mSpeedFactor(1.0){
  TR_USE(ADV_SOUND_ENGINE);
#ifndef DISABLE_SOUND
  mDevice = alcOpenDevice(NULL);
  if (mDevice){
    mContext = alcCreateContext(mDevice, NULL);
    alcMakeContextCurrent(mContext);
  }
#ifndef DISABLE_EFX
  if (!alcIsExtensionPresent(mDevice, ALC_EXT_EFX_NAME)){
    TR_ERROR("EFX exension not present");
  }
  else{
    alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
    alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
    alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
    alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
    alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
    alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
    alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
    alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
    alGenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
    alDeleteFilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
    alFilteri = (LPALFILTERI)alGetProcAddress("alFilteri");
    alFilterf = (LPALFILTERF)alGetProcAddress("alFilterf");

    alGenAuxiliaryEffectSlots(1, &mEffectSlot);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR){
      TR_ERROR("AL error %i", error);
    }
    //alAuxiliaryEffectSloti(mEffectSlot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, AL_FALSE);
    //alAuxiliaryEffectSlotf(mEffectSlot, AL_EFFECTSLOT_GAIN, 1.0f);
    alGenEffects(6, mEffect);
    alEffecti(mEffect[0], AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
    error = alGetError();
    if (error != AL_NO_ERROR)
      TR_WARN("No eaxreverb effect available");
    alEffecti(mEffect[1], AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    error = alGetError();
    if (error != AL_NO_ERROR)
      TR_WARN("No reverb effect available");
    alEffecti(mEffect[2], AL_EFFECT_TYPE, AL_EFFECT_ECHO);
    error = alGetError();
    if (error != AL_NO_ERROR)
      TR_WARN("No echo effect available");
    alEffecti(mEffect[3], AL_EFFECT_TYPE, AL_EFFECT_CHORUS);
    error = alGetError();
    if (error != AL_NO_ERROR)
      TR_WARN("No chorus effect available");
    alEffecti(mEffect[4], AL_EFFECT_TYPE, AL_EFFECT_DISTORTION);
    error = alGetError();
    if (error != AL_NO_ERROR)
      TR_WARN("No distortion effect available");
    alEffecti(mEffect[5], AL_EFFECT_TYPE, AL_EFFECT_FLANGER);
    error = alGetError();
    if (error != AL_NO_ERROR)
      TR_WARN("No flanger effect available");
    alGenFilters(2, mFilters);
    alFilteri(mFilters[0], AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    alFilteri(mFilters[1], AL_FILTER_TYPE, AL_FILTER_LOWPASS);
  }
#endif
  //init codecs
  alutInitWithoutContext(NULL, NULL);
  av_register_all();
  av_log_set_level(AV_LOG_ERROR);
#endif
#ifndef ENGINE_SINGLE_THREADED
  mThread.setPriority(CGE::Thread::LOW);
  //start();
#endif
}

SoundEngine::~SoundEngine(){
  //stop();
  reset();
#ifndef DISABLE_SOUND
#ifndef DISABLE_EFX
  alDeleteFilters(2, mFilters);
  alDeleteEffects(6, mEffect);
  alDeleteAuxiliaryEffectSlots(1, &mEffectSlot);
#endif
  alcMakeContextCurrent(NULL);
  alcDestroyContext(mContext);
  alcCloseDevice(mDevice);
  //exit codecs
  alutExit();
#endif
}

void SoundEngine::reset(){
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    delete iter->second;
  }
  mActiveSounds.clear();
  delete mActiveMusic;
  mActiveMusic = NULL;
  delete mActiveVideo;
  mActiveVideo = NULL;
  mMusicVolume = 1.0f;
  mSpeechVolume = 1.0f;
  mCurrentEffect = "none";
  mIsDSPEffect = false;
  mFadingTime = 300;
  mSpeedFactor = 1.0f;
  setMasterVolume(1.0f);
}

void SoundEngine::setEAXEffect(const std::string& effect){
#if !defined(DISABLE_SOUND) && !defined(DISABLE_EFX)
  if (effect == "off" || effect == "none"){
    mCurrentEffect = "none";
    mIsDSPEffect = false;
  }
  else{
    EFXREVERBPROPERTIES efxReverb;
    if (effect == "paddedcell"){
      efxReverb = REVERB_PRESET_PADDEDCELL;
    }
    else if (effect == "livingroom"){
      efxReverb = REVERB_PRESET_LIVINGROOM;
    }
    else if (effect == "cave"){
      efxReverb = REVERB_PRESET_CAVE;
    }
    else if (effect == "carpetedhallway"){
      efxReverb = REVERB_PRESET_CARPETTEDHALLWAY;
    }
    else if (effect == "alley"){
      efxReverb = REVERB_PRESET_ALLEY;
    }
    else if (effect == "mountains"){
      efxReverb = REVERB_PRESET_MOUNTAINS;
    }
    else if (effect == "parkinglot"){
      efxReverb = REVERB_PRESET_PARKINGLOT;
    }
    else if (effect == "drugged"){
      efxReverb = REVERB_PRESET_DRUGGED;
    }
    else if (effect == "concerthall"){
      efxReverb = REVERB_PRESET_CONCERTHALL;
    }
    else if (effect == "room"){
      efxReverb = REVERB_PRESET_ROOM;
    }
    else if (effect == "stoneroom"){
      efxReverb = REVERB_PRESET_STONEROOM;
    }
    else if (effect == "arena"){
      efxReverb =  REVERB_PRESET_ARENA;
    }
    else if (effect == "hallway"){
      efxReverb = REVERB_PRESET_HALLWAY;
    }
    else if (effect == "forest"){
      efxReverb = REVERB_PRESET_FOREST;
    }
    else if (effect == "quarry"){
      efxReverb = REVERB_PRESET_QUARRY;
    }
    else if (effect == "sewerpipe"){
      efxReverb = REVERB_PRESET_SEWERPIPE;
    }
    else if (effect == "dizzy"){
      efxReverb = REVERB_PRESET_DIZZY;
    }
    else if (effect == "bathroom"){
      efxReverb = REVERB_PRESET_BATHROOM;
    }
    else if (effect == "auditorium"){
      efxReverb = REVERB_PRESET_AUDITORIUM;
    }
    else if (effect == "hangar"){
      efxReverb = REVERB_PRESET_HANGAR;
    }
    else if (effect == "stonecorridor"){
      efxReverb = REVERB_PRESET_STONECORRIDOR;
    }
    else if (effect == "city"){
      efxReverb = REVERB_PRESET_CITY;
    }
    else if (effect == "plain"){
      efxReverb = REVERB_PRESET_PLAIN;
    }
    else if (effect == "underwater"){
      efxReverb = REVERB_PRESET_UNDERWATER;
    }
    else if (effect == "psychotic"){
      efxReverb = REVERB_PRESET_PSYCHOTIC;
    }
    else{
      TR_USE(ADV_SOUND_ENGINE);
      TR_BREAK("Unknown effect %s", effect.c_str());
    }
    mCurrentEffect = effect;
    mIsDSPEffect = false;
    alEffectf(mEffect[0], AL_EAXREVERB_DENSITY, efxReverb.flDensity);
    alEffectf(mEffect[0], AL_EAXREVERB_DIFFUSION, efxReverb.flDiffusion);
    alEffectf(mEffect[0], AL_EAXREVERB_GAIN, efxReverb.flGain);
    alEffectf(mEffect[0], AL_EAXREVERB_GAINHF, efxReverb.flGainHF);
    alEffectf(mEffect[0], AL_EAXREVERB_GAINLF, efxReverb.flGainLF);
    alEffectf(mEffect[0], AL_EAXREVERB_DECAY_TIME, efxReverb.flDecayTime);
    alEffectf(mEffect[0], AL_EAXREVERB_DECAY_HFRATIO, efxReverb.flDecayHFRatio);
    alEffectf(mEffect[0], AL_EAXREVERB_DECAY_LFRATIO, efxReverb.flDecayLFRatio);
    alEffectf(mEffect[0], AL_EAXREVERB_REFLECTIONS_GAIN, efxReverb.flReflectionsGain);
    alEffectf(mEffect[0], AL_EAXREVERB_REFLECTIONS_DELAY, efxReverb.flReflectionsDelay);
    alEffectfv(mEffect[0], AL_EAXREVERB_REFLECTIONS_PAN, efxReverb.flReflectionsPan);
    alEffectf(mEffect[0], AL_EAXREVERB_LATE_REVERB_GAIN, efxReverb.flLateReverbGain);
    alEffectf(mEffect[0], AL_EAXREVERB_LATE_REVERB_DELAY, efxReverb.flLateReverbDelay);
    alEffectfv(mEffect[0], AL_EAXREVERB_LATE_REVERB_PAN, efxReverb.flLateReverbPan);
    alEffectf(mEffect[0], AL_EAXREVERB_ECHO_TIME, efxReverb.flEchoTime);
    alEffectf(mEffect[0], AL_EAXREVERB_ECHO_DEPTH, efxReverb.flEchoDepth);
    alEffectf(mEffect[0], AL_EAXREVERB_MODULATION_TIME, efxReverb.flModulationTime);
    alEffectf(mEffect[0], AL_EAXREVERB_MODULATION_DEPTH, efxReverb.flModulationDepth);
    alEffectf(mEffect[0], AL_EAXREVERB_AIR_ABSORPTION_GAINHF, efxReverb.flAirAbsorptionGainHF);
    alEffectf(mEffect[0], AL_EAXREVERB_HFREFERENCE, efxReverb.flHFReference);
    alEffectf(mEffect[0], AL_EAXREVERB_LFREFERENCE, efxReverb.flLFReference);
    alEffectf(mEffect[0], AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, efxReverb.flRoomRolloffFactor);
    alEffecti(mEffect[0], AL_EAXREVERB_DECAY_HFLIMIT, efxReverb.iDecayHFLimit);
    alAuxiliaryEffectSloti(mEffectSlot, AL_EFFECTSLOT_EFFECT, mEffect[0]);
    alFilterf(mFilters[0], AL_LOWPASS_GAIN, 1.0f); //dry mix
    alFilterf(mFilters[1], AL_LOWPASS_GAIN, 1.0f); //wet mix
  }
#endif
}


static inline float mBtoGain(float mb)
{
  return powf(10.0f, mb / 2000.0f);
}

bool SoundEngine::setDSPEffect(const std::string& effect){
  TR_USE(ADV_SOUND_ENGINE);
  if (effect == "off" || effect == "none"){
    mCurrentEffect = "none";
    mIsDSPEffect = true;
    return true;
  }
  std::map<std::string, DSPEffect>::const_iterator iter = Engine::instance()->getSettings()->dspeffects.find(effect);
  if (iter == Engine::instance()->getSettings()->dspeffects.end()){
    TR_BREAK("unknown dsp effect %s", effect.c_str());
    return false;
  }
#ifndef DISABLE_EFX
  const DSPEffect& eff = iter->second;
  switch (eff.type){
  case  DSPEffect::REVERB:{
    float dry = eff.params[0] / 1000.0f;
    alFilterf(mFilters[0], AL_LOWPASS_GAIN, dry > AL_LOWPASS_MAX_GAIN ? AL_LOWPASS_MAX_GAIN : dry); //dry mix
    float wet = eff.params[1] / 1000.0f;
    alFilterf(mFilters[1], AL_LOWPASS_GAIN, wet > AL_LOWPASS_MAX_GAIN ? AL_LOWPASS_MAX_GAIN : wet); //wet mix
    float rsize = eff.params[2] / 1000.0f;
    alEffectf(mEffect[eff.type], AL_REVERB_DECAY_TIME, rsize*(AL_REVERB_MAX_DECAY_TIME - AL_REVERB_MIN_DECAY_TIME) + AL_REVERB_MIN_DECAY_TIME);
    alEffectf(mEffect[eff.type], AL_REVERB_REFLECTIONS_DELAY, rsize*(AL_REVERB_MAX_REFLECTIONS_DELAY - AL_REVERB_MIN_REFLECTIONS_DELAY) + AL_REVERB_MIN_REFLECTIONS_DELAY);
    alEffectf(mEffect[eff.type], AL_REVERB_LATE_REVERB_DELAY, rsize*(AL_REVERB_MAX_LATE_REVERB_DELAY - AL_REVERB_MIN_LATE_REVERB_DELAY) + AL_REVERB_MIN_LATE_REVERB_DELAY);

    float damping = eff.params[3] / 1000.0f;
    alEffectf(mEffect[eff.type], AL_REVERB_DENSITY, damping);

    float width = eff.params[4] / 1000.0f;
    alEffectf(mEffect[eff.type], AL_REVERB_DIFFUSION, width);
    break;
  }
  case  DSPEffect::ECHO:{
    float dry = abs(eff.params[0]) / 1000.0f;
    alFilterf(mFilters[0], AL_LOWPASS_GAIN, dry > AL_LOWPASS_MAX_GAIN ? AL_LOWPASS_MAX_GAIN : dry); //dry mix
    float wet = (abs(eff.params[1]) / 1000.0f)*0.1 + 0.9;
    alFilterf(mFilters[1], AL_LOWPASS_GAIN, wet > AL_LOWPASS_MAX_GAIN ? AL_LOWPASS_MAX_GAIN : wet); //wet mix
    alEffectf(mEffect[eff.type], AL_ECHO_DAMPING, AL_ECHO_MIN_DAMPING);
    float feedback = abs(eff.params[2]) / 1000.f;
    alEffectf(mEffect[eff.type], AL_ECHO_FEEDBACK, feedback > AL_ECHO_MAX_FEEDBACK ? AL_ECHO_MAX_FEEDBACK : feedback);
    float delay = eff.params[3] / 1000.f;
    alEffectf(mEffect[eff.type], AL_ECHO_DELAY, delay > AL_ECHO_MAX_DELAY ? AL_ECHO_MAX_DELAY : delay);
    alEffectf(mEffect[eff.type], AL_ECHO_LRDELAY, delay > AL_ECHO_MAX_LRDELAY ? AL_ECHO_MAX_LRDELAY : delay);
    alEffectf(mEffect[eff.type], AL_ECHO_SPREAD, eff.params[2] < 0 ? -1.0 : 1.0);
    break;
  }
  case  DSPEffect::CHORUS:{
    float dry = eff.params[0] / 1000.0f;
    alFilterf(mFilters[0], AL_LOWPASS_GAIN, dry > AL_LOWPASS_MAX_GAIN ? AL_LOWPASS_MAX_GAIN : dry); //dry mix
    float wet = (abs(eff.params[1]) / 1000.0f);
    alFilterf(mFilters[1], AL_LOWPASS_GAIN, wet > AL_LOWPASS_MAX_GAIN ? AL_LOWPASS_MAX_GAIN : wet); //wet mix
    float sweep = eff.params[4] / 400.0f * 0.016f;
    alEffectf(mEffect[eff.type], AL_CHORUS_DELAY, sweep > AL_CHORUS_MAX_DELAY ? AL_CHORUS_MAX_DELAY : sweep);
    float rate = eff.params[5] / 400.0f * 10.0f;
    alEffectf(mEffect[eff.type], AL_CHORUS_RATE, rate > AL_CHORUS_MAX_RATE ? AL_CHORUS_MAX_RATE : rate);
    alEffectf(mEffect[eff.type], AL_CHORUS_FEEDBACK, 0.75f);
    alEffectf(mEffect[eff.type], AL_CHORUS_DEPTH, 0.5f);
    break;
  }
  case  DSPEffect::DISTORTION:{
    float dry = abs(eff.params[0]) / 1000.0f * 2;
    alFilterf(mFilters[0], AL_LOWPASS_GAIN, dry > AL_LOWPASS_MAX_GAIN ? AL_LOWPASS_MAX_GAIN : dry); //dry mix
    float wet = (abs(eff.params[1]) / 1000.0f) * 2;
    alFilterf(mFilters[1], AL_LOWPASS_GAIN, wet > AL_LOWPASS_MAX_GAIN ? AL_LOWPASS_MAX_GAIN : wet); //wet mix
    float volume = eff.params[3] / 1000.f;
    alEffectf(mEffect[eff.type], AL_DISTORTION_GAIN, volume > AL_DISTORTION_MAX_GAIN ? AL_DISTORTION_MAX_GAIN : volume);
    float drive = eff.params[4] / 300.f;
    alEffectf(mEffect[eff.type], AL_DISTORTION_EDGE, drive > AL_DISTORTION_MAX_EDGE ? AL_DISTORTION_MAX_EDGE : drive);
    alEffectf(mEffect[eff.type], AL_DISTORTION_LOWPASS_CUTOFF, AL_DISTORTION_MAX_LOWPASS_CUTOFF);
    break;
  }
  case  DSPEffect::PHASER:{
    float dry = abs(eff.params[0]) / 1000.0f;
    alFilterf(mFilters[0], AL_LOWPASS_GAIN, dry > AL_LOWPASS_MAX_GAIN ? AL_LOWPASS_MAX_GAIN : dry); //dry mix
    float wet = (abs(eff.params[1]) / 1000.0f);
    alFilterf(mFilters[1], AL_LOWPASS_GAIN, wet > AL_LOWPASS_MAX_GAIN ? AL_LOWPASS_MAX_GAIN : wet); //wet mix
    float feedback = eff.params[2] / 1000.0f;
    alEffectf(mEffect[eff.type], AL_FLANGER_FEEDBACK, feedback > AL_FLANGER_MAX_FEEDBACK ? AL_FLANGER_MAX_FEEDBACK : feedback);
    float rate = eff.params[3] / 1000.0f;
    alEffectf(mEffect[eff.type], AL_FLANGER_RATE, rate > AL_FLANGER_MAX_RATE ? AL_FLANGER_MAX_RATE : rate);
    float range = eff.params[4] / 10000.0f;
    alEffectf(mEffect[eff.type], AL_FLANGER_DEPTH, range > AL_FLANGER_MAX_DEPTH ? AL_FLANGER_MAX_DEPTH : range);
    alEffecti(mEffect[eff.type], AL_FLANGER_PHASE, 180);
    alEffecti(mEffect[eff.type], AL_FLANGER_WAVEFORM, 0);
    break;
  }
  };
  alAuxiliaryEffectSloti(mEffectSlot, AL_EFFECTSLOT_EFFECT, mEffect[eff.type]);
#endif
  mCurrentEffect = effect;
  mIsDSPEffect = true;
  return true;
}

SoundPlayer* SoundEngine::getSound(const std::string& name, int flags){
  return getSound(name, isEffectEnabled(), flags);
}

SoundPlayer* SoundEngine::getSound(const std::string& name, const std::string& effect, int flags){
  std::string oldEffect = mCurrentEffect;
  setDSPEffect(effect);
  SoundPlayer* ret = getSound(name, isEffectEnabled(), flags);
  setDSPEffect(oldEffect);
  return ret;
}

SoundPlayer* SoundEngine::getSound(const std::string& name, bool effectEnabled, int flags){
  TR_USE(ADV_SOUND_ENGINE);
  mMutex.lock();
  if (!(flags & PLAYER_CREATE_ALWAYS)){
    std::multimap<std::string,SoundPlayer*>::iterator lowerit = mActiveSounds.find(name);
    if (lowerit != mActiveSounds.end() && lowerit->second != NULL){
      SoundPlayer* ret = lowerit->second;
      mMutex.unlock();
      return ret;
    }
  }
  DataBuffer* db = new DataBuffer();
  TR_DETAIL("buffer %p is %s", db, name.c_str());
  if (!mData->getSound(name, *db)){
    delete db;
    mMutex.unlock();
    return NULL;
  }
  SoundPlayer* plyr = createPlayer(name, *db, effectEnabled);
  //TR_WARN("creating player %s %p", name.c_str(), plyr);
  if (plyr && !(flags & PLAYER_UNREALIZED))
    plyr->realize();
  if (plyr && (flags & PLAYER_UNMANAGED))
    plyr->setAutoDelete(false);
  mActiveSounds.insert(std::make_pair(name, plyr));
  mMutex.unlock();
  return plyr;
}

SoundPlayer* SoundEngine::getMusic(const std::string& name){
  return getMusic(name, isEffectEnabled());
}

SoundPlayer* SoundEngine::getMusic(const std::string& name, bool effectEnabled){
  SoundPlayer* plyr = NULL;
  if (name.empty() || (mActiveMusic != NULL && mActiveMusic->getName() == name)){
    return mActiveMusic;
  }
  //else
  //  plyr = mActiveSounds[name];
  if (plyr)
    return plyr;
  DataBuffer* db = new DataBuffer();
  mData->getMusic(name, *db);
  plyr = createPlayer(name, *db, effectEnabled);
  if (plyr)
    plyr->realize();
  if (mActiveMusic){
    //crossfade
    mActiveMusic->fadeVolume(mMusicVolume, 0.0f, mFadingTime);
    std::multimap<std::string, SoundPlayer*>::iterator oldplayer = mActiveSounds.find(mActiveMusic->getName());
    if (oldplayer != mActiveSounds.end()){
      if (oldplayer->second){
        oldplayer->second->stop();
        delete oldplayer->second;
      }
      mActiveSounds.erase(oldplayer);
    }
    mActiveSounds.insert(std::make_pair(mActiveMusic->getName(), mActiveMusic));
  }
  mActiveMusic = plyr;
  if (plyr){
    plyr->fadeVolume(0.0f, mMusicVolume, mFadingTime);
  }
  else{
    delete db;
  }
  return plyr;
}

VideoPlayer* SoundEngine::getMovie(const std::string& name, bool isSwf){
  VideoPlayer* plyr = NULL;
  if (name.empty()){
    return mActiveVideo;
  }
  DataBuffer* db = new DataBuffer();
  mData->getMovie(name, *db);
  plyr = createVideoPlayer(name, *db, isSwf);
  if (plyr)
    plyr->realize();
  if (mActiveVideo){
    mActiveVideo->stop();
    delete mActiveVideo;
  }
  mActiveVideo = plyr;
  return plyr;
}

SoundPlayer* SoundEngine::createPlayer(const std::string& name, const DataBuffer& db, bool effectEnabled){
#ifndef DISABLE_SOUND
  if (db.data == NULL || db.length == 0){
    return NULL;
  }
  /*char* tmp = tmpnam(NULL);
  std::string filename = mData->getProjectSettings()->savedir
#ifdef WIN32
+"/tmp"
#endif
+tmp+db.name;
  FILE* f = fopen(filename.c_str(), "wb");
  if (!f)
    return NULL;
  fwrite(db.data, 1, db.length, f);
  fclose(f);*/
  //ALuint buffer = alutCreateBufferFromFileImage(db.data, db.length);
  //SoundPlayer* plyr = new SimpleSoundPlayer(buffer);
  StreamSoundPlayer* plyr = new StreamSoundPlayer(name, effectEnabled);
  plyr->setStream(db);
  //if (plyr->openStream(db)){
    plyr->setSpeed(mSpeedFactor);
    return plyr;
  /*}
  else{
    delete plyr;
    return NULL;
  }*/
#else
  return NULL;
#endif
}

VideoPlayer* SoundEngine::createVideoPlayer(const std::string& name, const DataBuffer& db, bool isSwf){
#ifndef DISABLE_SOUND
  if (db.data == NULL || db.length == 0){
    return NULL;
  }
  if (isSwf){
    SwfPlayer* plyr = new SwfPlayer(name, &db);
    return plyr;
  }
  /*char* tmp = tmpnam(NULL);
  std::string filename = mData->getProjectSettings()->savedir
#ifdef WIN32
+"/tmp"
#endif
+tmp+db.name;
  FILE* f = fopen(filename.c_str(), "wb");
  if (!f)
    return NULL;
  fwrite(db.data, 1, db.length, f);
  fclose(f);*/
  //ALuint buffer = alutCreateBufferFromFileImage(db.data, db.length);
  //SoundPlayer* plyr = new SimpleSoundPlayer(buffer);
  StreamVideoPlayer* plyr = new StreamVideoPlayer(name);
  plyr->setStream(db);
  //if (plyr->openStream(db)){
  return plyr;
  //}
  /*else{
    delete plyr;
    return NULL;
  }*/
#else
  return NULL;
#endif
}

void SoundEngine::update(unsigned time){
  run(time);
  if (mActiveVideo)
    mActiveVideo->render(time);
}

void SoundEngine::removeSpeaker(CharacterObject* chr){
  mMutex.lock();
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && iter->second->getSpeaker() == chr){
      delete iter->second;
      iter->second = NULL;
    }
  }
  mMutex.unlock();
}

void SoundEngine::setMusicVolume(float volume){
  mMusicVolume = volume;
  if (mActiveMusic != NULL){
    if (!mActiveMusic->isFading())
      mActiveMusic->setVolume(volume);
  }
}

void SoundEngine::setSpeechVolume(float volume){
  mSpeechVolume = volume;
}

std::ostream& SoundEngine::save(std::ostream& out){
  out << mMusicVolume << " " << mSpeechVolume << "\n";
  out << mFadingTime << " " << mCurrentEffect << " " << mIsDSPEffect << "\n";
  out << mSpeedFactor << std::endl;
  float volume;
  alGetListenerf(AL_GAIN, &volume);
  out << volume << std::endl;
  if (mActiveMusic != NULL){
    out << String(mActiveMusic->getName()) << " " << mActiveMusic->hasEffect();
  }
  else{
    out << String("none");
  }
  out << "\n";
  int loopcount = 0;
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && iter->second->isLooping())
      ++loopcount;
  }
  out << loopcount;
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && iter->second->isLooping()){
      out << " " << iter->second->getName() << " " << iter->second->hasEffect();
    }
  }
  out << "\n";
  return out;
}

std::istream& SoundEngine::load(std::istream& in, int version){
  //clear previous sounds
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && iter->second->hasAutoDeletion()){
      delete iter->second;
      iter->second = NULL;
    }
  }
  //mActiveSounds.clear(); //that does not work anymore with unmanaged sounds. Let the sound garbage collector do the work
  delete mActiveMusic;
  mActiveMusic = NULL;
  
  in >> mMusicVolume >> mSpeechVolume;
  in >> mFadingTime >> mCurrentEffect;
  if (version >= 2)
    in >> mIsDSPEffect;
  else
    mIsDSPEffect = false;
  if (mIsDSPEffect)
    setDSPEffect(mCurrentEffect);
  else
    setEAXEffect(mCurrentEffect);
  in >> mSpeedFactor;
  float volume = 1.0f;
  if (version >= 2)
    in >> volume;
  setMasterVolume(volume);
  std::string music;
  if (version >= 2){
    String m;
    in >> m;
    music = m;
  }
  else
    in >> music;
  if (music != "none"){
    bool effect;
    in >> effect;
    SoundPlayer* mp = getMusic(music, effect);
    if (mp)
      mp->play(true);
  }
  int loopcount;
  in >> loopcount;
  for (int i = 0; i < loopcount; ++i){
    std::string sound;
    bool effect;
    in >> sound >> effect;
    SoundPlayer* sp = getSound(sound, effect, PLAYER_DEFAULT);
    if (sp)
      sp->play(true);
  }
  return in;
}

void SoundEngine::removeSoundPlayer(SoundPlayer* plyr){
  TR_USE(ADV_SOUND_ENGINE);
  mMutex.lock();
  std::multimap<std::string, SoundPlayer*>::iterator lower = mActiveSounds.lower_bound(plyr->getName());
  std::multimap<std::string, SoundPlayer*>::iterator upper = mActiveSounds.upper_bound(plyr->getName());
  while(lower != upper){
    if (lower->second == plyr){
      mActiveSounds.erase(lower);
      break;
    }
    ++lower;
  }
  mMutex.unlock();
  plyr->stop();
  //TR_WARN("deleting player %p", plyr);
  delete plyr;
}

void SoundEngine::setSpeedFactor(float speed){
  mMutex.lock();
  mSpeedFactor = speed;
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second == NULL)
      continue;
    iter->second->setSpeed(speed);
  }
  if (mActiveMusic != NULL)
    mActiveMusic->setSpeed(speed);
  mMutex.unlock();
}

bool SoundEngine::run(){
  static int64 lastTime = (int64)(getTime() * 1000);
  int64 now = (int64)(getTime() * 1000);
  int time = now - lastTime;
  lastTime = now;
  return run(time);
}

bool SoundEngine::run(unsigned time){
  mMutex.lock();
  std::multimap<std::string, SoundPlayer*>::iterator garbage = mActiveSounds.end();
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && !iter->second->update(time)){
      delete iter->second;
      iter->second = NULL;
    }
    if (iter->second == NULL)
      garbage = iter;
  }
  if (garbage != mActiveSounds.end()){ //a little cleanup to get rid of old sounds
    mActiveSounds.erase(garbage);
    garbage = mActiveSounds.end();
  }
  if (mActiveMusic && !mActiveMusic->update(time)){
    delete mActiveMusic;
    mActiveMusic = NULL;
  }
  if (mActiveVideo && !mActiveVideo->update(time)){
    delete mActiveVideo;
    mActiveVideo = NULL;
  }
  mMutex.unlock();
  //CGE::Thread::sleep(10);
  return true;
}

void SoundEngine::setMasterVolume(float volume){
#ifndef DISABLE_SOUND
  alListenerf(AL_GAIN, volume);
#endif
}