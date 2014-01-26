#include <Adafruit_NeoPixel.h>
#include <Encoder.h>
#include <SPI.h>
#include <Adafruit_MAX31855.h>

#define WSLED 4

//pb5 = digital pin 9 = d2
//pb6 = digital pin 10 = d1
//PC6 = digital pin 5 = DA
//int LED1 = 5;
int DDP = A4;
int DA = 5;
int DB = 13;
//DC = PE2 = HWB
int DD = A0;
int DE = A1;
int DF = A2;
int DG = A3;
//DD = PF7 = Analog In 0
//DE = PF6 = analog in 1
//DF = PF5 = analog in 2

int D3 = 8;
int D2 = 9;
int D1 = 10;
int RELAY = 12;
int ENC_BUTTON = 3;
int BACK_BUTTON = 1;
int ENC_A = 2;
int ENC_B = 0;
int TC_CS = 11;

int numerals[] = {0b01111110,0b00110000,0b01101101,0b01111001,0b00110011,0b01011011,0b00011111,0b01110000,0b01111111,0b01110011};

// the setup routine runs once when you press reset:
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, WSLED, NEO_GRB + NEO_KHZ800);
Encoder myEnc(ENC_A, ENC_B);
Adafruit_MAX31855 thermocouple(TC_CS);


void setup() {
  strip.begin();
  strip.show();
  
  // initialize the digital pin as an output.
  pinMode(DDP, OUTPUT);
  pinMode(DA, OUTPUT);
  pinMode(DB, OUTPUT);
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
  DDRE = DDRE | 0b00000100;
  
  
//  pinMode(ENC_A, INPUT_PULLUP);
//  pinMode(ENC_B, INPUT_PULLUP);
  //digitalWrite(RELAY,HIGH);
//  

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = 34286;            // preload timer 65536-16MHz/256/2Hz
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();
}

ISR(TIMER1_OVF_vect) {
  TCNT1 = 65500;            // preload timer
  doDisplay();
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



int g = 0;
int b = 0;
int i=0;
long oldPosition  = -999;
void loop() {
  if (!digitalRead(ENC_BUTTON)) {
    myEnc.write(0);
  }
  
  double temp = thermocouple.readCelsius();
  int tempi = static_cast<int>(temp);
  displayValue(tempi);

  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    int c = newPosition >> 2;
    strip.setPixelColor(0, strip.Color(c,c,c));
    //displayValue(c);
    strip.show();
  }
//  g = newPosition;
//  if (digitalRead(ENC_BUTTON) == LOW) {
//    g = 100;
//  }
  
//  if (!digitalRead(BACK_BUTTON)) {
//    b = 100;
//  }
  

//  strip.setPixelColor(0, strip.Color(g,0,b));
//  g--;
//  b--;
  
//  if (g < 0) g = 0;
//  if (b < 0) b = 0;
  
//  if (newPosition > oldPosition) {
//    strip.setPixelColor(0, strip.Color(10,0,0));
//  }
//  if (newPosition < oldPosition) {
//    strip.setPixelColor(0, strip.Color(0,10,0));
//  }
//  
//  oldPosition = newPosition;
//  strip.setPixelColor(0, 0);



  
//  int buttonState = digitalRead(ENC_BUTTON);
//  if (buttonState == HIGH) {
//    strip.setPixelColor(0, strip.Color(10,0,0));
//  } else {
//    strip.setPixelColor(0, 0);
//  }
////  digitalWrite(D2, LOW);   // turn the LED on (HIGH is the voltage level)
////  digitalWrite(DA, HIGH);   // turn the LED on (HIGH is the voltage level)
////  delay(100);               // wait for a second
////  digitalWrite(DA, LOW);   // turn the LED on (HIGH is the voltage level)
//    strip.show();
//    delay(10);               // wait for a second
}
