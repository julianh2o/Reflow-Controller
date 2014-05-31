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
int pinConfiguration_beep;

Reflowster::Reflowster() {
  pinConfiguration_statusLed = 12;
  pinConfiguration_relay = 4;
  pinConfiguration_encoderButton = 3;
  pinConfiguration_encoderA = 2;
  pinConfiguration_encoderB = 0;
  pinConfiguration_backButton = 1;
  pinConfiguration_thermocoupleCS = 7;
  pinConfiguration_displayDS = 5;
  pinConfiguration_displaySTCP = 13;
  pinConfiguration_displaySHCP = A0;
  pinConfiguration_beep = A5;
}

void Reflowster::init() {
  status = new Adafruit_NeoPixel(1, pinConfiguration_statusLed, NEO_GRB + NEO_KHZ800);
//  knob = new Encoder(pinConfiguration_encoderA, pinConfiguration_encoderB);
//  probe = new MAX31855(pinConfiguration_thermocoupleCS);
  //display = new ReflowDisplay();
  
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
  
  pinMode(6,OUTPUT);
  digitalWrite(6,HIGH);
  
  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);
  
  pinMode(10,OUTPUT);
  digitalWrite(10,HIGH);
  
  pinMode(8,OUTPUT); //separator
  digitalWrite(8,HIGH);
  
  //Serial.begin(9600);

//    while(!getBackButton()) {
//      delay(1000);
//      Serial.println("not back..");
//    }
//    delay(100);
//    while(getBackButton()) {
//      delay(1000);
//      Serial.println("back..");
//    }
//    Serial.println("button");
//    delay(100);
  tone(pinConfiguration_beep,200);
  delay(150);
  tone(pinConfiguration_beep,230);
  delay(150);
  noTone(pinConfiguration_beep);
  
  status->setPixelColor(0,status->Color(100,0,0));
  delay(200);
  status->setPixelColor(0,status->Color(0,100,0));
  delay(200);
  status->setPixelColor(0,status->Color(0,0,100));
  delay(200);
  status->setPixelColor(0,status->Color(0,0,0));
  
//  digitalWrite(pinConfiguration_displaySTCP,LOW);
//  digitalWrite(pinConfiguration_displayDS,HIGH);
//  digitalWrite(pinConfiguration_displaySHCP,LOW);
//  while(1);x
        
  while(1) {
    for (byte c=0; c<8; c++) {
      unsigned char b = 1 << c;
//      unsigned char b = 0xff;
      Serial.println((unsigned char)b);
      digitalWrite(pinConfiguration_displaySTCP, LOW);
      for (char i=0; i<8; i++) {
        digitalWrite(pinConfiguration_displaySHCP,LOW);
        delay(10);
        digitalWrite(pinConfiguration_displayDS,b&1);
        delay(10);
        Serial.print("  ");
        Serial.println(b&1);
        digitalWrite(pinConfiguration_displaySHCP,HIGH);
        delay(10);
        b = b >> 1;
      }
      digitalWrite(pinConfiguration_displaySTCP, HIGH);
      delay(500);
    }
    delay(100); 
  }
  
  //setKnobPosition(50);

  Encoder k(2,0);
  while(1) {
    status->setPixelColor(0,status->Color(0,getKnobPosition(),0));
    Serial.println(digitalRead(pinConfiguration_encoderA));
    Serial.println(digitalRead(pinConfiguration_encoderB));
    Serial.println();
    status->show();
    delay(200);
  }
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
