// -------------------------------------------------------------
//
//
// 19/04/2023  Flash :  21844  Mem : 857   Reste : 1191 // 2 lines - ok
// 20/04/2023  Flash :  21714  Mem : 729   Reste : 1319 // 4 lines - Change sequence from int to byte
// 20/04/2023  Flash :  21681  Mem : 681   Reste : 1367 // all changed from int to byte
//                                   713           1335 // 6 lines - ok
// 21/04/2023  Flash :  21862  Mem : 713   Reste : 1335
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

byte Values[16];
byte LastChanelchanged = 0;

const byte ch0[16] PROGMEM = { 0,0,0,0, 1,1,1,1, 0,0,0,0, 1,1,1,1 };
const byte ch1[16] PROGMEM = { 0,0,0,0, 0,0,0,0, 1,1,1,1, 1,1,1,1 };
const byte ch2[16] PROGMEM = { 0,1,0,1, 0,1,0,1, 0,1,0,1, 0,1,0,1 };
const byte ch3[16] PROGMEM = { 0,0,1,1, 0,0,1,1, 0,0,1,1, 0,0,1,1 };

byte ReadChannel(byte chn){            // Read One Channel
  digitalWrite(S0, pgm_read_byte(&ch0[chn]) );
  digitalWrite(S1, pgm_read_byte(&ch1[chn]) );
  digitalWrite(S2, pgm_read_byte(&ch2[chn]) );
  digitalWrite(S3, pgm_read_byte(&ch3[chn]) );
  delay(10);
  byte val = analogRead(SIGNAL) >> 2;
  return val;
}

void ReadChannels(){                // Read all Channels
  for(byte i=0;i<16;i++) {
    byte temp=ReadChannel(i);
    if ( abs( Values[i] - temp ) > 2) {
       LastChanelchanged=i;
    }
    Values[i]= temp;
  }
}

// -------------------------------------------------------------
//  Set Servo values
// -------------------------------------------------------------

#define MIN_US   50    //   500 us
#define MAX_US  250    //  2500 us

byte MinUs[16];  //  
byte MaxUs[16];  //  

void SetServo(byte num, byte pos) {
  int microsec = map(pos, 0, 255, MinUs[num]*10,  MaxUs[num]*10); 
  pwm.writeMicroseconds(num, microsec); 
}

