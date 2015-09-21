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
int startEmergencyPin = 2;
int armPin = 3;
int startEmergencyLed = 5;
int armLed = 6;
int testStarted = 0;
int armed = 0;

void setup() {
  Serial.begin(38400);
  Serial.println("HX711 Demo");
  myservo.attach(9);
  delay(3000);
  myservo.writeMicroseconds(1950);
  delay(1500);
  myservo.writeMicroseconds(950);
  delay(1500);
  pinMode(startEmergencyPin,INPUT_PULLUP);
  pinMode(armPin,INPUT_PULLUP);
  pinMode(startEmergencyLed,OUTPUT);
  pinMode(armLed,OUTPUT);
//  Serial.println("Before setting up the scale:");
//  Serial.print("read: \t\t");
//  Serial.println(scale.read());			// print a raw reading from the ADC
//
//  Serial.print("read average: \t\t");
//  Serial.println(scale.read_average(20));  	// print the average of 20 readings from the ADC
//
//  Serial.print("get value: \t\t");
//  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight (not set yet)
//
//  Serial.print("get units: \t\t");
//  Serial.println(scale.get_units(5), 1);	// print the average of 5 readings from the ADC minus tare weight (not set) divided 
//						// by the SCALE parameter (not set yet)  
  scale.set_gain(128);
  scale.set_scale(-419.66f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();	
  scale2.set_gain(32);
  scale2.set_scale(-1256.36f);
  scale2.tare();			        // reset the scale to 0
//  scale2.set_scale(2280.f);
//  scale2.tare();

//  Serial.println("After setting up the scale:");
//
//  Serial.print("read: \t\t");
//  Serial.println(scale.read());                 // print a raw reading from the ADC
//
//  Serial.print("read average: \t\t");
//  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC
//
//  Serial.print("get value: \t\t");
//  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight, set with tare()
//
//  Serial.print("get units: \t\t");
//  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided 
//						// by the SCALE parameter set with set_scale

  Serial.println("Readings:");
}

void loop() {
   
//  Serial.print("one reading:\t");
//  Serial.print(scale.get_units(), 1);
//  Serial.print("\t| average:\t");
//  Serial.println(scale.get_units(1), 1);
////  Serial.print("second reading:\t");
////  Serial.println(scale2.get_units(1),1);
//
//  scale.power_down();			        // put the ADC in sleep mode
////  delay(5000);
//  scale.power_up();
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
      delay(1000);
      for(int j = 0; j < 5; j++){
        int thrustValue = 950+(i*100);
        myservo.writeMicroseconds(thrustValue);
        VRaw = analogRead(A0);
        IRaw = analogRead(A5);
        VFinal = VRaw/12.99; //180 Amp board
        IFinal = IRaw/3.7; //180 Amp board
        scale.set_gain(128);
      //  scale.set_scale(-423.92f);  
        Serial.print("Thrust: ");
        Serial.print(scale.get_units(1)/2.4525);
        Serial.print("g ");
        Serial.print(" Torque: ");
        scale2.set_gain(32);
      //  scale2.set_scale(-1284.38f);
        Serial.print(scale2.get_units(1)*0.1019);
        Serial.print("g-m ");
        Serial.print(" Attopilot V: ");
        Serial.print(VFinal);
        Serial.print(" Attopilot I: ");
        Serial.print(IFinal);
        Serial.print(" PWM Value: ");
        Serial.print(thrustValue);
        Serial.print(" Thrust %: ");
        Serial.print(((thrustValue-950)/1000.0)*100);
        Serial.print("%");
        Serial.print(" A1: ");
        Serial.println(analogRead(A1));
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
