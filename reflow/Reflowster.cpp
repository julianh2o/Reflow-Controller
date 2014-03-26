#include "Reflowster.h"

int pinConfiguration_statusLed = 4;
int pinConfiguration_relay = 12;
int pinConfiguration_encoderButton = 3;
int pinConfiguration_encoderA = 2;
int pinConfiguration_encoderB = 0;
int pinConfiguration_backButton = 1;
int pinConfiguration_thermocoupleCS = 11;

Reflowster::Reflowster() {
  pinConfiguration_statusLed = 4;
  pinConfiguration_relay = 12;
  pinConfiguration_encoderButton = 3;
  pinConfiguration_encoderA = 2;
  pinConfiguration_encoderB = 0;
  pinConfiguration_backButton = 1;
  pinConfiguration_thermocoupleCS = 11;
}

void Reflowster::init() {
  status = new Adafruit_NeoPixel(1, pinConfiguration_statusLed, NEO_GRB + NEO_KHZ800);
  knob = new Encoder(pinConfiguration_encoderA, pinConfiguration_encoderB);
  probe = new Adafruit_MAX31855(pinConfiguration_thermocoupleCS);
  display = new ReflowDisplay();
  
  status->begin();
  status->show();
  
  pinMode(pinConfiguration_thermocoupleCS, OUTPUT);
  pinMode(pinConfiguration_relay, OUTPUT);  
  pinMode(pinConfiguration_backButton, INPUT_PULLUP);
  pinMode(pinConfiguration_encoderButton, INPUT_PULLUP);
  pinMode(pinConfiguration_encoderA, INPUT_PULLUP);
  pinMode(pinConfiguration_encoderB, INPUT_PULLUP);
}

void Reflowster::tick() {
  display->tick();
}

// Status
/////////
void Reflowster::setStatusColor(byte r, byte g, byte b) {
  status->setPixelColor(0, status->Color(r,g,b));
  status->show(); 
}

// Display
//////////
ReflowDisplay * Reflowster::getDisplay() {
  return display;
}

void Reflowster::displayTest() {
  int i,l;
  byte testSegments[8] = {0b00000010,0b01000000,0b00100000,0b00010000,0b00001000,0b00000100,0b00000001,0b10000000};
  for (l=0; l<8; l++) {
    for (i=0; i<3; i++) {
      display->setSegment(testSegments[l],i);
    }
    delay(100);
  }
}

// Buttons
//////////
boolean Reflowster::getBackButton() {
  return !digitalRead(pinConfiguration_backButton);
}

boolean Reflowster::getButton() {
  return !digitalRead(pinConfiguration_encoderButton);
}

// Encoder
//////////
void Reflowster::setKnobPosition(int val) {
    knob->write(val << 2); 
}

int Reflowster::getKnobPosition() {
    long encPosition = knob->read();
    int enc = (int)(encPosition >> 2);
    return enc;
}

// Thermocouple
///////////////
double Reflowster::readThermocouple() {
  return probe->readCelsius();
}

// Relay
////////
void Reflowster::relayOn() {
  digitalWrite(pinConfiguration_relay,HIGH);
  Serial.println("Relay ON");
}

void Reflowster::relayOff() {
  digitalWrite(pinConfiguration_relay,LOW);
  Serial.println("Relay OFF");
}

void Reflowster::relayToggle() {
  if (digitalRead(pinConfiguration_relay)) {
    relayOff();
  } else {
    relayOn();    
  }
}
