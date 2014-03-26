#ifndef ReflowMenu_h
#define ReflowMenu_h

#include <arduino.h>
#include "ReflowMenuItem.h"

class ReflowMenu : public ReflowMenuItem {
public:
  ReflowMenu(char * name, ReflowMenuItem item[]);
  int show();
  
private:
  ReflowMenuItem* item;
};

#endif
