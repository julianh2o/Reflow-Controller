#include "ReflowMenuItem.h"

ReflowMenuItem::ReflowMenuItem(char * name) : displayName(name) {
}

int ReflowMenuItem::show() {
  return -1;
}

char * ReflowMenuItem::getName() {
  return displayName;
}
