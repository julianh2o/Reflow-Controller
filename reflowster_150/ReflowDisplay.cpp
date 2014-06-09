#include "ReflowDisplay.h"

//TODO remove these, but keep the function somehow
int MARQUEE_START_WAIT = 3;
int MARQUEE_END_WAIT = 3;

byte ReflowDisplay::numerals[] = {0b01111110,0b00110000,0b01101101,0b01111001,0b00110011,0b01011011,0b00011111,0b01110000,0b01111111,0b01110011};
byte ReflowDisplay::alphabet[] = {
    0b01110111, //a
    0b00011111, //b
    0b00001101, //c
    0b00111101, //d
    0b01001111, //e
    0b01000111, //f
    0b01011110, //g
    0b00110111, //h
    0b00010000, //i
    0b00111100, //j
    0b01010111, //k
    0b00001110, //l
    0b01010100, //m
    0b00010101, //n
    0b00011101, //o
    0b01100111, //p
    0b01110011, //q
    0b00000101, //r
    0b01011011, //s
    0b00001111, //t
    0b00011100, //u
    0b00101010, //v
    0b00101011, //w
    0b00110111, //x
    0b00111011, //y
    0b01101101  //z
};

int DS,STCP,SHCP,D1,D2,D3; //tODO protect these variables

ReflowDisplay::ReflowDisplay(int cDS, int cSTCP, int cSHCP, int cD1, int cD2, int cD3) {
  DS = cDS;
  STCP = cSTCP;
  SHCP = cSHCP;
  D1 = cD1;
  D2 = cD2;
  D3 = cD3;
  
  pinMode(DS, OUTPUT);
  pinMode(STCP, OUTPUT);
  pinMode(SHCP, OUTPUT);
  
  pinMode(D3, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(8, OUTPUT);
  
  tickCounter = 0;
  displayedDigits[0] = 0;
  displayedDigits[1] = 0;
  displayedDigits[2] = 0;
  marqueeString = "\0";
  marqueeLength = 0;
  marqueeIndex = 0;
  marqueeCompleteFlag = false;
}

void ReflowDisplay::display(int n) {
  stopMarquee();
  char neg = 0;
  if (n < 0) {
    n = n*-1;
    neg = 1;
  }
  displayedDigits[2] = numerals[n % 10] | (neg?0b10000000:0); //TODO fix the way we handle negative numbers
  displayedDigits[1] = n < 10 ? 0 : numerals[(n/10) % 10];
  displayedDigits[0] = n < 100 ? 0 : numerals[(n/100) % 10]; 
}

void ReflowDisplay::display(char * s) {
  Serial.println("display: ");
  Serial.println(s);
  stopMarquee();
  byte len = strlen(s);
  if (len > 3) len = 3;
  displayChars(s,len);
}

boolean ReflowDisplay::marqueeComplete() {
  return marqueeCompleteFlag;
}

void ReflowDisplay::displayMarquee(char * chars) {
  marqueeIndex = -MARQUEE_START_WAIT;
  marqueeCompleteFlag = false;
  marqueeString = chars;
  marqueeLength = strlen(chars);
  displayChars(marqueeString,3);
}

void ReflowDisplay::clear() {
  display("   ");
}

void ReflowDisplay::marqueeHandler() {
  if (marqueeLength == 0) return; //no current marquee
  
  //if (marqueeIndex >= 0 && marqueeIndex <= marqueeLength-3) {
   if (marqueeIndex >= marqueeLength-3+MARQUEE_END_WAIT) { //last index after the end pause is blank
     displayChars("   ",3);
   } else if (marqueeIndex >= marqueeLength-3) { //after scrolling, pause on the last 3 chars
     displayChars(marqueeString+marqueeLength-3,3);
   } else if (marqueeIndex >= 0) { //this is the meat of the string and the scroll
     displayChars(marqueeString+marqueeIndex,3);
   } else if (marqueeIndex == -MARQUEE_START_WAIT) { //prior to scrolling, pause on the first 3 chars
     displayChars(marqueeString,3);
   }
  
  if (marqueeLength > 3) marqueeIndex++;  //no scrolling unless the string is more than 3 chars long
  if (marqueeIndex >= marqueeLength-3+MARQUEE_END_WAIT+1) {
    //marquee has wrapped.. we consider it complete now
    marqueeCompleteFlag = true;
    marqueeIndex = -MARQUEE_START_WAIT;
  }
}

void ReflowDisplay::stopMarquee() {
  marqueeLength = 0;
}

byte ReflowDisplay::getLetter(char a) {
  if (a == '-') return 0b00000001;
  if (a>=65 && a<=90) return alphabet[a-65];
  if (a>=97 && a<=122) return alphabet[a-97];
  return 0;
}

void ReflowDisplay::displayChars(char * chars, int len) {
  displayedDigits[0] = getLetter(chars[0]);
  displayedDigits[1] = len > 1 ? getLetter(chars[1]) : 0;
  displayedDigits[2] = len > 2 ? getLetter(chars[2]) : 0;
}

void ReflowDisplay::setSegment(byte segment, byte index) {
  displayedDigits[index] = segment;
}

void ReflowDisplay::tick() {
  if (tickCounter % 200 == 0) {
    marqueeHandler();
    tickCounter = 0;
  }
  
  displayDigit(displayedDigits[tickCounter % 3],tickCounter % 3); //cycles through the digits displaying each one for a tick
  
  tickCounter++;
}

void ReflowDisplay::displayDigit(byte segments, byte displayDigit) {
  digitalWrite(D1,LOW);
  digitalWrite(D2,LOW);
  digitalWrite(D3,LOW);
  
//  Serial.println(segments,BIN);
//  segments = 0b00000000;
  //This is performing the pin mapping to account for the arbitrary order of the lettered display pins with the output pins of the shift register
//  Serial.print("n = 0b");
//  Serial.print(n,BIN);
//  Serial.println();
  unsigned char mapped = (0b10000000 & (segments << 0)) | //TODO make this clearer and more obvious??
                (0b01000000 & (segments << 3)) |
                (0b00100000 & (segments << 1)) |
                (0b00010000 & (segments << 2)) |
                (0b00001000 & (segments << 2)) |
                (0b00000100 & (segments >> 4)) |
                (0b00000010 & (segments >> 4)) |
                (0b00000001 & (segments << 0));
                
//  Serial.print("m = 0b");
//  Serial.print(mapped,BIN);
//  Serial.println();

  digitalWrite(STCP, LOW);
  for (char i=0; i<8; i++) {
    digitalWrite(SHCP,LOW);
    digitalWrite(DS,!(mapped&1));
    digitalWrite(SHCP,HIGH);
    mapped = mapped >> 1;
  }
  digitalWrite(STCP, HIGH);
  
  digitalWrite(D1,displayDigit == 0 ? HIGH : LOW);
  digitalWrite(D2,displayDigit == 1 ? HIGH : LOW);
  digitalWrite(D3,displayDigit == 2 ? HIGH : LOW);
}
