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

int  Values[16];
int  OValues[16];
byte LastChanelchanged = 0;

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
    int temp=ReadChannel(i);
    if ( abs( OValues[i] - temp ) > 5) {
       OValues[i]=Values[i];
       LastChanelchanged=i;
    }
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

void ClearEncoder() {
  cli();
  encodermov=0;
  sei(); 
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

#define BTN_LONGDELAY  1000

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
                   if ( NTime - BTNTime >= BTN_LONGDELAY )  key = BTN_ENC_Long;                
                   else                                     key = BTN_ENC;
                   keydown=BTN_NONE;
                 }
                 break;
           case BTN_RIGHT:
                 if ( digitalRead (BTNRight)==HIGH ) { // keypress on release ;)   
                   if ( NTime - BTNTime >= BTN_LONGDELAY )  key = BTN_RIGHT_Long;                
                   else                                     key = BTN_RIGHT;
                   keydown=BTN_NONE;
                 } 
                 break;           
           case BTN_LEFT:
                 if ( digitalRead (BTNLeft)==HIGH ) { // keypress on release ;)   
                   if ( NTime - BTNTime >= BTN_LONGDELAY )  key = BTN_LEFT_Long;                
                   else                                     key = BTN_LEFT;
                   keydown=BTN_NONE;
                 } 
                 break;
           case BTN_SELECT:
                 if ( digitalRead (BTNSel)==HIGH ) { // keypress on release ;)   
                   if ( NTime - BTNTime >= BTN_LONGDELAY )  key = BTN_SELECT_Long;                
                   else                                     key = BTN_SELECT;
                   keydown=BTN_NONE;
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

const int ProgStamp = 3218;

#define Adr_Stamp  0
#define Adr_MinUs  2
#define Adr_MaxUs 34

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
  Serial.println(F("Save eeprom"));
}

void InitFromEprom () {
  int temp;
  EEPROM.get(Adr_Stamp, temp);
  if ( temp == ProgStamp ) {  // Stamp found , we can assume eerom contain Valid Datas 
    Serial.println(F("Stamp Found"));
    Serial.println(F("Read eeprom"));
    EEPROM.get(Adr_MinUs, MinUs);
    EEPROM.get(Adr_MaxUs, MaxUs);

    Serial.print("UsMin : ");
    for (byte i=0;i<16;i++ ) {
      Serial.print(MinUs[i]);    
      Serial.print(",");  
    }
    Serial.println("");

    Serial.print("UsMax : ");
    for (byte i=0;i<16;i++ ) {
      Serial.print(MaxUs[i]);    
      Serial.print(",");  
    }
    Serial.println("");
  
  } else {                    // Stamp not found , Values need to be initialized
    Serial.println(F("Stamp Not Found"));
    InitVars();
    SaveToEprom(); 
  }
}

// -------------------------------------------------------------
// 
// -------------------------------------------------------------

void TextMenu(String str) {
  Serial.println(str);
  OLed.clearDisplay();
  OLed.setTextSize(2);
  OLed.setTextColor(SSD1306_WHITE);
  OLed.setCursor(5, 10);
  OLed.println(str); 
}

// -------------------------------------------------------------
//  Mode Live
// -------------------------------------------------------------

void DoLive() {
//  Serial.println(F("Enter Live Mode"));
//  OLed.clearDisplay();
//  OLed.setTextSize(2);
//  OLed.setTextColor(SSD1306_WHITE);
//  OLed.setCursor(0, 10);
//  OLed.println(F("Live Mode"));
//  OLed.display();
    TextMenu(F("Live Mode"));
    OLed.display();
    
  boolean LetRunning = true;
  unsigned long omillis = 0;
  unsigned long amillis = 0;  
  
  while ( LetRunning ) {
    ReadChannels();
    UpdateServos();
    ReadBtnState();
    if ( readkey() == BTN_SELECT ) { // if Select Pressed exit
      LetRunning=false;
    }

//    if ( millis() - omillis > 1000 ) { // send datas to serial
//      omillis=millis();
//      Serial.print("Value : ");
//      for(int i = 0; i < 16; i ++){
//        Serial.print(Values[i]); 
//        Serial.print(" , ");
//      }
//      Serial.println(); 
//    }

    if ( millis() - amillis > 150 ) {  // Display pot changed
      TextMenu(F("Live Mode"));
      int microsec = map(Values[LastChanelchanged], 0, 1023, MinUs[LastChanelchanged],  MaxUs[LastChanelchanged]);
      OLed.setCursor(4 ,50);  OLed.print(LastChanelchanged);
      OLed.setCursor(68,50);  OLed.print(microsec);
      OLed.display();
    }
  }

  ClearEncoder();
  Serial.println(F("Leave Live Mode"));
}

// -------------------------------------------------------------
//          P L A Y  &  R E C C O R D    S E Q U E N C E 
// -------------------------------------------------------------

#define NbStep  10
#define NbLines  2
int Sequence [NbLines][16] = {
//  { 1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500 },
//  { 1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000 },
  { 2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000 },
  { 1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500 } };
int Current[16];
int Speed = 500;

void DoRecord() {
  TextMenu(F("Reccord"));
  OLed.display();
  boolean LetRunning = true;
  while ( LetRunning ) {
    ReadBtnState();
    if ( readkey() == BTN_SELECT ) { // if Select Pressed exit
      LetRunning=false;
    }
       
  }  
  ClearEncoder();
}

// -------------------------------------------------------------

void setCurrent( int Line, int Step ) {
  for (int i=0; i<16; i++ ) {
    Current[1]= ( Step * Sequence[Line][i] + ( NbStep-1 - Step ) * Sequence[Line][i] ) / NbStep;
  }
}
  
