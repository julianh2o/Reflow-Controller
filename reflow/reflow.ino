#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Encoder.h>
#include <Adafruit_MAX31855.h>

#include <string.h>
#include <Time.h>
#include <EEPROM.h>
#include "Reflowster.h"
#include "ReflowMenu.h"
#include "ReflowMenuItem.h"
#include "ReflowMenuEndpoint.h"

/*
go
set profile
	pb
	pb free
	custom
		soak temp
		soak duration
		peak temperature
		save
monitor
*/

Reflowster reflowster;

struct profile {
  byte soakTemp;
  byte soakTime;
  byte peakTemp;
};

profile leaded, unleaded, custom;
byte activeProfile;

const int CMD_REFLOW_START=1;
const int CMD_REFLOW_STOP=2;

const int MODE_MAIN_MENU=1;
const int MODE_REFLOW=2;

void setup() {
  Serial.begin(9600);

  reflowster.init();
  reflowster.displayTest();
  
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = 65500;            // preload timer 65536-16MHz/256/2Hz
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();
  
  loadProfiles();
}

ISR(TIMER1_OVF_vect) {
  TCNT1 = 65500;
  
  reflowster.tick();
}

byte debounceButton(int b) {
  if (!digitalRead(b)) {
    while(!digitalRead(b));
    delay(100);
    return 1;
  }
  return 0;
}

int processCommands(int mode) {
/*
  char buffer[30];
  char i = 0;
  while (Serial.available()) {
    buffer[i++] = Serial.read();
  }
  if (i != 0) {
    buffer[i] = 0;
    String command = String(buffer);
    int spaceAt = command.indexOf(" ");
    String arguments = command.substring(spaceAt+1);
    if (spaceAt == -1) arguments = "";
    Serial.write("> ");
    Serial.write(buffer);
    Serial.write("\n");
    if (mode == MODE_MAIN_MENU) {
      if (command.startsWith("relay ")) {
        if (arguments.equalsIgnoreCase("on")) {
          reflowster.relayOn();
        } else if (arguments.equalsIgnoreCase("off")) {
          reflowster.relayOff();        
        } else if (arguments.equalsIgnoreCase("toggle")) {
          reflowster.relayToggle();
        }
      } else if (command.startsWith("setst ")) {
        setData(0,(byte)arguments.toInt());
        Serial.print("Set soak temperature: ");
        Serial.println(data[0]);
        //TODO range checking
      } else if (command.startsWith("setsd ")) {
        setData(1,(byte)arguments.toInt());
        Serial.print("Set soak duration: ");
        Serial.println(data[1]);
      } else if (command.startsWith("setpt ")) {
        setData(2,(byte)arguments.toInt());
        Serial.print("Set peak temperature: ");
        Serial.println(data[2]);
      } else if (command.equalsIgnoreCase("start")) {
        return CMD_REFLOW_START;
      }
    } else if (mode == MODE_REFLOW) {
      if (command.equalsIgnoreCase("stop")) {
        return CMD_REFLOW_STOP;
      }
    }
    
    if (command.equalsIgnoreCase("status")) {
      if (mode == MODE_MAIN_MENU) {
        Serial.println("Mode: main menu");
      } else if (mode == MODE_REFLOW) {
        Serial.println("Mode: reflow");        
      }
      Serial.print("Current thermocouple reading (C): ");
      Serial.println(reflowster.readThermocouple());
      
      Serial.println();
      
      Serial.println("Configuration: ");
      Serial.print("Soak Temperature: ");
      Serial.println(data[0]);
      
      Serial.print("Soak Duration: ");
      Serial.println(data[1]);
      
      Serial.print("Peak Temperature: ");
      Serial.println(data[2]);
    }
  }
*/
}

