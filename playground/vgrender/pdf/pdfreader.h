#ifndef PDF_READER_H
#define PDF_READER_H

#include <string>
#include <io/Reader.h>
#include "pdfdocument.h"

class PdfReader{
public:
  PdfReader(const std::string& file);
  PdfDocument* readDocument();
  pdf::Reference readCrt(unsigned crtOffset, pdf::CrossReferenceTable& crt);
  pdf::Object* readObject(unsigned offset, pdf::Reference& refr);
protected:
  void findBackwards(const std::string what);
  
  const char* getNextToken(bool breakAtSlash = true);
  int getNextInt();
  const char* getNextDictEntry();
  pdf::Reference getNextReference();
  char mBuffer[512];
  char* mCurrent;

  CGE::Reader* mReader;
};

#endif
