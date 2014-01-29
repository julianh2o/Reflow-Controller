#include "ReflowDisplay.h"

//TODO remove these
int MARQUEE_START_WAIT = 4;
int MARQUEE_END_WAIT = 1;

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

ReflowDisplay::ReflowDisplay() {
  pinMode(DDP, OUTPUT);
  pinMode(DA, OUTPUT);
  pinMode(DB, OUTPUT);
  //pinMode(DC, OUTPUT);
  DDRE = DDRE | 0b00000100;
  pinMode(DD, OUTPUT);
  pinMode(DE, OUTPUT);
  pinMode(DF, OUTPUT);
  pinMode(DG, OUTPUT);
  
  pinMode(D3, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D1, OUTPUT);
  
  tickCounter = 0;
  displayedDigits[0] = 0;
  displayedDigits[1] = 0;
  displayedDigits[2] = 0;
  marqueeString = "\0";
  marqueeLength = 0;
  marqueeIndex = 0;
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
  stopMarquee();
  byte len = strlen(s);
  if (len > 3) len = 3;
  displayChars(s,len);
}

void ReflowDisplay::displayMarquee(char * chars) {
  marqueeIndex = -MARQUEE_START_WAIT;
  marqueeIndex = 0;
  marqueeString = chars;
  marqueeLength = strlen(chars);
  displayChars(marqueeString,3);
}

void ReflowDisplay::clear() {
  display("   ");
}

void ReflowDisplay::marqueeHandler() {
  if (marqueeLength == 0) return; //no current marquee
  
  if (marqueeIndex >= 0 && marqueeIndex <= marqueeLength-3) {
    displayChars(marqueeString+marqueeIndex,3);
  }
  
  marqueeIndex++;
//  if (marqueeIndex >= marqueeLength) marqueeIndex = 0;
  if (marqueeIndex >= marqueeLength-3+MARQUEE_END_WAIT) marqueeIndex = -MARQUEE_START_WAIT;
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

void ReflowDisplay::tick() {
  if (tickCounter % 500 == 0) {
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
  
  digitalWrite(DDP,0b10000000 & segments ? LOW : HIGH);
  digitalWrite(DA,0b01000000 & segments ? LOW : HIGH);
  digitalWrite(DB,0b00100000 & segments ? LOW : HIGH);
  //temporary hack while this pin is PE
  //digitalWrite(DC,0b00010000 & val ? LOW : HIGH);
  PORTE = (0b00010000 & segments) ? (PORTE & (~0b00000100)) : (PORTE | 0b00000100);
  //end temporary hack
  digitalWrite(DD,0b00001000 & segments ? LOW : HIGH);
  digitalWrite(DE,0b00000100 & segments ? LOW : HIGH);
  digitalWrite(DF,0b00000010 & segments ? LOW : HIGH);
  digitalWrite(DG,0b00000001 & segments ? LOW : HIGH);
  
  digitalWrite(D1,displayDigit == 0 ? HIGH : LOW);
  digitalWrite(D2,displayDigit == 1 ? HIGH : LOW);
  digitalWrite(D3,displayDigit == 2 ? HIGH : LOW);
}
