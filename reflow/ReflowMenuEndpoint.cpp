#include "ReflowMenuEndpoint.h"

ReflowMenuEndpoint::ReflowMenuEndpoint(int num, char * name) : ReflowMenuItem(name), id(num) {
}

int ReflowMenuEndpoint::show() {
  return id;
}
