#include "Reflowster.h"

int pinConfiguration_statusLed;
int pinConfiguration_relay;
int pinConfiguration_encoderButton;
int pinConfiguration_encoderA;
int pinConfiguration_encoderB;
int pinConfiguration_backButton;
int pinConfiguration_thermocoupleCS;
int pinConfiguration_displayDS;
int pinConfiguration_displaySTCP;
int pinConfiguration_displaySHCP;
int pinConfiguration_displayD1;
int pinConfiguration_displayD2;
int pinConfiguration_displayD3; //todo add display separator
int pinConfiguration_displayDD;
int pinConfiguration_beep;

Reflowster::Reflowster() {
  pinConfiguration_statusLed = 12;
  pinConfiguration_relay = 4;
  pinConfiguration_encoderButton = 3;
  pinConfiguration_encoderA = 2;
  pinConfiguration_encoderB = 0;
  pinConfiguration_backButton = 7;
  pinConfiguration_thermocoupleCS = 1;
  pinConfiguration_displayDS = 5;
  pinConfiguration_displaySTCP = 13;
  pinConfiguration_displaySHCP = A0;
  pinConfiguration_displayD1 = 6;
  pinConfiguration_displayD2 = 9;
  pinConfiguration_displayD3 = 10;
  pinConfiguration_displayDD = 8;  
  pinConfiguration_beep = A5;
}

void Reflowster::init() {
  status = new Adafruit_NeoPixel(1, pinConfiguration_statusLed, NEO_GRB + NEO_KHZ800);
  knob = new Encoder(pinConfiguration_encoderA, pinConfiguration_encoderB);
//  probe = new MAX31855(pinConfiguration_thermocoupleCS);

  display = new ReflowDisplay(pinConfiguration_displayDS,pinConfiguration_displaySTCP,pinConfiguration_displaySHCP,pinConfiguration_displayD1,pinConfiguration_displayD2,pinConfiguration_displayD3);
  
  status->begin();
  status->show();
  
  pinMode(pinConfiguration_thermocoupleCS, OUTPUT);
  pinMode(pinConfiguration_relay, OUTPUT);  
  pinMode(pinConfiguration_backButton, INPUT_PULLUP);
  pinMode(pinConfiguration_encoderButton, INPUT_PULLUP);
  pinMode(pinConfiguration_encoderA, INPUT_PULLUP);
  pinMode(pinConfiguration_encoderB, INPUT_PULLUP);
  pinMode(pinConfiguration_displayDS, OUTPUT);
  pinMode(pinConfiguration_displaySTCP, OUTPUT);
  pinMode(pinConfiguration_displaySHCP, OUTPUT);
  pinMode(pinConfiguration_beep, OUTPUT);
}

void Reflowster::selfTest() {
  display->displayMarquee("Testing");
  
  tone(pinConfiguration_beep,200);
  delay(150);
  tone(pinConfiguration_beep,230);
  delay(150);
  noTone(pinConfiguration_beep);
  
  setStatusColor(50,0,0);
  delay(200);
  setStatusColor(0,50,0);
  delay(200);
  setStatusColor(0,0,50);
  delay(200);
  setStatusColor(0,0,0);
  
  relayOn();
  delay(500);
  relayOff();
  
  display->display("enc");
  while(!getButton());
  
  display->display("bck");
  while(!getBackButton());
  
  int pknob = 5;
  setKnobPosition(pknob);
  boolean up = false;
  boolean down = false;
  while(!up || !down) {
    if (getKnobPosition() > pknob) up = true;
    if (getKnobPosition() < pknob) down = true;
    pknob = getKnobPosition();
    display->display(pknob);
  }
  
  //TODO test thermocouple
  
  tone(pinConfiguration_beep,200);
  delay(150);
  tone(pinConfiguration_beep,230);
  delay(150);
  noTone(pinConfiguration_beep);
  
  display->clear();
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
    delay(50);
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
//  return probe->extTemp(0);
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

boolean Reflowster::relayStatus() {
  return digitalRead(pinConfiguration_relay);
}