void DoPlay() { 
  boolean WaitToStart = true;
  boolean LetRunning  = true;
  while ( WaitToStart ) {
    ReadBtnState();
    OLed.clearDisplay();
    OLed.setTextSize(2);
    OLed.setCursor(4,  4);  OLed.print(F("Press Sel"));
    OLed.setCursor(8, 21);  OLed.print(F("to start"));
    OLed.setCursor(4, 50);  OLed.print(F("speed"));
    OLed.setCursor(68,50);  OLed.print(Speed);
    OLed.display();
    if ( readkey() == BTN_SELECT ) { 
      WaitToStart=false;
    }
    if (encodermov != 0 ) {
      cli();
      Speed+=encodermov*10;
      encodermov=0;
      sei();
      if ( Speed > 1000 ) { Speed = 1000; }
      if ( Speed <    0 ) { Speed =    0; }
    }   
  } 

  boolean disp = true;
  int cline=0;
  int cstep=0;
  int otime = millis();
  
  while ( LetRunning ) {
    ReadBtnState();
    if ( readkey() == BTN_SELECT ) { // if Select Pressed exit
      LetRunning=false;
    }
    setCurrent( cline, cstep );
    for(int i=0;i<16;i++) {
      SetServo(i, Current[i] );
    }      
    if ( disp) {
      OLed.clearDisplay();
      OLed.setTextSize(2);
      OLed.setCursor( 4,  4);  OLed.print(F("Running"));
      OLed.setCursor( 4, 16);  OLed.print(cline);
      OLed.setCursor(68, 16);  OLed.print(cstep);
      OLed.setCursor( 4, 50);  OLed.print(F("speed"));
      OLed.setCursor(68, 50);  OLed.print(Speed);
      OLed.display();
    }
    int t0=millis();
    if ( ( t0 - otime ) >= Speed ) {
      otime = t0;
      cstep++;
      disp=true;
      if ( cstep >= NbStep ) {
        cstep=0;
        cline++;
        if ( cline >= NbLines ) {
          cline=0;
          LetRunning=false;
        }
      }
    }  
          
  }
  for(int i=0;i<16;i++) {
    SetServo(i, Sequence[0][i] );
  }    
  ClearEncoder(); 
}

// -------------------------------------------------------------
//                  C O N F I G U R A T I O N
// -------------------------------------------------------------

void DoSetup() {
  boolean LetRunning = true;
  int Channel        = 0;
  int AnaVal;
  int us;
  
  while ( LetRunning ) {
    ReadBtnState();
    
    switch ( readkey() ) {
      case BTN_RIGHT:       MaxUs[Channel]=us;
              break;     
      case BTN_RIGHT_Long:  MaxUs[Channel]=MAX_US;
              break;
      case BTN_LEFT:        MinUs[Channel]=us;
              break;
      case BTN_LEFT_Long:   MinUs[Channel]=MIN_US;
              break;
      case BTN_SELECT:      LetRunning=false;
              break;
      case BTN_SELECT_Long: SaveToEprom();
                            LetRunning=false;
              break;
    }
             
    if (encodermov != 0 ) {
      cli();
      Channel+=encodermov;
      encodermov=0;
      sei();
      if ( Channel > 15 ) { Channel =  0; }
      if ( Channel <  0 ) { Channel = 15; }
    }
    AnaVal=ReadChannel(Channel);
    us    =map(AnaVal, 0, 1023, MIN_US,MAX_US);
    
    OLed.clearDisplay();
    OLed.setTextColor(SSD1306_WHITE);
    OLed.setCursor(4,  0);  OLed.print(F("Setup"));
//    OLed.setCursor(4,  0);  OLed.print(F("L Min"));
//    OLed.setCursor(4, 16);  OLed.print(F("R Max"));
    OLed.setCursor(4, 33);  OLed.print(F("C ")); OLed.print(Channel);
    OLed.setCursor(4, 50);  OLed.print(us);      
    OLed.setCursor(68,33);  OLed.print(MinUs[Channel]);
    OLed.setCursor(68,50);  OLed.print(MaxUs[Channel]);
    OLed.display();
      
  }
  ClearEncoder();
}
    
// -------------------------------------------------------------
//                 I N I T I A L I Z A T I O N
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
  Serial.println(F("PCA9685 Initialized"));
  InitFromEprom();  
  
  if(!OLed.begin(SSD1306_SWITCHCAPVCC, OLedAdr)) {
    Serial.println(F("SSD1306 Error"));
  } else {
    Serial.println(F("SSD1306 Initialized"));
  }
  OLed.display();  // display adafruit logo

  attachInterrupt(digitalPinToInterrupt(ROT_A), doEncoder, CHANGE );  

  Serial.println(F("Running ..."));
  delay(2000);
}

// -------------------------------------------------------------
//                     M A I N    L O O P
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
    OLed.setCursor(4, 16);  OLed.println(F("Play"));
    OLed.setCursor(4, 33);  OLed.println(F("Record"));
    OLed.setCursor(4, 50);  OLed.println(F("Setup"));
    OLed.display();
    omenu=menu;
  }
  if ( keypressed() ) {
    byte k = readkey();
    if ( ( k == BTN_ENC ) || ( k == BTN_SELECT ) ) {
      switch (menu ) {
        case 0: DoLive();
                break;
        case 1: DoPlay();
                break;        
        case 2: DoRecord(); 
                break;
        case 3: DoSetup();
                break;                                       
      }
    }
    omenu=menu+1;  // to force redraw 
  }
}
