/** 
SUAVE Dyno Data Collection
Elias Wu and Trent Lukaczyk 
October 2015

Usage: 
Plug in Arduino or upload sketch, 
bring up serial monitor at 38400 baud, wait 8 seconds. 
Arm dyno, then press start test/emergency.
At any point, long press start test/emergency to terminate and power down motor.

To calibrate ESC,
hold the arming switch at startup,
then power your ESC, when it's ready
for the low PWM, push the arming switch again.

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

// High level settings
#define PWM_THROTTLE_STEPS 10   // number of pwm steps
#define TIME_DELAY_SETTLE  4000 // milliseconds to wait after each step
#define N_SENSOR_SAMPLES   5    // number of samples per step

// Scales
#define SCALE_THRUST_CALIBRATION +461.406f
#define SCALE_TORQUE_CALIBRATION -1461.036f
#define SCALE_THRUST_CONSTANT    0.4080f
#define SCALE_TORQUE_CONSTANT    0.1020f
#define SCALE_THRUST_CHANNEL     128
#define SCALE_TORQUE_CHANNEL     32
HX711 scale_thrust(A2,A3);
HX711 scale_torque(A2,A3);

// ADC for Battery Voltage and Current
#define VOLTAGE_CONSTANT 0.002955
#define CURRENT_CONSTANT 0.005184
Adafruit_ADS1115 adc_battery;
double Vcc;

// Servo driver for throttle
#define PWM_THROTTLE_LOW  1000
#define PWM_THROTTLE_HIGH 2000
Servo servo_throttle;

// RPM sensor
#define RPM_NUMBER_CHANGES 5
#define RPM_TIMEOUT        1000000 // [us]
#define RPM_HIGH_THRESHOLD 300

// State Flags
bool test_started = false;
bool armed        = false;

// Pins definitions
int pin_startstop        = 6;
int pin_arm              = 3;
int pin_startstop_status = 4;
int pin_arm_status       = 7;
int pin_throttle         = 9;
int pin_rpm              = A1;

int jnk = 0;

// ------------------------------------------------------------------------------
//   SETUP
// ------------------------------------------------------------------------------

void setup() {
    
  // Run serial interface
  Serial.begin(9600); 
  
  // Startup
  Serial.println("SUAVE Motor Dyno");
  Serial.print("Initializing ... ");
  
  // Set analog pin reference voltage
  analogReference(DEFAULT);
  
  // Initialize PWM driver
  servo_throttle.attach(pin_throttle);
  servo_throttle.writeMicroseconds(PWM_THROTTLE_LOW-50);
  
  // Initialize Pins
  pinMode(pin_startstop_status,OUTPUT);
  pinMode(pin_startstop,INPUT_PULLUP);
  pinMode(pin_arm,INPUT_PULLUP);
  pinMode(pin_arm_status,OUTPUT);

  // ADC Initialization
  adc_battery.begin();

  // Scale Initialization
  tare_scales();

  // Let it soak
  delay(1000);
    
  // Check for ESC Calibration
  if ( digitalRead(pin_arm) == 0 ) { 
    Serial.println("");
    calibrate_esc();
  }
  
  // Startup complete
  for(int i = 0; i < 5; i ++){ 
    digitalWrite(pin_startstop_status,1);
    delay(50);
    digitalWrite(pin_startstop_status,0);
    delay(50);
  }
  analogWrite(pin_startstop_status,0);
  Serial.println();
  Serial.println("Done!");
  
  // Here we go!
  Serial.println("Ready to Sample.");
  Serial.println("");
}


// ------------------------------------------------------------------------------
//   LOOP
// ------------------------------------------------------------------------------

void loop() {
  
  // Check if device is being armed
  if( digitalRead(pin_arm) == 0 ){ 
    if( !armed ){
      armed = true;
      servo_throttle.writeMicroseconds(PWM_THROTTLE_LOW);
      analogWrite(pin_arm_status,200);
    }
  }
  
  // Check if start button is pressed
  if( armed && digitalRead(pin_startstop) == 0 ){ 
    if( !test_started ){
      test_started = true;
      analogWrite(pin_startstop_status,200);
      Serial.println("Start!");
      delay(2000);
    }
  }
  
  
  // --------------------------------------------------------------------------
  //   Run The Test
  // --------------------------------------------------------------------------
  
  if( test_started ){
    
    // Tare the scales for each new test
    tare_scales();

    // Calibrate on VCC
    Vcc = read_vcc();
    
    // Run throttle setpoints
    for(int i_thr = 0; i_thr < (PWM_THROTTLE_STEPS+1); i_thr++){
      
      // Set this throttle setpoint
      int this_pwm_thrust = PWM_THROTTLE_LOW 
                          + ( i_thr * (PWM_THROTTLE_HIGH-PWM_THROTTLE_LOW)
                                    / PWM_THROTTLE_STEPS );
      servo_throttle.writeMicroseconds(this_pwm_thrust);
      
      // Allow time to settle
      if (i_thr > 0) {
        delay(TIME_DELAY_SETTLE);
      }
      
      // Run sensor readings
      for(int i_samp = 0; i_samp < N_SENSOR_SAMPLES; i_samp++){
        
        // Throttle
        Serial.print("Throttle: ");
        Serial.print( 100.0f * ((float)i_thr)/((float)PWM_THROTTLE_STEPS) );
        Serial.print(" %, ");
        Serial.print("PWM: ");
        Serial.print(this_pwm_thrust);
        Serial.print(" us, ");

        // Thrust
        Serial.print("Thrust: ");
        scale_thrust.set_gain(SCALE_THRUST_CHANNEL);
        Serial.print( scale_thrust.get_units(1) * SCALE_THRUST_CONSTANT );
        Serial.print(" g, ");
        
        // Torque
        Serial.print("Torque: ");
        scale_torque.set_gain(SCALE_TORQUE_CHANNEL);
        Serial.print( scale_torque.get_units(1) * SCALE_TORQUE_CONSTANT );
        Serial.print(" g-m, ");
        
        // Battery Voltage and Current
        Serial.print("Voltage: ");
        Serial.print( adc_battery.readADC_SingleEnded(0) * VOLTAGE_CONSTANT );
        Serial.print(" V, ");
        Serial.print("Current: ");
        Serial.print( adc_battery.readADC_SingleEnded(1) * CURRENT_CONSTANT );
        Serial.print(" A, ");
        
        // RPM
        Serial.print("RPM: ");
        Serial.print( measure_rpm() );
        
        // Next line
        Serial.println();
        
        // Flushhhh
        Serial.flush();
        
        // Emergency button press
        if( digitalRead(pin_startstop) == 0 || test_started == 0){
          Serial.println("EMERGENCY STOP!");
          test_started = 0;
          break;
        }
      }
      
      // Break out on emergency button press
      if ( !test_started ) {
        break;
      }
      
    }
    
    // Stop Motor
    servo_throttle.writeMicroseconds(PWM_THROTTLE_LOW-50);
    
    // LED Signal Stop
    for(int i = 0; i < 5; i ++){ 
      digitalWrite(pin_startstop_status,1);
      delay(50);
      digitalWrite(pin_startstop_status,0);
      delay(50);
    }
    analogWrite(pin_startstop_status,0);
    Serial.println();
    
    // Message for successful test completion
    if ( test_started ) {
      Serial.println("Test completed.");
      Serial.println();
      test_started = 0;
    }
    
    // Clean up and done
    Serial.flush();
    delay(3000);
    
  }
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


//ESC Calibration
void calibrate_esc() {  

  //Write ESC high for calibration
  servo_throttle.writeMicroseconds(PWM_THROTTLE_HIGH); 
  
  //Notify user that calibration has begun
  Serial.print("Calibrate ESC ... ");
  for(int i = 0; i < 3; i ++){ 
    digitalWrite(pin_startstop_status,1);
    delay(100);
    digitalWrite(pin_startstop_status,0);
    delay(100);
  }
  
  // Wait for user to release then press arm switch again
  while( digitalRead(pin_arm) == 0 )
    delay(10);
  while( digitalRead(pin_arm) == 1 )
    delay(10);
  
  servo_throttle.writeMicroseconds(PWM_THROTTLE_LOW);
  
  //Notify user that calibration is done
  Serial.println("Done!");
  for(int i = 0; i < 3; i ++){ 
    digitalWrite(pin_startstop_status,1);
    delay(100);
    digitalWrite(pin_startstop_status,0);
    delay(100);
  }
  
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


// ADC Calibration
long read_vcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}
