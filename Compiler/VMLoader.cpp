#include "VMLoader.h"
#include "VMclass.h"
#include <io/Tracing.h>
#include "VMContext.h"
#include <system/utilities.h>

TR_CHANNEL(Java_Loader);

VMLoader::VMLoader(VMObject* loader) : mLoader(loader) {

}

VMLoader::~VMLoader(){
  for (std::map<std::string, VMClass*>::iterator iter = mClasses.begin(); iter != mClasses.end(); ++iter){
    delete iter->second;
  }
  mClasses.clear();
  for (std::list<CGE::SOLoader*>::iterator iter = mLibs.begin(); iter != mLibs.end(); ++iter){
    delete *iter;
  }
}

VMClass* VMLoader::load(VMContext* ctx, const std::string& name, CGE::Reader& rdr){
  VMClass* cls = new VMClass(ctx, this, rdr);
  cls->initClass(ctx, true);
  if (ctx->getException() != NULL){
    delete cls;
    return NULL;
  }
  mClasses[name] = cls;
  return cls;
}

VMClass* VMLoader::find(VMContext* ctx, const std::string& name){
  JNIEnv* env = ctx->getJNIEnv();
  std::string binName = name;
  CGE::Utilities::replaceWith(binName, '/', '.');
  jstring str = env->NewStringUTF(binName.c_str());
  VMClass* ldrcls = ((VMObject*)mLoader)->getClass();
  jmethodID findClass = env->GetMethodID(ldrcls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
  VMClass* ret = (VMClass*)env->CallObjectMethod(mLoader, findClass, str);
  return ret;
}


VMClass* VMLoader::get(const std::string& name){
  std::map<std::string,VMClass*>::iterator iter = mClasses.find(name);
  if (iter == mClasses.end())
    return NULL;
  return iter->second;
}

nativeMethod VMLoader::findNativeMethod(const std::string& name){
  for (std::list<CGE::SOLoader*>::iterator iter = mLibs.begin(); iter != mLibs.end(); ++iter){
    nativeMethod mthd = (nativeMethod)(*iter)->getFunction(name.c_str());
    if (mthd != NULL)
      return mthd;
  }
  return NULL;
}

jlong VMLoader::addLibrary(const std::string& name){
  CGE::SOLoader* ldr = new CGE::SOLoader;
  if (!ldr->open(name, false)){
    //cannot load library
    delete ldr;
    return 0;
  }
  mLibs.push_back(ldr);
  return (jlong)ldr;
}

#include <stdlib.h>
#include <io/BinFileReader.h>
#include "JVM.h"
#include "VMArray.h"
#include "VMContext.h"

BootstrapLoader::BootstrapLoader(VMArgs* args) : VMLoader(NULL){
  TR_USE(Java_Loader);
  std::string prefix;
#ifndef UNDER_CE
  char* tmp = getenv("JAVA_HOME");
  if (tmp != NULL){
    prefix = tmp;
  }
  else{
    tmp = getenv("ProgramFiles");
    if (tmp != NULL){
      prefix = tmp;
      prefix += "/Java/jre6";
    }
  }
#endif
  if (!mRuntimeClasses.openFile(prefix+"/lib/rt.jar")){
    //hack for my crappy environment
    prefix[0] = 'D';
    if (!mRuntimeClasses.openFile(prefix+"/lib/rt.jar")){
      TR_BREAK("Java runtime classes not found");
    }
  }
  if (!mRuntime.open("javaruntime")){
    TR_BREAK("Java runtime not found");
  }

  //TODO parse class path correctly
  //mFilePaths.push_back(args->classpath);
}

CGE::Reader* BootstrapLoader::filenameToReader(const std::string& filename){
  TR_USE(Java_Loader);

  char* buffer = NULL;
  bool classFound = false;
  CGE::Reader* reader = NULL;
  for (unsigned i = 0; i < mFilePaths.size(); ++i){
    CGE::BinFileReader* brdr = new CGE::BinFileReader(mFilePaths[i]+"/"+filename+".class");
    if (brdr->isWorking()){
      classFound = true;
      reader = brdr;
      break;
    }
    delete brdr;
  }
  if (!classFound){
    //try to load runtime jar
    TR_DEBUG("using jar mode");
    CGE::MemReader mrdr = getClassFile(filename+".class");
    if (!mrdr.isWorking()){
      TR_WARN("Class %s not found in jar", filename.c_str());
      /*JNIEnv* env = ctx->getJNIEnv();
      jclass exception = env->FindClass("java/lang/ClassNotFoundException");
      env->ThrowNew(exception, ("Class "+filename+" could not be found").c_str());*/
      return NULL;
    }
    reader = new CGE::MemReader(mrdr);
  }
  return reader;
}

VMClass* BootstrapLoader::find(VMContext* ctx, const std::string& name, bool initClass){
  TR_USE(Java_Loader);
  std::map<std::string,VMClass*>::iterator iter = mUninitializedClasses.find(name);
  if (iter != mUninitializedClasses.end()){
    VMClass* cls = iter->second;
    mUninitializedClasses.erase(iter);
    mClasses[name] = cls;
    //delayed class init
    unsigned idx = cls->findMethodIndex("<clinit>", "()V");
    VMMethod* mthd = cls->getMethod(idx);
    if (mthd){
      TR_INFO("Delayed execution of class init method");
      mthd->execute(ctx, -1);
    }
    return cls;
  }
  VMClass* entry = mClasses[name];
  if (entry == 0){
    //array functions
    if (name[0] == '['){
      entry = new VMArrayClass(this, name);
      mClasses[name] = entry;
      return entry;
    }
    else if (name.size() == 1){
      //primitive types
      return getPrimitiveClass(ctx, name);
    }
    //Java::ClassFile* clfile = new Java::ClassFile();
    CGE::Reader* rdr = filenameToReader(name);
    if (!rdr)
      return NULL;
    entry = new VMClass(ctx, this, *rdr);
    delete rdr;

    if (ctx->getException() != NULL){
      delete entry;
      return NULL;
    }

    if (!initClass)
      mUninitializedClasses[name] = entry;
    else
      mClasses[name] = entry;

    entry->initClass(ctx, initClass);
  }
  return entry;
}

VMClass* BootstrapLoader::getPrimitiveClass(VMContext* ctx, std::string name){
  VMClass* entry = mClasses[name];
  if (entry == 0){
    entry = new VMClass(this);
    entry->setName(name);

    mClasses[name] = entry;
    //entry->print(std::cout);

    //entry->initFields(ctx);

    VMClass* cls = find(ctx, "java/lang/Class");
    VMMethod* clsmthd = cls->getMethod(cls->findMethodIndex("<init>", "()V"));
    entry->init(ctx, cls);
    ctx->push((VMObject*)cls);
    clsmthd->execute(ctx, -1);
  }
  return entry;
}

CGE::MemReader BootstrapLoader::getClassFile(const std::string& filename){
  return mRuntimeClasses.openEntry(filename);
}

nativeMethod BootstrapLoader::findNativeMethod(const std::string& name){
  nativeMethod mthd = (nativeMethod)mRuntime.getFunction(name.c_str());
  if (mthd == NULL)
    return VMLoader::findNativeMethod(name);
  return mthd;
}
