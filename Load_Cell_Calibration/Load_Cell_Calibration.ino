/** 
SuaveDyno Load Cell Calibration
Elias Wu and Trent Lukaczyk 2015

Procedure: 
Boot up Arduino with this code, start Serial Monitor at 38400 baud, wait for readings, then 
place known weight on load cell. Divide reading by actual weight to obtain scaling factor
(The value in set_scale(value))
**/

#include <HX711.h>
#include <Servo.h>

// HX711.DOUT	- pin #A2
// HX711.PD_SCK	- pin #A3

HX711 scale(A2, A3);		// parameter "gain" is ommited; the default value 128 is used by the library

void setup() {
  Serial.begin(38400);
  Serial.println("HX711 Calibration");

  //First (5kg) load cell calibration
  scale.set_gain(32);
  scale.set_scale();
  scale.tare();	
  
  //Second (300g) load cell calibration - comment above 3 lines and uncomment following 3 lines.
//  scale.set_gain(32);
//  scale.set_scale(-1284.38f);
//  scale.tare();		
  Serial.println("Readings:");
}

void loop() {
  Serial.println(scale.get_units(10));
}
