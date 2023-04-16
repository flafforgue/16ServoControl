// -------------------------------------------------------------
//
// -------------------------------------------------------------

#define OSCFREQUENCY 28729000
// #define OSCFREQUENCY // clone    25758000
// #define OSCFREQUENCY // adafruit 28729000

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define UPDATEFREQUENCY 50

#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -------------------------------------------------------------

#define SCL       A5
#define SDA       A4

#define ROT_A      2
#define ROT_B      3
#define BTNEncoder 4

#define BTNLeft   11
#define BTNRight   9
#define BTNSel    12

#define S0         5
#define S1         6
#define S2         7
#define S3         8

#define SIGNAL    A3

// -------------------------------------------------------------

#define OLedWidth     128
#define OLedHight      64
#define OLedReset      -1
#define OLedAdr      0x3C

Adafruit_SSD1306 OLed(OLedWidth, OLedHight, &Wire, OLedReset);

// -------------------------------------------------------------
//  Lectures Valeurs 74HC4067
// -------------------------------------------------------------

int Values[16];

//                          v v v    v v v    v v v      v v  
//const byte ch0[16]  = { 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0 };
//const byte ch1[16]  = { 1,1,0,0, 1,1,0,0, 1,1,0,0, 1,1,0,0 };
//const byte ch2[16]  = { 0,0,0,0, 1,1,1,1, 0,0,0,0, 1,1,1,1 };
//const byte ch3[16]  = { 0,0,0,0, 0,0,0,0, 1,1,1,1, 1,1,1,1 };

const byte ch0[16]  = { 0,0,0,0, 1,1,1,1, 0,0,0,0, 1,1,1,1 };
const byte ch1[16]  = { 0,0,0,0, 0,0,0,0, 1,1,1,1, 1,1,1,1 };
const byte ch2[16]  = { 0,1,0,1, 0,1,0,1, 0,1,0,1, 0,1,0,1 };
const byte ch3[16]  = { 0,0,1,1, 0,0,1,1, 0,0,1,1, 0,0,1,1 };

int ReadChannel(int chn){            // Read One Channel
  digitalWrite(S0, ch0[chn] );
  digitalWrite(S1, ch1[chn] );
  digitalWrite(S2, ch2[chn] );
  digitalWrite(S3, ch3[chn] );
  delay(10);
  int val = analogRead(SIGNAL);
  return val;
}

void ReadChannels(){                // Read all Channels
  for(int i=0;i<16;i++) {
    Values[i]= ReadChannel(i);
  }
}

// -------------------------------------------------------------
//  Set Servo values
// -------------------------------------------------------------

#define MIN_US   800  
#define MAX_US  2200

int MinUs[16];
int MaxUs[16];

void SetServo(int num, int pos) {
  int microsec = map(pos, 0, 1023, MinUs[num],  MaxUs[num]);
  pwm.writeMicroseconds(num, microsec); 
}

void UpdateServos() {
  for(int i=0;i<16;i++) {
    SetServo(i, Values[i] );
  }  
}

// -------------------------------------------------------------
// Rotary Encoder
// -------------------------------------------------------------

int  encodermov = 0;

void doEncoder() {
  if (digitalRead(ROT_A) == digitalRead(ROT_B)) {
    encodermov = -1;
  } else {
    encodermov = +1;
  }
}

// -------------------------------------------------------------
// Boutons 
// -------------------------------------------------------------

#define BTN_NONE          0
#define BTN_ENC           1
#define BTN_ENC_Long      2
#define BTN_RIGHT         3
#define BTN_RIGHT_Long    4
#define BTN_LEFT          5
#define BTN_LEFT_Long     6
#define BTN_SELECT        7
#define BTN_SELECT_Long   8

#define BTN_LONGDELAY   800

byte keydown  = BTN_NONE;
byte key      = BTN_NONE;
unsigned long   BTNTime;

void ReadBtnState() {
  unsigned long NTime; 
  NTime=millis();
  
  if ( keydown == BTN_NONE ) { // no key waiting 
    if ( digitalRead (BTNEncoder )==LOW ) { BTNTime=NTime;  keydown=BTN_ENC;    }
    if ( digitalRead (BTNRight   )==LOW ) { BTNTime=NTime;  keydown=BTN_RIGHT;  }   
    if ( digitalRead (BTNLeft    )==LOW ) { BTNTime=NTime;  keydown=BTN_LEFT;   }
    if ( digitalRead (BTNSel     )==LOW ) { BTNTime=NTime;  keydown=BTN_SELECT; }    
  } else {                        // key allready down
    if ( NTime - BTNTime > 10 ) { // avoid rebounds
        switch (keydown) {
           case BTN_ENC:
                 if ( digitalRead (BTNEncoder)==HIGH ) { // keypress on release ;)   
                   key     = BTN_ENC;
                   keydown = BTN_NONE;
                 }
                 break;
           case BTN_RIGHT:
                 if ( digitalRead (BTNRight)==HIGH ) { // keypress on release ;)   
                   key     = BTN_RIGHT;
                   keydown = BTN_NONE;
                 } 
                 break;           
           case BTN_LEFT:
                 if ( digitalRead (BTNLeft)==HIGH ) { // keypress on release ;)   
                   key     = BTN_LEFT;
                   keydown = BTN_NONE;
                 } 
                 break;
           case BTN_SELECT:
                 if ( digitalRead (BTNSel)==HIGH ) { // keypress on release ;)   
                   key     = BTN_SELECT;
                   keydown = BTN_NONE;
                 } 
                 break;          
       }       
    }
  }
}

