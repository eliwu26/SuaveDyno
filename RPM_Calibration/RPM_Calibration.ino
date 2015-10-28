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

// Servo driver for throttle
#define PWM_THROTTLE_LOW  1000
#define PWM_THROTTLE_HIGH 2000
Servo servo_throttle;

// RPM sensor
#define RPM_NUMBER_CHANGES 5
#define RPM_TIMEOUT        1000000 // [us]
#define RPM_HIGH_THRESHOLD 300

// Pins definitions
int pin_throttle         = 9;
int pin_rpm              = A1;

// ------------------------------------------------------------------------------
//   SETUP
// ------------------------------------------------------------------------------

void setup() {
    
  // Run serial interface
  Serial.begin(38400); 
  
  // Startup
  Serial.println("SUAVE Motor Dyno");
  
  // Set analog pin reference voltage
  analogReference(DEFAULT);
  
}


// ------------------------------------------------------------------------------
//   LOOP
// ------------------------------------------------------------------------------

void loop() {

  // RPM Pin
  Serial.println( analogRead(pin_rpm) );

  // RPM
//  Serial.print("RPM: ");
//  Serial.println( measure_rpm() );

  delay(300);
  
}


// Measure RPM
unsigned measure_rpm(){

  int changes  = 0;
  unsigned rpm = 0;
  bool timeOut = false;

  // get current time
  long currtime = micros();                 
  
  while(changes < RPM_NUMBER_CHANGES && !timeOut){
    if((micros()-currtime)>RPM_TIMEOUT) 
      timeOut = true;
    delayMicroseconds(1);
    
    if(analogRead(pin_rpm) < RPM_HIGH_THRESHOLD){
      
      while(analogRead(pin_rpm) < RPM_HIGH_THRESHOLD && !timeOut){        
        if((micros()-currtime)>RPM_TIMEOUT) 
          timeOut = true;
        delayMicroseconds(1);
        
      }
      if(analogRead(pin_rpm) > RPM_HIGH_THRESHOLD){
        if(changes == 0) 
          currtime = micros();
        changes ++;
      }
    }
  }
  if (changes == RPM_NUMBER_CHANGES){
    rpm = (((changes-1)* 60 * 1000000)/(micros()-currtime));
  }
  return rpm;
    
}