void UpdateServos() {
  for(byte i=0;i<16;i++) {
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

const int ProgStamp = 3219;

#define Adr_Stamp  0
#define Adr_MinUs  2
#define Adr_MaxUs 18

void InitVars() {
  for ( byte i=0; i<16 ; i++ ) {
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
//                  M E N U    &   D I S P L A Y
// -------------------------------------------------------------

#define L1       0
#define L2      16
#define L3      32
#define L4      48
#define Lmargin  4

void TextMenu(String str) {
//  Serial.println(str);
  OLed.clearDisplay();
  OLed.setTextSize(2);
  OLed.setTextColor(SSD1306_WHITE);
  OLed.setCursor(Lmargin, L1);
  OLed.println(str); 
}

void OLedprint2 ( int value ) {
//  char buff[3];
//  sprintf(buff, "%2d", value);
//  OLed.print(buff);
   if (( value ) < 10 ) { OLed.print(F(" ")); } 
   OLed.print(value);
}

void OLedprint4 ( int value ) {
//  char buff[5];
//  sprintf(buff, "%4d", value);
//  OLed.print(buff);
   if (( value ) < 1000 ) { OLed.print(F(" ")); } 
   if (( value ) <  100 ) { OLed.print(F(" ")); }   
   if (( value ) <   10 ) { OLed.print(F(" ")); }
   OLed.print(value);
}

// -------------------------------------------------------------
//  Mode Live
// -------------------------------------------------------------

void DoLive() {
  boolean       LetRunning = true;
  unsigned long omillis    = 0;  
    
  while ( LetRunning ) {
    ReadChannels();
    UpdateServos();
    ReadBtnState();
    if ( readkey() == BTN_SELECT ) {  // if Select Pressed exit
      LetRunning=false;
    }

    if ( millis() - omillis > 150 ) { // Refresh Display every 150 us
      TextMenu(F("Live"));
      int microsec = map(Values[LastChanelchanged], 0, 255, MinUs[LastChanelchanged],  MaxUs[LastChanelchanged]);
      OLed.setCursor(4 ,L4);  OLedprint2(LastChanelchanged);
      OLed.setCursor(68,L4);  OLedprint4(microsec*10);
      OLed.display();
      omillis = millis();
    }
  }

  ClearEncoder();
}

// -------------------------------------------------------------
//          P L A Y  &  R E C C O R D    S E Q U E N C E 
// -------------------------------------------------------------

#define NbStep  10
#define NbLines  6
byte Sequence [NbLines][16] = {
  { 000,255,000,000,000,000,000,000,000,000,000,000,000,000,000,000 },
  { 255,000,255,000,000,000,000,000,000,000,000,000,000,000,000,000 },
  { 000,255,000,000,000,000,000,000,000,000,000,000,000,000,000,000 },
  { 255,000,255,000,000,000,000,000,000,000,000,000,000,000,000,000 },
  { 000,255,000,000,000,000,000,000,000,000,000,000,000,000,000,000 },
  { 255,000,255,000,000,000,000,000,000,000,000,000,000,000,000,000 } };

// int Current[16]; // deleted use Values
int Speed = 500;



void DoRecord() {
  boolean       LetRunning = true;
  unsigned long omillis    = 0;  
  char          CurLine    = 0;

  while ( LetRunning ) {
    ReadChannels();
    UpdateServos();
    ReadBtnState();

    switch ( readkey() ) {
      case BTN_RIGHT:       CurLine++;
                            break;
      case BTN_LEFT:        CurLine--;
                            break;    
      case BTN_SELECT:      
      case BTN_SELECT_Long: LetRunning=false;                        
    }
    if ( CurLine <       0 ) { CurLine =       0; }
    if ( CurLine > CurLine ) { CurLine = CurLine; }
    
    if ( millis() - omillis > 150 ) {  // Refresh Display every 150 us
      TextMenu(F("Reccord"));
      int microsec = map(Values[LastChanelchanged], 0, 255, MinUs[LastChanelchanged],  MaxUs[LastChanelchanged]);

      OLed.setCursor( 4, L2);  OLed.print(F("Step"));
      OLed.setCursor(68, L2);  OLedprint4( (byte) CurLine+1);
      
      OLed.setCursor( 4, L4);  OLedprint2(LastChanelchanged);
      OLed.setCursor(68, L4);  OLedprint4(microsec*10);
      OLed.display();
      omillis = millis();
    }
  }

  ClearEncoder();
//  Serial.println(F("Leave Live"));
}

// -------------------------------------------------------------

void setCurrent( int Line, int Step ) {
  int tmp;
//  Serial.print(Line);   Serial.print(" , ");
//  Serial.print(Step);   Serial.print(" : ");
//  Serial.print(Sequence[Line][0]);   Serial.print(" : ");
  for (byte i=0; i<16; i++ ) {
    tmp = (( int ) ( NbStep-1 -  Step) * (int) Sequence[Line][i]  +  (int )( Step ) * ( int ) Sequence[(Line+1) % NbLines][i])/10;
    Values[i]= tmp; 
//    Serial.print(tmp);   Serial.print(" , ");
//    Values[1]= (( Step * Sequence[Line][i] + ( NbStep-1 - Step ) * Sequence[Line][i] ) *10 )/ NbStep;
  }
//  Serial.println("");
}

void DoPlay() { 
  boolean WaitToStart = true;
  boolean LetRunning  = true;
  while ( WaitToStart ) {
    ReadBtnState();
    OLed.clearDisplay();
    OLed.setTextSize(2);
    OLed.setCursor(4, L1);  OLed.print(F("Press Sel"));
    OLed.setCursor(8, L2);  OLed.print(F("to start"));
    OLed.setCursor(4, L4);  OLed.print(F("speed"));
    OLed.setCursor(68,L4);  OLedprint4(Speed);
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

  boolean       disp  = true;
  byte          cline = 0;
  byte          cstep = 0;
  unsigned long otime = millis();
  
  while ( LetRunning ) {
    ReadBtnState();
    if ( readkey() == BTN_SELECT ) { // if Select Pressed exit
      LetRunning=false;
    }
    setCurrent( cline, cstep );
    UpdateServos();
//    for(byte i=0;i<16;i++) {
//      SetServo(i, Values[i] );
//    }      
    if ( disp) {
      OLed.clearDisplay();
      OLed.setTextSize(2);
      OLed.setCursor( 4, L1);  OLed.print(F("Running"));
      OLed.setCursor(20, L2);  OLedprint2(cline);
      OLed.setCursor(68, L2);  OLedprint2(cstep);
      OLed.setCursor( 4, L4);  OLed.print(F("speed"));
      OLed.setCursor(68, L4);  OLedprint4(Speed);
      OLed.display();
    }
    unsigned long t0=millis();
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
  boolean   LetRunning  = true;
  char      Channel     = 0;
  byte      AnaVal;
  int       us;
  
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
      if ( Channel <  0 ) { Channel = 15; }
      if ( Channel > 15 ) { Channel =  0; }
    }
    AnaVal=ReadChannel(Channel);
    us    =map(AnaVal, 0, 255, MIN_US,MAX_US);
    
    OLed.clearDisplay();
    OLed.setTextColor(SSD1306_WHITE);
    OLed.setCursor(4, L1);  OLed.print(F("Setup"));
    OLed.setCursor(4, L3);  OLed.print(F("C ")); OLedprint2( (byte) Channel);
    OLed.setCursor(4, L4);  OLedprint4(us*10);      
    OLed.setCursor(68,L3);  OLedprint4(MinUs[Channel]*10);
    OLed.setCursor(68,L4);  OLedprint4(MaxUs[Channel]*10);
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

//  for (byte i=0; i<NbLines ; i++ ) {
//    for ( byte j=0; j<16 ; j++ ) {
//      Sequence[i][j]=127;
//    }
//  }

  Serial.println(F("Running ..."));
  delay(2000);
  ClearEncoder();
}

// -------------------------------------------------------------
//                     M A I N    L O O P
// -------------------------------------------------------------

int menu  = 0;
int omenu = 2;

void loop(){
  ReadBtnState(); 

  if (encodermov != 0 ) {
    cli();
    menu+=encodermov;
    encodermov=0;
    sei();
    if ( menu > 3 ) { menu=0; }
    if ( menu < 0 ) { menu=3; }
  }

  if ( menu != omenu ) {
    OLed.clearDisplay();
    OLed.setTextSize(2);
    OLed.fillRect(0, menu*16, 127, 16, SSD1306_WHITE);
    OLed.setTextColor(SSD1306_INVERSE);
    OLed.setCursor(4, L1);  OLed.print(F("Live"));
    OLed.setCursor(4, L2);  OLed.print(F("Play"));
    OLed.setCursor(4, L3);  OLed.print(F("Record"));
    OLed.setCursor(4, L4);  OLed.print(F("Setup"));
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
