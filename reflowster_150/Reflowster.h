#ifndef Reflowster_h
#define Reflowster_h

#include <arduino.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Encoder.h>
#include "ReflowDisplay.h"
#include <MAX31855.h>

class Reflowster {
public:
  int pinConfiguration_statusLed;
  int pinConfiguration_relay;
  int pinConfiguration_encoderButton;
  int pinConfiguration_encoderA;
  int pinConfiguration_encoderB;
  int pinConfiguration_backButton;
  int pinConfiguration_thermocoupleCS;
  
  Reflowster();
  void init();
  void tick();
  void setStatusColor(byte r, byte g, byte b);
  ReflowDisplay * getDisplay();
  void displayTest();
  boolean getBackButton();
  boolean getButton();
  void setKnobPosition(int val);
  int getKnobPosition();
  double readThermocouple();
  void relayOn();
  void relayOff();
  void relayToggle();
  boolean relayStatus();
  
private:
  Adafruit_NeoPixel * status;
  Encoder * knob;
//  MAX31855 * probe;
  ReflowDisplay * display;
};

#endif
