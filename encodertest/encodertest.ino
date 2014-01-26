#include <Adafruit_NeoPixel.h>
#include <Encoder.h>
#include <SPI.h>
#include <Adafruit_MAX31855.h>
#include <string.h>

#define WSLED 4

int DDP = A4;
int DA = 5;
int DB = 13;
int DD = A0;
int DE = A1;
int DF = A2;
int DG = A3;

int D3 = 8;
int D2 = 9;
int D1 = 10;
int RELAY = 12;
int ENC_BUTTON = 3;
int BACK_BUTTON = 1;
int ENC_A = 2;
int ENC_B = 0;
int TC_CS = 11;

byte numerals[] = {0b01111110,0b00110000,0b01101101,0b01111001,0b00110011,0b01011011,0b00011111,0b01110000,0b01111111,0b01110011};
byte alphabet[] = {
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
0b01101101}; //z

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, WSLED, NEO_GRB + NEO_KHZ800);
Encoder myEnc(ENC_A, ENC_B);
Adafruit_MAX31855 thermocouple(TC_CS);

void setup() {
  strip.begin();
  strip.show();
  
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

int interruptCounter = 0;
ISR(TIMER1_OVF_vect) {
  TCNT1 = 65500;
  
  doDisplay();
  if (interruptCounter % 500 == 0) marqueeHandler();
  
  interruptCounter++;
}

volatile int displayDigit = 0;
char displayed[] = {0,0,0};
void doDisplay() {
  digitalWrite(D1,LOW);
  digitalWrite(D2,LOW);
  digitalWrite(D3,LOW);
  
  char val = displayed[displayDigit];
  digitalWrite(DDP,0b10000000 & val ? LOW : HIGH);
  digitalWrite(DA,0b01000000 & val ? LOW : HIGH);
  digitalWrite(DB,0b00100000 & val ? LOW : HIGH);
  //temporary hack while this pin is PE
  //digitalWrite(DC,0b00010000 & val ? LOW : HIGH);
  PORTE = (0b00010000 & val) ? (PORTE & (~0b00000100)) : (PORTE | 0b00000100);
  //end temporary hack
  digitalWrite(DD,0b00001000 & val ? LOW : HIGH);
  digitalWrite(DE,0b00000100 & val ? LOW : HIGH);
  digitalWrite(DF,0b00000010 & val ? LOW : HIGH);
  digitalWrite(DG,0b00000001 & val ? LOW : HIGH);
  
  digitalWrite(D1,displayDigit == 0 ? HIGH : LOW);
  digitalWrite(D2,displayDigit == 1 ? HIGH : LOW);
  digitalWrite(D3,displayDigit == 2 ? HIGH : LOW);
  
  if (displayDigit++ > 2) displayDigit = 0;
}

void displayValue(int n) {
  char neg = 0;
  if (n < 0) {
    n = n*-1;
    neg = 1;
  }
  displayed[2] = numerals[n % 10] | (neg?0b10000000:0);
  displayed[1] = n < 10 ? 0 : numerals[(n/10) % 10];
  displayed[0] = n < 100 ? 0 : numerals[(n/100) % 10];
}

byte getLetter(char a) {
  if (a == '-') return 0b00000001;
  if (a>=65 && a<=90) return alphabet[a-65];
  if (a>=97 && a<=122) return alphabet[a-97];
  return 0;
}

void displayChars(char * chars, int len) {
  displayed[0] = getLetter(chars[0]);
  displayed[1] = len > 1 ? getLetter(chars[1]) : 0;
  displayed[2] = len > 2 ? getLetter(chars[2]) : 0;
}


char * marqueeString = "\0";
int marqueeLength = 0;
int marqueeIndex = 0;
int MARQUEE_START_WAIT = 4;
int MARQUEE_END_WAIT = 1;
void marqueeHandler() {
  if (marqueeLength == 0) return;
  
  if (marqueeIndex >= 0 && marqueeIndex <= marqueeLength-3) {
    displayChars(marqueeString+marqueeIndex,3);
  }
  
  marqueeIndex++;
  if (marqueeIndex >= marqueeLength-3+MARQUEE_END_WAIT) marqueeIndex = -MARQUEE_START_WAIT;
}

void marqueeText(char * chars) {
  marqueeIndex = -MARQUEE_START_WAIT;
  marqueeString = chars;
  marqueeLength = strlen(chars);
  displayChars(marqueeString,3);
}

void stopMarquee() {
  marqueeLength = 0;
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
      stopMarquee();
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
      marqueeText(options[menuIndex]);
    }
    
    strip.setPixelColor(0, strip.Color(hb,hb,hb));
    hb = (hb+1) % 2;
    strip.show();
    delay(100);
  }
  stopMarquee();
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
    
    displayValue(val);
    
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
//  marqueeText("hello world");
//  while(1);

  while(!mainMenu());
//  displayed[0] = LED_S;
//  displayed[1] = LED_T;
//  displayed[2] = LED_R;
  //do oven here
}

void colorPicker() {
  int index = 0;
  int oldPosition;
  if (!digitalRead(BACK_BUTTON)) {
    index++;
    if (index >= dsize) index=0;
    
    //flash the current color
    strip.setPixelColor(0, strip.Color(index==0?100:0,index==1?100:0,index==2?100:0));
    strip.show();
    delay(200);
    
    myEnc.write(data[index]*4);
    while(!digitalRead(BACK_BUTTON));
  }
  
//  double temp = thermocouple.readCelsius();
//  int tempi = static_cast<int>(temp);
//  displayValue(tempi);

  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    int c = newPosition >> 2;
    displayValue(c);
    data[index] = c;
    strip.setPixelColor(0, strip.Color(data[0],data[1],data[2]));
    strip.show();
  } 
}