bool keypressed() {
  return ( key != BTN_NONE );
}

byte readkey() {
  byte tmp = key;
  key=BTN_NONE;
  return( tmp);
}

// -------------------------------------------------------------
// Sauvegarde et restauration eeprom
// -------------------------------------------------------------

const int ProgStamp = 3219;

#define Adr_Stamp  0
#define Adr_MinUs  2
#define Adr_MaxUs 18

void InitVars() {
  for ( int i=0; i<16 ; i++ ) {
    MinUs[i]=MIN_US;
    MaxUs[i]=MAX_US;   
  }
}

void SaveToEprom() {
  EEPROM.put(Adr_Stamp, ProgStamp);
  EEPROM.put(Adr_MinUs, MinUs);
  EEPROM.put(Adr_MaxUs, MaxUs);   
}

void InitFromEprom () {
  int temp;
  EEPROM.get(Adr_Stamp, temp);
  if ( temp == ProgStamp ) {  // Stamp found , we can assume eerom contain Valid Datas 
    EEPROM.get(Adr_MinUs, MinUs);
    EEPROM.get(Adr_MaxUs, MaxUs);
  } else {                    // Stamp not found , Values need to be initialized
    InitVars();
    SaveToEprom(); 
  }
}

// -------------------------------------------------------------
//  Mode Live
// -------------------------------------------------------------

void DoLive() {
  Serial.println(F("Enter Live Mode"));
  OLed.clearDisplay();
  OLed.setTextSize(2);
  OLed.setTextColor(SSD1306_WHITE);
  OLed.setCursor(0, 10);
  OLed.println(F("Live Mode"));
  OLed.display();
  
  boolean LetRunning = true;
  unsigned long omillis = 0;
  
  while ( LetRunning ) {
    ReadChannels();
    UpdateServos();
    ReadBtnState();
    if ( readkey() == BTN_SELECT ) { // if Select Pressed exit
      LetRunning=false;
    }

    if ( millis() - omillis > 1000 ) {
      omillis=millis();
      Serial.print("Value : ");
      for(int i = 0; i < 16; i ++){
        Serial.print(Values[i]); 
        Serial.print(" , ");
      }
      Serial.println(); 
    }
  }

  cli();
  encodermov=0;
  sei();
  Serial.println(F("Leave Live Mode"));
}

// -------------------------------------------------------------
//
// -------------------------------------------------------------

void setup(){
  Serial.begin(115200);
    
  pinMode(ROT_A      , INPUT_PULLUP);
  pinMode(ROT_B      , INPUT_PULLUP);
  pinMode(BTNEncoder , INPUT_PULLUP);
  pinMode(BTNLeft    , INPUT_PULLUP);
  pinMode(BTNRight   , INPUT_PULLUP);
  pinMode(BTNSel     , INPUT_PULLUP); 
  pinMode(S0         , OUTPUT); 
  pinMode(S1         , OUTPUT); 
  pinMode(S2         , OUTPUT); 
  pinMode(S3         , OUTPUT); 
  
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);

  pwm.begin();
  pwm.setOscillatorFrequency(OSCFREQUENCY);
  pwm.setPWMFreq(UPDATEFREQUENCY);  
  InitVars();  
  Serial.println(F("PCA9685 Initialized"));
  
  if(!OLed.begin(SSD1306_SWITCHCAPVCC, OLedAdr)) {
    Serial.println(F("SSD1306 Error"));
  } else {
    Serial.println(F("SSD1306 Initialized"));
  }
//  OLed.clearDisplay();
  OLed.display();  // display adafruit logo

  attachInterrupt(digitalPinToInterrupt(ROT_A), doEncoder, CHANGE);  
  Serial.println(F("Running ..."));
  delay(2000);
}


// -------------------------------------------------------------

int menu  = 0;
int omenu = 1;

void loop(){
  ReadBtnState(); 

  if (encodermov != 0 ) {
    cli();
    menu+=encodermov;
    encodermov=0;
    sei();
    if ( menu > 4 ) { menu=0; }
    if ( menu < 0 ) { menu=4; }
  }

  if ( menu != omenu ) {
    OLed.clearDisplay();
    OLed.setTextSize(2);
    OLed.fillRect(0, menu*17, 127, 16, SSD1306_WHITE);
    OLed.setTextColor(SSD1306_INVERSE);
    OLed.setCursor(4,  0);  OLed.println(F("Live Mode"));
//    OLed.setTextColor(SSD1306_WHITE);
    OLed.setCursor(4, 16);  OLed.println(F("Play"));
    OLed.setCursor(4, 33);  OLed.println(F("Record"));
    OLed.setCursor(4, 50);  OLed.println(F("Setup"));
    OLed.display();
    omenu=menu;
  }
  if ( keypressed() ) {
    if ( readkey() == BTN_ENC ) {
      switch (menu ) {
        case 0: DoLive();
                break;
        case 1:  
                break;
        case 2:  
                break;
        case 3:  
                break;                                       
      }
    }
    omenu=menu+1;  // to force redraw 
  }
}
