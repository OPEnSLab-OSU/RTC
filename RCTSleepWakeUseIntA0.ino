/*
 * Author: Chet Udell
 * CC License 3.0, share-alike, attribution
 * 
 * Use DS3231 RTC Adafruit breakout and RTCLibExtended for power management
 * Because Adafruit ProTrinket only has Pin 3 onterrupt available, we need to be able to wake from sleep 
 * this from an additional INT pin 
 * 
 * Use Sparkfun Low Power library
 * Sleep until Interrupt
 * Use Analog 0 pin as Interrupt
 * 
 * You may need to download RTCLibExtended:
 * https://github.com/FabioCuomo/FabioCuomo-DS3231
 * 
 * And Sparkfun Low Power Library:
 * https://github.com/rocketscream/Low-Power/archive/master.zip
 * 
 * Instructions on how to install library:
 * https://learn.sparkfun.com/tutorials/installing-an-arduino-library
 * 
 * RTC and RTCLib-extended tutorial/reference:
 * http://www.instructables.com/id/Arduino-Sleep-and-Wakeup-Test-With-DS3231-RTC/
 * 
 */

// **** INCLUDES *****
//#include "LowPower.h"
#include <Wire.h>
#include <RTClibExtended.h>

// Create instance of DS3231 called RTC
RTC_DS3231 RTC; //we are using the DS3231 RTC

volatile bool TakeSampleFlag = false; // Flag is set with external Pin A0 Interrupt by RTC
volatile bool LEDState = false; // flag t toggle LED
volatile int HR = 8; // Hr of the day we want alarm to go off
volatile int MIN = 0; // Min of each hour we want alarm to go off
volatile int WakePeriodMin = 1;  // Period of time to take sample in Min, reset alarm based on this period

int sensorPin = A1;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup()
{
  pinMode(13, OUTPUT); // LED 13
  Serial.begin(9600);
  Serial.println("Sleep wake from Analog Pin as interrupt test");
  InitializeIO();
  //InitializeInterrupt();
  InitalizeRTC();
  Serial.print("Alarm set to go off every "); Serial.print(WakePeriodMin); Serial.println("min from program time");
  delay(1000);
}

void loop() {
   
    attachInterrupt(digitalPinToInterrupt(0), wake, FALLING); 
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    //LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    // <----  Wait in sleep here until pin interrupt
    
    // On Wakeup, proceed from here:
    detachInterrupt(digitalPinToInterrupt(0));           // Disable interrupt
    clearAlarmFunction(); // Clear RTC Alarm
    
    if(TakeSampleFlag)
    {
      // Stuff I'm doing personally
      // read the value from the sensor:
      sensorValue = analogRead(sensorPin);
      Serial.println("I took a sample!, Analog 01 pin is");
      Serial.println(sensorValue);
      digitalWrite(13, LEDState);

      // Stuff that NEEDS to happen at the end
      // Reset alarm1 for next period
      setAlarmFunction();
      delay(75);  // delay so serial stuff has time to print out all the way
      TakeSampleFlag = false; // Clear Sample Flag
    }
    delay(2000);
}  


void InitializeIO(){
  pinMode(0, INPUT_PULLUP);
}

void InitializeInterrupt(){
  attachInterrupt(digitalPinToInterrupt(0), wake, FALLING); 
}

void wake()
{
  
}

void InitalizeRTC()
{
  // RTC Timer settings here
  if (! RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
// This may end up causing a problem in practice - what if RTC looses power in field? Shouldn't happen with coin cell batt backup
  if (RTC.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
  // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //clear any pending alarms
  clearAlarmFunction();

  // Querry Time and print
  DateTime now = RTC.now();
  Serial.print("RTC Time is: ");
  Serial.print(now.hour(), DEC); Serial.print(':'); Serial.print(now.minute(), DEC); Serial.print(':'); Serial.print(now.second(), DEC); Serial.println();

  //Set SQW pin to OFF (in my case it was set by default to 1Hz)
  //The output of the DS3231 INT pin is connected to this pin
  //It must be connected to arduino Interrupt pin for wake-up
  RTC.writeSqwPinMode(DS3231_OFF);

  //Set alarm1
  setAlarmFunction();
}

// *********
// RTC helper function
// Function to query current RTC time and add the period to set next alarm cycle
// *********
void setAlarmFunction()
{
  DateTime now = RTC.now(); // Check the current time

  // Calculate new time
  MIN = (now.minute()+WakePeriodMin)%60;  // wrap-around using modulo every 60 sec
  HR  = (now.hour()+((now.minute()+WakePeriodMin)/60))%24; // quotient of now.min+periodMin added to now.hr, wraparound every 24hrs

  Serial.print("Resetting Alarm 1 for: "); Serial.print(HR); Serial.print(":");Serial.println(MIN);
  

  //Set alarm1
  RTC.setAlarm(ALM1_MATCH_HOURS, MIN, HR, 0);   //set your wake-up time here
  RTC.alarmInterrupt(1, true);
  
}

//*********
// RTC helper function
// When exiting the sleep mode we clear the alarm
//*********
void clearAlarmFunction()
{
  //clear any pending alarms
  RTC.armAlarm(1, false);
  RTC.clearAlarm(1);
  RTC.alarmInterrupt(1, false);
  RTC.armAlarm(2, false);
  RTC.clearAlarm(2);
  RTC.alarmInterrupt(2, false);
}

//******************
// A5:0 Interrupt service routine. Every single PCINT8..14 (=ADC0..5) change
// will generate an interrupt: but this will always be the same interrupt routine
// Here, you would have to check which pin has changed to your desired state
// Only using A0
// Interrupt checks to see if A0 is pulled low (RTC SQW pin asserted)
// If so, enable TakeSampleFlag, which authorizes sample being taken in main LOOP
ISR(PCINT1_vect) {    
  if (digitalRead(A0)==0)  
  {
    Serial.println("A0 is Low");
    TakeSampleFlag = true; // set sample flag
    LEDState = !LEDState; // flip state
  }
  if (digitalRead(A0)==1)
  {
    // Do nothing if pulled back high, or redundantly disable SampleFlag
  }
  
}


