#include <Adafruit_NeoPixel.h>
#include <Encoder.h>
#include <SPI.h>
#include <Adafruit_MAX31855.h>
#include <string.h>
#include "ReflowDisplay.h"

#define WSLED 4

int RELAY = 12;
int ENC_BUTTON = 3;
int BACK_BUTTON = 1;
int ENC_A = 2;
int ENC_B = 0;
int TC_CS = 11;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, WSLED, NEO_GRB + NEO_KHZ800);
Encoder myEnc(ENC_A, ENC_B);
Adafruit_MAX31855 thermocouple(TC_CS);
ReflowDisplay display;

void setup() {
  strip.begin();
  strip.show();
  
  pinMode(TC_CS, OUTPUT);
  pinMode(RELAY, OUTPUT);
  
  pinMode(BACK_BUTTON, INPUT_PULLUP);
  pinMode(ENC_BUTTON, INPUT_PULLUP);
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = 65500;            // preload timer 65536-16MHz/256/2Hz
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();
}


ISR(TIMER1_OVF_vect) {
  TCNT1 = 65500;
  
  display.tick();
}

void setEnc(int val) {
    myEnc.write(val << 2); 
}

int getEnc() {
    long encPosition = myEnc.read();
    int enc = (int)(encPosition >> 2);
    return enc;
}

byte debounceButton(int b) {
  if (!digitalRead(b)) {
    while(!digitalRead(b));
    return 1;
  }
  return 0;
}

byte displayMenu(char * options[], int len, int defaultChoice) {
  int menuIndex = -1;
  char hb = 0;
  setEnc(defaultChoice);
  while(1) {
    if (debounceButton(BACK_BUTTON)) {
      display.clear();
      return menuIndex;
    }
//    if (debounceButton(BACK_BUTTON)) {
//      stopMarquee();
//      return -1;
//    }
    
    int newIndex = getEnc();
    
    if (newIndex >= len) {
      newIndex = len - 1;
      setEnc(newIndex);
    }
    
    if (newIndex < 0) {
      newIndex = 0;
      setEnc(newIndex);
    }
    
    if (newIndex != menuIndex) {
      menuIndex = newIndex;
      display.displayMarquee(options[menuIndex]);
    }
    
    strip.setPixelColor(0, strip.Color(hb,hb,hb));
    hb = (hb+1) % 2;
    strip.show();
    delay(100);
  }
  display.clear();
  return menuIndex;
}

int chooseNum(int low, int high, int defaultVal) {
  int val = defaultVal;
  char hb = 0;
  setEnc(val);
  while(1) {
    if (debounceButton(ENC_BUTTON)) return val;
    if (debounceButton(BACK_BUTTON)) return val;
    
    val = getEnc();
    if (val > high) {
      val = high;
      setEnc(val);
    }
    if (val < low) {
      val = low;
      setEnc(val);
    }
    
    display.display(val);
    
    strip.setPixelColor(0, strip.Color(0,hb,hb));
    hb = (hb+1) % 2;
    strip.show();
    delay(100);
  }
}

char* menu[] = {"st-soak temp","sd-soak duration","pt-peak temp","go"};
int dsize = 3;
byte data[3];
byte mainMenu() {
  int choice = 0;
  while(1) {
    choice = displayMenu(menu,4,choice);
    if (choice == -1) return 0; //re-enter menu
    if (choice == 3) return -1;
    data[choice] = chooseNum(0,255,data[choice]);
  }
}

void loop() {
//  display.displayMarquee("hello world");
//  while(1);
  while(!mainMenu());
//  displayed[0] = LED_S;
//  displayed[1] = LED_T;
//  displayed[2] = LED_R;
  //do oven here
}
