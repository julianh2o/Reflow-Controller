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

char* menu[] = {"monitor","st-soak temp","sd-soak duration","pt-peak temp","go"};
int menuSize = 5;
int dsize = 3;
byte data[3] = {0,0,0};

void setup() {
  Serial.begin(9600);
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
  
  hardwareTest();
  
  loadData();
}


ISR(TIMER1_OVF_vect) {
  TCNT1 = 65500;
  
  display.tick();
}

void hardwareTest() {
  int i,l;
  byte testSegments[8] = {0b00000010,0b01000000,0b00100000,0b00010000,0b00001000,0b00000100,0b00000001,0b10000000};
  for (l=0; l<8; l++) {
    for (i=0; i<3; i++) {
      display.setSegment(testSegments[l],i);
    }
    delay(100);
  }
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
    if (debounceButton(ENC_BUTTON)) {
      display.clear();
      return menuIndex;
    }
//    if (debounceButton(BACK_BUTTON)) {
//      display.clear();
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
    if (debounceButton(BACK_BUTTON)) return defaultVal;
    
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

byte editSetting(byte index) {
<<<<<<< HEAD
=======
  Serial.println("EEPROM Write");
  Serial.println(index);
  Serial.println(data[index]);
>>>>>>> b25adc26b9ca82c208d88877be4cff95bd6d3022
  data[index] = chooseNum(0,255,data[index]);
  EEPROM.write(index,data[index]);
}

void monitorTemp() {
  unsigned long lastReport = millis();
  while(1) {
    
    double temp = readThermocouple();
    display.display((int)temp);

    if ((millis() - lastReport) > 100) {  //generate a 100ms event period
      Serial.println(temp);
      lastReport += 100;
    }
    
    if (debounceButton(ENC_BUTTON)) return;
    if (debounceButton(BACK_BUTTON)) return;

    delay(10);
  } 
}

byte mainMenu() {
  int choice = 0;
  while(1) {
    choice = displayMenu(menu,menuSize,choice);
    if (choice == -1) return 0; //re-enter menu
    if (choice == menuSize-1) return -1;
    if (choice == 0) monitorTemp();
    if (choice >= 1 && choice <= 3) {
      editSetting(choice-1);
    }
  }
}

void ovenOn() {
  digitalWrite(RELAY,HIGH);
  Serial.println("Relay ON");
//  strip.setPixelColor(0, strip.Color(10,10,10));
//  strip.show();
}

void ovenOff() {
  digitalWrite(RELAY,LOW);
  Serial.println("Relay OFF");
//  strip.setPixelColor(0, strip.Color(0,0,0));
//  strip.show();
}

double readThermocouple() {
  return thermocouple.readCelsius();
//  return getEnc();
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
  delay(3500);
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
  unsigned long lastReport = millis();
  int phase = PHASE_PRE_SOAK;
  
  byte soakTemp = data[0];
  byte soakTime = data[1];
  byte peakTemp = data[2];
  setEnc(25); //TODO remove me for production
  
  ovenOn();
  while(1) {
    double temp = readThermocouple();
    
    if ((millis() - lastReport) > 1000) {  //generate a 1000ms event period
      Serial.println(temp);
      lastReport += 1000;
    }
    
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
