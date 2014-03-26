#ifndef ReflowMenuEndpoint_h
#define ReflowMenuEndpoint_h

#include <arduino.h>
#include "ReflowMenuItem.h"

class ReflowMenuEndpoint : public ReflowMenuItem {
public:
  ReflowMenuEndpoint(int num, char* name);
  virtual int show();
  
private:
  int id;
};

#endif
