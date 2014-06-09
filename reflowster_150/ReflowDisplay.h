#ifndef ReflowDisplay_h
#define ReflowDisplay_h

#include <arduino.h>

class ReflowDisplay {
public:
  ReflowDisplay(int DS, int STCP, int SHCP, int D1, int D2, int D3); //setup constructor
  
  void display(int n);
//  void display(string s);
  void display(char * s);
  void displayMarquee(char * chars);
  boolean marqueeComplete();
  void setSegment(byte segment, byte index);
  void tick();
  void clear();
  
    void displayDigit(byte segments, byte displayDigit);  //TODO make private
  
private:
  int tickCounter;
  byte displayedDigits[3];
  char * marqueeString;
  int marqueeLength;
  int marqueeIndex;
  boolean marqueeCompleteFlag;
  
  byte getLetter(char a);
  void displayChars(char * chars, int len);
  void stopMarquee();
  void marqueeHandler();
  
  static byte numerals[];
  static byte alphabet[];
};

#endif
