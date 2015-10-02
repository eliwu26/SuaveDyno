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
int Pot;

void setup() {
  Serial.begin(38400);
  Serial.println("HX711 Demo");
  myservo.attach(9);
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
  scale.set_scale(-423.92f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();	
  scale2.set_gain(32);
  scale2.set_scale(-1284.38f);
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
  VRaw = analogRead(A0);
  IRaw = analogRead(A1);
  VFinal = VRaw/12.99; //180 Amp board
  IFinal = IRaw/3.7; //180 Amp board
  Pot = analogRead(A4);
  scale.set_gain(128);
  scale.set_scale(-423.92f);  
  Serial.print("5KG: ");
  Serial.print(scale.get_units(1));
  Serial.print(" .3KG: ");
  scale2.set_gain(32);
  scale2.set_scale(-1284.38f);
  Serial.print(scale2.get_units(1));
  Serial.print(" Attopilot V: ");
  Serial.print(VFinal);
  Serial.print(" Attopilot I: ");
  Serial.print(IFinal);
  Serial.print(" Pot: ");
  Serial.println(Pot);
  myservo.writeMicroseconds(map(Pot,0,900,950,1700));
}
