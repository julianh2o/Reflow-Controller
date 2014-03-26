#ifndef ReflowDisplay_h
#define ReflowDisplay_h

#include <arduino.h>

class ReflowDisplay {
public:
  ReflowDisplay(); //setup constructor
  
  void display(int n);
//  void display(string s);
  void display(char * s);
  void displayMarquee(char * chars);
  boolean marqueeComplete();
  void setSegment(byte segment, byte index);
  void tick();
  void clear();
  
private:
  int tickCounter;
  byte displayedDigits[3];
  char * marqueeString;
  int marqueeLength;
  int marqueeIndex;
  boolean marqueeCompleteFlag;
  
  byte getLetter(char a);
  void displayChars(char * chars, int len);
  void displayDigit(byte segments, byte displayDigit);
  void stopMarquee();
  void marqueeHandler();

  //TODO make this configurable
  static const int DDP = A4;
  static const int DA = 5;
  static const int DB = 13;
  static const int DD = A0;
  static const int DE = A1;
  static const int DF = A2;
  static const int DG = A3;
  static const int D3 = 8;
  static const int D2 = 9;
  static const int D1 = 10;
  
  static byte numerals[];
  static byte alphabet[];
};

#endif
