#include <Adafruit_NeoPixel.h>
#include <Encoder.h>
#include <SPI.h>
#include <Adafruit_MAX31855.h>
#include <string.h>
#include <Time.h>
#include <EEPROM.h>
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
  
  loadData();
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

byte buttonDown(int b) {
  return !digitalRead(b);
}

byte displayMenu(char * options[], int len, int defaultChoice) {
  int menuIndex = -1;
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

    delay(100);
  }
  display.clear();
  return menuIndex;
}

int chooseNum(int low, int high, int defaultVal) {
  int val = defaultVal;
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
    delay(100);
  }
}

char* menu[] = {"st-soak temp","sd-soak duration","pt-peak temp","go"};
int dsize = 3;
byte data[3] = {0,0,0};
byte mainMenu() {
  int choice = 0;
  while(1) {
    choice = displayMenu(menu,4,choice);
    if (choice == -1) return 0; //re-enter menu
    if (choice == 3) return -1;
    data[choice] = chooseNum(0,255,data[choice]);
    EEPROM.write(choice,data[choice]);
  }
}

void ovenOn() {
//    digitalWrite(RELAY,HIGH);
    strip.setPixelColor(0, strip.Color(10,10,10));
    strip.show();
}

void ovenOff() {
//    digitalWrite(RELAY,LOW);
    strip.setPixelColor(0, strip.Color(0,0,0));
    strip.show();
}

double readThermocouple() {
  return getEnc(); //for testing purposes
}

#define DEFAULT_SOAK_TEMP 100
#define DEFAULT_SOAK_TIME 90
#define DEFAULT_PEAK_TEMP 190
void loadData() {
  for (byte i=0; i<3; i++) {
    data[i] = EEPROM.read(i);
  }
  if (data[0] == 255) data[0] = DEFAULT_SOAK_TEMP;
  if (data[1] == 255) data[1] = DEFAULT_SOAK_TIME;
  if (data[2] == 255) data[2] = DEFAULT_PEAK_TEMP;
}

void loop() {  
  while(!mainMenu());
  
  if (doReflow() == 0) {  
    display.displayMarquee("done");
  } else {
    display.displayMarquee("cancelled");    
  }
  delay(5000);
}

#define PHASE_PRE_SOAK 0
#define PHASE_SOAK 1
#define PHASE_SPIKE 2
#define PHASE_COOL 3

#define CANCEL_TIME 5000

byte doReflow() {
  unsigned long startTime = millis();
  unsigned long phaseStartTime = millis();
  unsigned long buttonStartTime = 0;
  int phase = PHASE_PRE_SOAK;
  
  byte soakTemp = data[0];
  byte soakTime = data[1];
  byte peakTemp = data[2];
  setEnc(25);
  
  ovenOn();
  while(1) {
    double temp = readThermocouple();
    if (buttonStartTime == 0) {
      display.display((int)temp);
      if (buttonDown(BACK_BUTTON)) {
        display.displayMarquee("Hold to cancel");
        buttonStartTime = millis();
      }
    } else {
      if ((millis() - buttonStartTime) > CANCEL_TIME) {
        ovenOff();
        return -1;
      }
      if (!buttonDown(BACK_BUTTON)) buttonStartTime = 0;
    }
    switch(phase) {
      case PHASE_PRE_SOAK: {
        if (temp >= soakTemp) {
          phase = PHASE_SOAK;
          phaseStartTime = millis();
          ovenOff();
        }
        break;
      }
      
      case PHASE_SOAK: {
        unsigned long currentSoakSeconds = (millis() - phaseStartTime) / 1000;
        if (currentSoakSeconds > soakTime) {
          phase = PHASE_SPIKE;
          phaseStartTime = millis();
          ovenOn();
        }
        break;
      }
      
      case PHASE_SPIKE: {
        if (temp >= peakTemp) {
          phase = PHASE_COOL;
          phaseStartTime = millis();
          ovenOff();
        }
        break;
      }
      
      case PHASE_COOL: {
        unsigned long currentCoolSeconds = (millis() - phaseStartTime) / 1000;
        if (currentCoolSeconds > 30 || temp < 60 || debounceButton(BACK_BUTTON) || debounceButton(ENC_BUTTON)) {
          ovenOff();
          return 0;
        }
        break;
      }
    }
  }
}
