/** 
SuaveDyno Data Collection
Elias Wu and Trent Lukaczyk 2015

Usage: 
Plug in Arduino or upload sketch, bring up serial monitor at 38400 baud, wait 8 seconds, plug in ESC. Arm dyno, then press start test/emergency.
At any point, long press start test/emergency to terminate and power down motor.

In order to calibrate ESC, plug in Arduino or upload sketch, wait until start/emergency light flashes, plug in ESC, let ESC calibrate, then unplug and replug ESC.
**/

#include <HX711.h>
#include <Servo.h>

//Constants for APM Power Module
#define VOLTAGE_CONSTANT 0.04653
#define CURRENT_CONSTANT 0.06336
// HX711.DOUT	- pin #A2
// HX711.PD_SCK	- pin #A3

HX711 scale(A2,A3);
HX711 scale2(A2,A3);

Servo myservo;

//Arducopter Power Module
int VRaw; 
int IRaw;
float VFinal; 
float IFinal;

//Status LED
int startEmergencyLed = 4;
int armLed = 7;

//booleans
int testStarted = 0;
int armed = 0;

//Pins definitions
int startEmergencyPin = 6;
int armPin = 3;
int rpmPin = 2;
int vPin = A0;
int iPin = A5;

void setup() {
  Serial.begin(38400); //Run serial
  Serial.println("HX711 Demo");
  
  myservo.attach(9); //Initialize PWM on pin 9
  delay(3000);
  
  //Initialize Pins
  pinMode(startEmergencyLed,OUTPUT);
  pinMode(startEmergencyPin,INPUT_PULLUP);
  pinMode(armPin,INPUT_PULLUP);
  pinMode(armLed,OUTPUT);
  pinMode(rpmPin,INPUT);
  
  //ESC Calibration
  myservo.writeMicroseconds(2000); //Write ESC high for calibration
  for(int i = 0; i < 3; i ++){ //Notify user that calibration has begun
    digitalWrite(startEmergencyLed,1);
    delay(100);
    digitalWrite(startEmergencyLed,0);
    delay(100);
  }
  delay(2000);
  myservo.writeMicroseconds(950);
  digitalWrite(startEmergencyLed,1); //Notify user that calibration has finished
  delay(1000);
  digitalWrite(startEmergencyLed,0);
  delay(1500);
  
  //HX711 Initialization
  scale.set_gain(128);
  scale.set_scale(-409.08f); //This value needs to be calibrated.
  scale.tare();	
  scale2.set_gain(32);
  scale2.set_scale(1042.36f); //This value needs to be calibrated
  scale2.tare();

  Serial.println("Readings:");
}

void loop() {
  if(digitalRead(armPin) == 0){ //check if device is being armed
    if(armed == 0){
      armed = 1;
      myservo.writeMicroseconds(950);
      analogWrite(armLed,200);
    }
  }
  if(digitalRead(startEmergencyPin) == 0&&armed == 1){ //check if start button is pressed
    if(testStarted==0){
      testStarted = 1;
      analogWrite(startEmergencyLed,200);
      delay(2000);
    }
  }
  if(testStarted == 1){
    //Scales need to be re-initialized every loop in order to read the different load cells.
    scale.set_gain(128);
    scale.set_scale(-419.66f);
    scale.tare();	
    scale2.set_gain(32);
    scale2.set_scale(-1256.36f);
    scale2.tare();
    
    //Run 10 throttle settings
    for(int i = 0; i < 11; i++){
      int thrustValue = 950+(i*100);
      myservo.writeMicroseconds(thrustValue);
      delay(1000);
      
      //Run 5 sensor readings
      for(int j = 0; j < 5; j++){
        
        //Read voltage and current
        VRaw = analogRead(vPin);
        IRaw = analogRead(iPin);
        VFinal = VRaw*VOLTAGE_CONSTANT; 
        IFinal = (IRaw*CURRENT_CONSTANT); 
        
        //Set up to read thrust cell
        scale.set_gain(128);
        Serial.print("Thrust: ");
        Serial.print(scale.get_units(1)/2.4525);
        Serial.print("g,");
        Serial.print(" Torque: ");
        
        //Set up to read torque cell
        scale2.set_gain(32);
        Serial.print(scale2.get_units(1)*0.1019);
        Serial.print("g-m,");
        
        //Voltage and current
        Serial.print(" Voltage: ");
        Serial.print(VFinal);
        Serial.print("V,");
        Serial.print(" Current: ");
        Serial.print(IFinal);
        Serial.print("A,");
        
        //Control values
        Serial.print(" PWM: ");
        Serial.print(thrustValue);
        Serial.print(",");
        Serial.print(" Throttle: ");
        Serial.print(((thrustValue-950)/1000.0)*100);
        Serial.print("%,");
        Serial.print(" RPM: ");
        Serial.println(measureRPM());
        
        //Emergency button press
        if(digitalRead(startEmergencyPin) == 0||testStarted == 0){
          myservo.writeMicroseconds(950);
          for(int i = 0; i < 5; i ++){ //Notify user that emergency stop triggered
            digitalWrite(startEmergencyLed,1);
            delay(50);
            digitalWrite(startEmergencyLed,0);
            delay(50);
          }
          testStarted = 0;
          break;
        }
      }
      if(digitalRead(startEmergencyPin) == 0||testStarted == 0){
        myservo.writeMicroseconds(950);
        for(int i = 0; i < 5; i ++){ //Notify user that emergency stop triggered
          digitalWrite(startEmergencyLed,1);
          delay(50);
          digitalWrite(startEmergencyLed,0);
          delay(50);
        }
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

//Blocking measure RPM using EagleTree optical RPM sensor
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
