//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |                 The Open Source game engine                      |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: fiel.cpp                                                   |
//  ==================================================================

#include "file.h"

#include "engine.h"
#include "script.h"

#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifndef UNDER_CE
#include <direct.h>
#endif
#endif
#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

using namespace CGE;
using std::string;
using std::vector;

#ifdef UNDER_CE
std::string Filesystem::cwd_ = std::string();
#endif


//get all directories in a directory
std::vector<std::string> Filesystem::getDirectories(const std::string& path){
  vector<string> result;
#ifdef WIN32
  string tmp = path;
  tmp.append("\\*");
  WIN32_FIND_DATA fileData;
  HANDLE hFind;
#ifdef UNICODE
  wchar_t wtmp[512];
  mbstowcs(wtmp, tmp.c_str(), 512);
  hFind = FindFirstFile(wtmp, &fileData);
#else
  hFind = FindFirstFile(tmp.c_str(), &fileData);
#endif
  if (hFind == INVALID_HANDLE_VALUE){
    return result;
  }
  if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
#ifdef UNICODE
    char ctmp[512];
    wcstombs(ctmp, fileData.cFileName, 512);
    result.push_back(ctmp);
#else
    result.push_back(fileData.cFileName);
#endif
  }
  while(FindNextFile(hFind, &fileData)){
    if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
#ifdef UNICODE
      char ctmp[512];
      wcstombs(ctmp, fileData.cFileName, 512);
      result.push_back(ctmp);
#else
      result.push_back(fileData.cFileName);
#endif
    }
  }
  FindClose(hFind);
#endif
#ifdef UNIX
  DIR* dir = opendir(path.c_str());
  if (!dir)
    return result;
  struct dirent* entry;
  while((entry = readdir(dir))){
    struct stat st;
    string tmp = path+"/"+entry->d_name;
    stat(tmp.c_str(), &st);
    if ((st.st_mode & S_IFDIR)){
      result.push_back(entry->d_name);
    }
  }
  closedir(dir);
#endif

  return result;
}

//get all directories in a directory
std::vector<std::string> Filesystem::getFiles(const std::string& path){
  vector<string> result;
#ifdef WIN32
  string tmp = path;
  tmp.append("\\*");
  WIN32_FIND_DATA fileData;
  HANDLE hFind;
#ifdef UNICODE
  wchar_t wtmp[512];
  mbstowcs(wtmp, tmp.c_str(), 512);
  hFind = FindFirstFile(wtmp, &fileData);
#else
  hFind = FindFirstFile(tmp.c_str(), &fileData);
#endif
  if (hFind == INVALID_HANDLE_VALUE){
    return result;
  }
  if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
#ifdef UNICODE
    char ctmp[512];
    wcstombs(ctmp, fileData.cFileName, 512);
    result.push_back(ctmp);
#else
    result.push_back(fileData.cFileName);
#endif
  }
  while(FindNextFile(hFind, &fileData)){
    if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
#ifdef UNICODE
      char ctmp[512];
      wcstombs(ctmp, fileData.cFileName, 512);
      result.push_back(ctmp);
#else
      result.push_back(fileData.cFileName);
#endif
    }
  }
  FindClose(hFind);
#endif
#ifdef UNIX
  DIR* dir = opendir(path.c_str());
  if (!dir)
    return result;
  struct dirent* entry;
  while((entry = readdir(dir))){
    struct stat st;
    string tmp = path+"/"+entry->d_name;
    stat(tmp.c_str(), &st);
    if (!(st.st_mode & S_IFDIR)){
      result.push_back(entry->d_name);
    }
  }
  closedir(dir);
#endif
  return result;
}

// get the working directory
std::string Filesystem::getCwd(){
  char cwd[2048];
#ifdef WIN32
#ifdef UNDER_CE
  //no working directory under CE
  if (cwd_.empty()){
    wchar_t tmp[256];
    GetModuleFileName(NULL, tmp, 256);
    char ctmp[256];
    wcstombs(ctmp, tmp, 256);
    cwd_ = std::string(ctmp);
    int idx = cwd_.find_last_of('\\');
    cwd_.erase(idx);
  }
  return cwd_;
#else
  _getcwd(cwd, 2048);
#endif
#endif
#ifdef UNIX
  getcwd(cwd, 2048);
#endif
  return string(cwd) + "/";
}

// change directory
bool Filesystem::changeDir(const std::string& path){
#ifdef WIN32
#ifdef UNDER_CE
  //TODO
  return true;
#else
  return SetCurrentDirectory(path.c_str()) == TRUE ? true : false;
#endif
#endif
#ifdef UNIX
  return chdir(path.c_str()) == 0 ? true : false;
#endif
}

// gets the path component from a full path
std::string Filesystem::getPathComponent(std::string const & filepath){
  size_t pos = std::string::npos;
  for (int i = (int)filepath.size() - 1; i >= 0; --i){
    char ch = filepath[i];
    if (ch == '/' || ch == '\\'){
      pos = i;
      break;
    }
  }
  return filepath.substr(0, pos);
}

// gets the file component from a full path
std::string Filesystem::getFileComponent(std::string const & filepath){
  size_t pos = std::string::npos;
  for (int i = (int)filepath.size() - 1; i >= 0; --i){
    char ch = filepath[i];
    if (ch == '/' || ch == '\\'){
      pos = i;
      break;
    }
  }
  return filepath.substr(pos+1);
}

// gets the extension from a file name
std::string Filesystem::getExtension(std::string const & filepath){
  size_t pos = filepath.find_last_of('.');
  if (pos == std::string::npos)
    return "";
  return filepath.substr(pos + 1);
}

// combines two path components
std::string Filesystem::combinePath(std::string const & p1, std::string const & p2){
#ifdef WIN32
  return p1 + '\\' + p2;
#else
  return p1 + '/' + p2;
#endif
}

bool Filesystem::createDir(const std::string& path){
#ifdef WIN32
  int ret = _mkdir(path.c_str());
  return ret == 0;
#else
  int ret = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
  return ret == 0;
#endif
}

//! remove directory
bool Filesystem::removeDir(const std::string& path, bool recursive){
  if (recursive){
    std::vector<std::string> dirs = getDirectories(path);
    for (size_t i = 0; i < dirs.size(); ++i){
      if (dirs[i] == "." || dirs[i] == "..")
        continue;
      removeDir(combinePath(path, dirs[i]), recursive);
    }
    std::vector<std::string> files = getFiles(path);
    for (size_t i = 0; i < files.size(); ++i){
      remove(combinePath(path, files[i]).c_str());
    }
  }
#ifdef WIN32
  int ret = _rmdir(path.c_str());
#else
  int ret = rmdir(path.c_str());
#endif
  return ret == 0;
}

bool Filesystem::exists(const std::string& path){
#ifdef WIN32
  return GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES;
#else
  struct stat buf;
  int ret = stat(path.c_str(), &buf);
  return ret == 0;
#endif
}

std::string Filesystem::getStorageDir(){
#ifdef ANDROID
   return std::string(CGE::Engine::instance()->getApp()->activity->internalDataPath) + "/";
#else
  return getCwd();
#endif
}

std::string Filesystem::getAssetDir(){
	std::string ret = Script::instance()->getStringSetting("assetDir");
	if (ret.empty())
	  ret = getCwd();
	return ret;
}