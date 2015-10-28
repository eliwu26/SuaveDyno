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

// ADC for Battery Voltage and Current
#define VOLTAGE_CONSTANT 0.002955
#define CURRENT_CONSTANT 0.010000
Adafruit_ADS1115 adc_battery;

// ------------------------------------------------------------------------------
//   SETUP
// ------------------------------------------------------------------------------

void setup() {
    
  // Run serial interface
  Serial.begin(38400); 
  
  // Startup
  Serial.println("Voltage and Current Calibration");

  // Set analog pin reference voltage
  analogReference(DEFAULT);
  
  // ADC Initialization
  adc_battery.begin();

  // Let it soak
  delay(1000);

}


// ------------------------------------------------------------------------------
//   LOOP
// ------------------------------------------------------------------------------

void loop() {
  
  // Battery Voltage and Current
  Serial.print("Voltage: ");
  Serial.print( adc_battery.readADC_SingleEnded(0) * VOLTAGE_CONSTANT );
  Serial.print(" V, ");
  Serial.print("Current: ");
  Serial.print( adc_battery.readADC_SingleEnded(1) * CURRENT_CONSTANT );
  Serial.print(" A, ");

  delay(500);

  // Next line
  Serial.println();
  
}



