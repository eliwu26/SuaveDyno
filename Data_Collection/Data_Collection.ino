//SuaveDyno
#include "HX711.h"
#include <Servo.h>
// HX711.DOUT	- pin #A2
// HX711.PD_SCK	- pin #A3

HX711 scale(A2, A3);		// parameter "gain" is ommited; the default value 128 is used by the library
byte g = 32;
HX711 scale2(A2,A3);
Servo myservo;
int VRaw; //This will store our raw ADC data
int IRaw;
float VFinal; //This will store the converted data
float IFinal;
int startEmergencyPin = 6;
int armPin = 3;
int startEmergencyLed = 4;
int armLed = 7;
int testStarted = 0;
int armed = 0;
int rpmPin = 2;
int vPin = A0;
int iPin = A5;
void setup() {
  Serial.begin(38400);
  Serial.println("HX711 Demo");
  myservo.attach(9);
  delay(3000);
  pinMode(startEmergencyLed,OUTPUT);
  myservo.writeMicroseconds(2000);
  for(int i = 0; i < 3; i ++){
    digitalWrite(startEmergencyLed,1);
    delay(100);
    digitalWrite(startEmergencyLed,0);
    delay(100);
  }
  delay(2000);
  myservo.writeMicroseconds(950);
  delay(1500);
  pinMode(startEmergencyPin,INPUT_PULLUP);
  pinMode(armPin,INPUT_PULLUP);
  pinMode(armLed,OUTPUT);
  pinMode(rpmPin,INPUT);
  scale.set_gain(128);
  scale.set_scale(-409.08f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();	
  scale2.set_gain(32);
  scale2.set_scale(1042.36f);
  scale2.tare();			        // reset the scale to 0

  Serial.println("Readings:");
}

void loop() {
  if(digitalRead(armPin) == 0){
    if(armed == 0){
      armed = 1;
      myservo.writeMicroseconds(950);
      analogWrite(armLed,200);
    }
  }
  if(digitalRead(startEmergencyPin) == 0&&armed == 1){
    if(testStarted==0){
      testStarted = 1;
      analogWrite(startEmergencyLed,200);
      delay(2000);
    }
  }
  if(testStarted == 1){
    scale.set_gain(128);
    scale.set_scale(-419.66f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
    scale.tare();	
    scale2.set_gain(32);
    scale2.set_scale(-1256.36f);
    scale2.tare();
    for(int i = 0; i < 11; i++){
      int thrustValue = 950+(i*100);
      myservo.writeMicroseconds(thrustValue);
      delay(1000);
      for(int j = 0; j < 5; j++){
        
        VRaw = analogRead(vPin);
        IRaw = analogRead(iPin);
        VFinal = VRaw*.04932/1.06; //180 Amp board
        IFinal = (IRaw*.083/1.31); //180 Amp board
        scale.set_gain(128);
      //  scale.set_scale(-423.92f);  
        Serial.print("Thrust: ");
        Serial.print(scale.get_units(1)/2.4525);
        Serial.print("g,");
        Serial.print(" Torque: ");
        scale2.set_gain(32);
      //  scale2.set_scale(-1284.38f);
        Serial.print(scale2.get_units(1)*0.1019);
        Serial.print("g-m,");
        Serial.print(" Voltage: ");
        Serial.print(VFinal);
        Serial.print("V,");
        Serial.print(" Current: ");
        Serial.print(IFinal);
        Serial.print("A,");
        Serial.print(" PWM: ");
        Serial.print(thrustValue);
        Serial.print(",");
        Serial.print(" Throttle: ");
        Serial.print(((thrustValue-950)/1000.0)*100);
        Serial.print("%,");
        Serial.print(" RPM: ");
        Serial.println(measureRPM());
        if(digitalRead(startEmergencyPin) == 0||testStarted == 0){
          myservo.writeMicroseconds(950);
          analogWrite(startEmergencyLed,0);
          delay(50);
          analogWrite(startEmergencyLed,200);
          delay(50);
          analogWrite(startEmergencyLed,0);
          delay(50);
          analogWrite(startEmergencyLed,200);
          delay(50);
          analogWrite(startEmergencyLed,0);
          delay(50);
          analogWrite(startEmergencyLed,200);
          delay(50);
          analogWrite(startEmergencyLed,0);
          delay(50);
          analogWrite(startEmergencyLed,200);
          delay(50);
          analogWrite(startEmergencyLed,0);
          delay(50);
          analogWrite(startEmergencyLed,200);
          delay(50);
          testStarted = 0;
          break;
        }
      }
      if(digitalRead(startEmergencyPin) == 0||testStarted == 0){
        myservo.writeMicroseconds(950);
        analogWrite(startEmergencyLed,0);
        delay(50);
        analogWrite(startEmergencyLed,200);
        delay(50);
        analogWrite(startEmergencyLed,0);
        delay(50);
        analogWrite(startEmergencyLed,200);
        delay(50);
        analogWrite(startEmergencyLed,0);
        delay(50);
        analogWrite(startEmergencyLed,200);
        delay(50);
        analogWrite(startEmergencyLed,0);
        delay(50);
        analogWrite(startEmergencyLed,200);
        delay(50);
        analogWrite(startEmergencyLed,0);
        delay(50);
        analogWrite(startEmergencyLed,200);
        delay(50);
        testStarted = 0;
        Serial.println("EMERGENCY STOP!");
        break;
      }
    }
    myservo.writeMicroseconds(950);
    testStarted = 0;
    analogWrite(startEmergencyLed,0);
    Serial.println();
    Serial.println("Test completed.\n");
  }
}

 
int measureRPM(){
  long currtime = micros();                 // GET CURRENT TIME
  int changes = 0;
  int rpm = 0;
  bool timeOut = false;
  while(changes < 5 && !timeOut){
    
    if((micros()-currtime)>1000000) 
      timeOut = true;
    
    delayMicroseconds(1);
    
    if(digitalRead(2) == 0){
      
      while(digitalRead(2) == 0 && !timeOut){
        
        if((micros()-currtime)>1000000) 
          timeOut = true;
          
        delayMicroseconds(1);
        
      }
      if(digitalRead(2) == 1){
        if(changes == 0) 
          currtime = micros();
          
        changes ++;
      }
    }
  }
  if (changes == 5){
    rpm = (((changes-1)* 60 * 1000000)/(micros()-currtime)) ;       //  CALCULATE  RPM USING REVOLUTIONS AND ELAPSED TIM
  }
  return rpm;
    
}
