#ifndef ReflowMenuItem_h
#define ReflowMenuItem_h

#include <arduino.h>

class ReflowMenuItem {
public:
  ReflowMenuItem(char* name);
  virtual int show();
  char * getName();
  
private:
  char * displayName;
};

#endif
