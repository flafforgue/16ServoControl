
// -------------------------------------------------------------
//
//  Calibration on Servo 0 to 3 , others works normaly
//
// Servo 0     :  1000 us
// Servo 1     :  1500 us
// Servo 2     :  2000 us
// Servo 3     : 10000 us // not for servo but for more precision
// Servo 4 & + : acc pot values
//
// Set OSCFREQUENCY to 25000000
// Check with Osciloscope real with of pulse
// then correct value  New F = F * Measured Pusle With / Theoritical Pulse with
// otherwise measure frequency , and correcto for having exactly 50 Hz
//
// -------------------------------------------------------------

#define OSCFREQUENCY 28729000

// #define OSCFREQUENCY // clone    25758000
// #define OSCFREQUENCY // adafruit 28729000
  
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVO_FREQ 50

// -------------------------------------------------------------

#define s0 4
#define s1 5
#define s2 6
#define s3 7

#define SIG A3

// -------------------------------------------------------------

int muxChannel[16][4]={
  {1,1,0,0}, // channel 0
  {0,1,0,0}, // channel 1
  {1,0,0,0}, // channel 2
  {0,0,0,0}, // channel 3
    
  {1,1,1,0}, // channel 4
  {0,1,1,0}, // channel 5
  {1,0,1,0}, // channel 6
  {0,0,1,0}, // channel 7
    
  {1,1,0,1}, // channel 8
  {0,1,0,1}, // channel 9
  {1,0,0,1}, // channel 10
  {0,0,0,1}, // channel 11
    
  {1,1,1,1}, // channel 12
  {0,1,1,1}, // channel 13
  {1,0,1,1}, // channel 14
  {0,0,1,1}  // channel 15
};

int controlPin[] = {s0, s1, s2, s3};

int readMux(int channel){
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }
  delay(10);
  int val = analogRead(SIG);
  return val;
}

// -------------------------------------------------------------

#define USMIN   800  
#define USMAX  2200

void SetServo(int num, int pos) {
  int microsec = map(pos, 0, 1023, USMIN,  USMAX);
  pwm.writeMicroseconds(num, microsec); 
}

// -------------------------------------------------------------

void setup(){
  Serial.begin(115200);
    
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT); 
    
  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  pwm.begin();                          
  pwm.setOscillatorFrequency(OSCFREQUENCY); 
  pwm.setPWMFreq(SERVO_FREQ);   
  delay(500);
  
  pwm.writeMicroseconds(0, 1000);
  pwm.writeMicroseconds(1, 1500);  
  pwm.writeMicroseconds(2, 2000);
  pwm.writeMicroseconds(3,10000);
}


void loop(){
  Serial.print("Value : ");
  for(int i = 4; i < 16; i ++){
    int value = readMux(i);
    SetServo(i,value);
    Serial.print(value); 
    Serial.print(" , ");
  }
  Serial.println();
  delay(1000);
}