byte displayMenu(char * options[], int len, int defaultChoice) {
  int menuIndex = -1;
  reflowster.setKnobPosition(defaultChoice);
  while(1) {
    int command = processCommands(MODE_MAIN_MENU);
    if (command == CMD_REFLOW_START) {
      Serial.println("Reflow started");
      return len-1; //start
    }
    
    if (debounceButton(reflowster.pinConfiguration_encoderButton)) {
      reflowster.getDisplay()->clear();
      return menuIndex;
    }
    if (debounceButton(reflowster.pinConfiguration_backButton)) {
      reflowster.getDisplay()->clear();
      return -1;
    }
    
    int newIndex = reflowster.getKnobPosition();
    
    if (newIndex >= len) {
      newIndex = len - 1;
      reflowster.setKnobPosition(newIndex);
    }
    
    if (newIndex < 0) {
      newIndex = 0;
      reflowster.setKnobPosition(newIndex);
    }
    
    if (newIndex != menuIndex) {
      menuIndex = newIndex;
      reflowster.getDisplay()->displayMarquee(options[menuIndex]);
    }

    delay(100);
  }
  reflowster.getDisplay()->clear();
  return menuIndex;
}

int chooseNum(int low, int high, int defaultVal) {
  int val = defaultVal;
  reflowster.setKnobPosition(val);
  while(1) {
    if (debounceButton(reflowster.pinConfiguration_encoderButton)) return val;
    if (debounceButton(reflowster.pinConfiguration_backButton)) return defaultVal;
    
    val = reflowster.getKnobPosition();
    if (val > high) {
      val = high;
      reflowster.setKnobPosition(val);
    }
    if (val < low) {
      val = low;
      reflowster.setKnobPosition(val);
    }
    
    reflowster.getDisplay()->display(val);
    delay(100);
  }
}

void loadProfiles() {
  leaded.soakTemp = 100;
  leaded.soakTime = 90;
  leaded.peakTemp = 190;
  
  unleaded.soakTemp = 100;
  unleaded.soakTime = 90;
  unleaded.peakTemp = 190;

  custom.soakTemp = leaded.soakTemp;
  custom.soakTime = leaded.soakTime;
  custom.peakTemp = custom.peakTemp;
  
  activeProfile = EEPROM.read(0); //read the selected profile
  if (activeProfile > 2) activeProfile = 0;

  loadProfile(1,&custom); //use the leaded profile as the default

  //Serial.println("custom");
  //Serial.println(custom.soakTemp);
  //Serial.println(custom.soakTime);
  //Serial.println(custom.peakTemp);
}

void loadProfile(byte loc, struct profile * target) {
  for (byte i=0; i<3; i++) {
    byte val = EEPROM.read(loc+i);
    if (val != 255) *(((byte*)target)+i) = val; //pointer-fu to populate the profile struct
  }
}

void setActiveProfile(byte p) {
  activeProfile = p;
  if (EEPROM.read(0) != activeProfile) ewrite(0,activeProfile);
}

void saveProfile(byte loc, struct profile * target) {
  for (byte i=0; i<3; i++) {
    byte val = EEPROM.read(loc+i);
    if (val != *(((byte*)target)+i)) ewrite(loc+i,*(((byte*)target)+i)); //we only write to eeprom if the value is changed
  }
}

void ewrite(byte loc, byte val) {
  /*
  Serial.print("EEPROM WRITE ");
  Serial.print(val);
  Serial.print(" to ");
  Serial.println(loc);
  */
  EEPROM.write(loc,val);
}

void loop() { 
  mainMenu();
}

char * mainMenuItems[] = {"go","set profile","monitor"};
void mainMenu() {
  int choice = 0;
  while(1) {
    choice = displayMenu(mainMenuItems,3,choice);
    switch(choice) {
      case 0: doReflow(); break;

      case 1: 
        if (setProfile()) choice = 0;
      break;

      case 2: doMonitor(); break;
    }
  }
}

void doReflow() {
  struct profile * active = &leaded;
  if (activeProfile == 1) active = &unleaded;
  if (activeProfile == 2) active = &custom;

  byte soakTemp = active->soakTemp;
  byte soakTime = active->soakTime;
  byte peakTemp = active->peakTemp;

  if (reflowImpl(soakTemp,soakTime,peakTemp) == 0) {  
    reflowster.getDisplay()->displayMarquee("done");
  } else {
    reflowster.getDisplay()->displayMarquee("cancelled");    
  }
  delay(3500);
}

char * profileMenuItems[] = {"+pb leaded","-pb unleaded","custom"};
boolean setProfile() {
  int choice = 0;
  choice = displayMenu(profileMenuItems,3,choice);
  switch(choice) {
    case -1: return false;
    case 0:
      setActiveProfile(choice);
      return true;
    break;

    case 1:
      setActiveProfile(choice);
      return true;
    break;

    case 2:
      if (editCustomProfile()) {
        setActiveProfile(choice);
        return true;
      }
    break;
  }
}

