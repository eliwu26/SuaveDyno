/** 
Elias Wu and Trent Lukaczyk 
October 2015

**/

// ------------------------------------------------------------------------------
//   INCLUDES
// ------------------------------------------------------------------------------

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <HX711.h>
#include <Servo.h>


// ------------------------------------------------------------------------------
//   GLOBALS
// ------------------------------------------------------------------------------

// Scales
#define SCALE_THRUST_CALIBRATION +461.406f
//#define SCALE_THRUST_CALIBRATION +1.0f
//#define SCALE_THRUST_CONSTANT    0.4080f
#define SCALE_THRUST_CONSTANT    1.0f

#define SCALE_TORQUE_CALIBRATION -1461.036f
//#define SCALE_TORQUE_CALIBRATION 1.0f
//#define SCALE_TORQUE_CONSTANT    0.1020f
#define SCALE_TORQUE_CONSTANT    1.0f

#define SCALE_THRUST_CHANNEL     128
#define SCALE_TORQUE_CHANNEL     32
HX711 scale_thrust(A2,A3);
HX711 scale_torque(A2,A3);

// ------------------------------------------------------------------------------
//   SETUP
// ------------------------------------------------------------------------------

void setup() {
    
  // Run serial interface
  Serial.begin(38400); 
  
  // Startup
  Serial.println("Load Cell Calibration");
  
  // Set analog pin reference voltage
  analogReference(DEFAULT);
  
  // Scale Initialization
  tare_scales();

  // Let it soak
  delay(1000);
  
}


// ------------------------------------------------------------------------------
//   LOOP
// ------------------------------------------------------------------------------

void loop() {

//  scale_thrust.set_gain(SCALE_THRUST_CHANNEL);
//  Serial.print( scale_thrust.get_units(1) * SCALE_THRUST_CONSTANT );

  scale_torque.set_gain(SCALE_TORQUE_CHANNEL);
  Serial.print( scale_torque.get_units(1) * SCALE_TORQUE_CONSTANT );


  delay(300);

//  // Thrust
//  Serial.print("Thrust: ");
//  scale_thrust.set_gain(SCALE_THRUST_CHANNEL);
//  Serial.print( scale_thrust.get_units(1) * SCALE_THRUST_CONSTANT );
//  Serial.print(" g, ");
//  
//  // Torque
//  Serial.print("Torque: ");
//  scale_torque.set_gain(SCALE_TORQUE_CHANNEL);
//  Serial.print( scale_torque.get_units(1) * SCALE_TORQUE_CONSTANT );
//  Serial.print(" g-m, ");

  Serial.println();
      
}


// Tare Scales
void tare_scales() {
  scale_thrust.set_gain (SCALE_THRUST_CHANNEL);
  scale_thrust.set_scale(SCALE_THRUST_CALIBRATION);
  scale_thrust.tare();  
  scale_torque.set_gain (SCALE_TORQUE_CHANNEL);
  scale_torque.set_scale(SCALE_TORQUE_CALIBRATION);
  scale_torque.tare();
}


