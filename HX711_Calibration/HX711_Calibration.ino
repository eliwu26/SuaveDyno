#include "HX711.h"
#include <Servo.h>

// HX711.DOUT	- pin #A2
// HX711.PD_SCK	- pin #A3

HX711 scale(A2, A3);		// parameter "gain" is ommited; the default value 128 is used by the library

void setup() {
  Serial.begin(38400);
  Serial.println("HX711 Calibration");

  scale.set_gain(32);
  scale.set_scale();                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();	
//  scale.set_gain(32);
//  scale.set_scale(-1284.38f);
//  scale.tare();		
  Serial.println("Readings:");
}

void loop() {
  Serial.println(scale.get_units(10));
}