char * editProfileMenuItems[] = {"st-soak temp","sd-soak duration","pt-peak temp","set"};
boolean editCustomProfile() {
  int choice = 0;
  byte val;
  while(1) {
    choice = displayMenu(editProfileMenuItems,4,choice);
    switch(choice) {
      case -1: return false;
      case 0:
      case 1:
      case 2:
        val = *(((byte*)&custom)+choice);
        *(((byte*)&custom)+choice) = chooseNum(0,255,val);

        saveProfile(1,&custom);
      break;

      case 3:
        return true;
    }
  }
}

void doMonitor() {
  unsigned long lastReport = millis();
  while(1) {
    
    double temp = reflowster.readThermocouple();
    reflowster.getDisplay()->display((int)temp);

    if ((millis() - lastReport) > 1000) {  //generate a 1000ms event period
      Serial.println(temp);
      lastReport += 100;
    }
    
    if (debounceButton(reflowster.pinConfiguration_encoderButton)) return;
    if (debounceButton(reflowster.pinConfiguration_backButton)) return;

    delay(10);
  } 
}

#define PHASE_PRE_SOAK 0
#define PHASE_SOAK 1
#define PHASE_SPIKE 2
#define PHASE_COOL 3
#define CANCEL_TIME 5000
byte reflowImpl(byte soakTemp, byte soakTime, byte peakTemp) {
  unsigned long startTime = millis();
  unsigned long phaseStartTime = millis();
  unsigned long buttonStartTime = 0;
  unsigned long lastReport = millis();
  int phase = PHASE_PRE_SOAK;

  Serial.println("Starting Reflow: ");
  Serial.print("Soak Temp: ");
  Serial.println(soakTemp);
  Serial.print("Soak Time: ");
  Serial.println(soakTime);
  Serial.print("Peak Temp: ");
  Serial.println(peakTemp);
  
  reflowster.setKnobPosition(25); //TODO remove me for production
  
  reflowster.relayOn();
  while(1) {
    double temp = reflowster.readThermocouple();
    
    if ((millis() - lastReport) > 1000) {  //generate a 1000ms event period
      Serial.println(temp);
      lastReport += 1000;
    }
    
    int command = processCommands(MODE_REFLOW);
    if (command == CMD_REFLOW_STOP) {
        Serial.println("Reflow cancelled");
        reflowster.relayOff();
        return -1;
    }
    
    if (buttonStartTime == 0) {
      reflowster.getDisplay()->display((int)temp);
      if (reflowster.getBackButton()) {
        reflowster.getDisplay()->displayMarquee("Hold to cancel");
        buttonStartTime = millis();
      }
    } else {
      if ((millis() - buttonStartTime) > CANCEL_TIME) {
        reflowster.relayOff();
        return -1;
      }
      if (!reflowster.getBackButton()) buttonStartTime = 0;
    }
    switch(phase) {
      case PHASE_PRE_SOAK: {
        if (temp >= soakTemp) {
          phase = PHASE_SOAK;
          phaseStartTime = millis();
          reflowster.relayOff();
        }
        break;
      }
      
      case PHASE_SOAK: {
        unsigned long currentSoakSeconds = (millis() - phaseStartTime) / 1000;
        if (currentSoakSeconds > soakTime) {
          phase = PHASE_SPIKE;
          phaseStartTime = millis();
          reflowster.relayOn();
        }
        break;
      }
      
      case PHASE_SPIKE: {
        if (temp >= peakTemp) {
          phase = PHASE_COOL;
          phaseStartTime = millis();
          reflowster.relayOff();
        }
        break;
      }
      
      case PHASE_COOL: {
        unsigned long currentCoolSeconds = (millis() - phaseStartTime) / 1000;
        if (currentCoolSeconds > 30 || temp < 60 || debounceButton(reflowster.pinConfiguration_backButton) || debounceButton(reflowster.pinConfiguration_encoderButton)) {
          reflowster.relayOff();
          return 0;
        }
        break;
      }
    }
  }
}
