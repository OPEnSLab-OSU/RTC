/*
 * Author: Chet Udell, Thomas DeBell
 * CC License 3.0, share-alike, attribution
 * 
 * For Use with Feather 32u4 and DS3231 RTC
 * 
 * Use DS3231 RTC Adafruit breakout and RTCLibExtended for power management
 * 
 * Use Sparkfun Low Power library
 * Sleep until Interrupt
 * Use digital pin 11 as Interrupt
 * 
 * You may need to download OPEnSRTCLibExtended:
 * (replace this with a link to our OPEnSRTCLibExtended)
 * https . . . .
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
 * PINOUTS
 * 
 * Feather P11 <= RTC SQW
 * Feather SCL => RTC SCL
 * Feather SDA => RTC SDA
 * 
 */

#define RTC3231

#define AlarmType 1   // set to 0 for daily alarm at same time each day
                      // set to 1 for periodic alarm (every x Min)
#define LEDPIN    13


// **** INCLUDES *****
#include "LowPower.h"
#include <Wire.h>
#include <RTClibExtended.h>

#ifdef RTC3231
// Create instance of DS3231 called RTC
RTC_DS3231 myRtc; //we are using the DS3231 RTC
#endif

volatile bool TakeSampleFlag = false; // Flag is set with external Pin A0 Interrupt by RTC
volatile bool LEDState = false; // flag t toggle LED
volatile int HR = 15; // Hr of the day we want alarm to go off
volatile int MIN = 48; // Min of each hour we want alarm to go off
volatile int WakePeriodMin = 1;  // Period of time to take sample in Min, reset alarm based on this period

volatile byte PCINT_PIN = 11; // Pin to attach SQW from RTC

void setup()
{
  pinMode(LEDPIN, OUTPUT); // LED 13
  Serial.begin(9600);
  delay(10000);
  Serial.println("Sleep wake from PCINT as interrupt test");
  pinMode(PCINT_PIN, INPUT_PULLUP);     // Pin P12, PCINT7 is input, pullup
  InitalizeRTC();
//  Serial.print("Alarm set to go off every "); Serial.print(WakePeriodMin); Serial.println("min from program time");
}

void loop() {
   
    // enable interrupt for PCINT7...
     pciSetup(PCINT_PIN);
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
     LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    // <----  Wait in sleep here until pin interrupt
    
    
    
    if(TakeSampleFlag)
    {
      // On Wakeup, proceed from here:
      PCICR =0x00;          // Disable PCINT1 interrupt
      clearAlarmFunction(); // Clear RTC Alarm
      //We resume here after the Interrupt
      Blink(LEDPIN,100,3);
      Serial.println("I took a sample!");
      delay(50); //wait for serial to print
      digitalWrite(13, LEDState);

      // Reset alarm1 for next period
      setAlarmFunction();
      TakeSampleFlag = false; // Clear Sample Flag
    }
    
}  


void Blink(byte ledPin, int msDelay, byte loops)
{
  for (byte i=0; i<loops; i++)
  {
    digitalWrite(ledPin,HIGH);
    delay(msDelay);
    digitalWrite(ledPin,LOW);
    delay(msDelay);
  }
}



//******************
// Function to init PCI interrupt pin
     
void pciSetup(byte pin)
 {
        *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
        PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
        PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
 }
     
    // Use one Routine to handle each group
     
ISR (PCINT0_vect) // handle pin change interrupt for D8 to D13 here
  {    
         if(digitalRead(PCINT_PIN) == LOW)
          TakeSampleFlag = true;
  }

void InitalizeRTC()
{
  // RTC Timer settings here
  if (! myRtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
// This may end up causing a problem in practice - what if RTC looses power in field? Shouldn't happen with coin cell batt backup
  if (myRtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
  // following line sets the RTC to the date & time this sketch was compiled
    myRtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //clear any pending alarms
  clearAlarmFunction();

  // Querry Time and print
  DateTime now = myRtc.now();
  Serial.print("RTC Time is: ");
  Serial.print(now.hour(), DEC); Serial.print(':'); Serial.print(now.minute(), DEC); Serial.print(':'); Serial.print(now.second(), DEC); Serial.println();

  //Set SQW pin to OFF (in my case it was set by default to 1Hz)
  //The output of the DS3231 INT pin is connected to this pin
  //It must be connected to arduino Interrupt pin for wake-up
  myRtc.writeSqwPinMode(DS3231_OFF);

  //Set alarm1
  setAlarmFunction();
}

// *********
// RTC helper function
// Function to query current RTC time and add the period to set next alarm cycle
// *********
void setAlarmFunction()
{
if(AlarmType == 0)
  {
    Serial.print(F("the current sample Alarm set for Daily at"));
  }
else
  {
    DateTime now = myRtc.now(); // Check the current time
    // Calculate new time
    MIN = (now.minute()+WakePeriodMin)%60;  // wrap-around using modulo every 60 sec
    HR  = (now.hour()+((now.minute()+WakePeriodMin)/60))%24; // quotient of now.min+periodMin added to now.hr, wraparound every 24hrs

    Serial.print("Resetting Periodic Alarm for: ");
  }
  // Set Alarm
  myRtc.setAlarm(ALM1_MATCH_HOURS, MIN, HR, 0);   //set your wake-up time here
  myRtc.alarmInterrupt(1, true);
  Serial.print(HR); Serial.print(F(":")); Serial.println(MIN);
}

//*********
// RTC helper function
// When exiting the sleep mode we clear the alarm
//*********
void clearAlarmFunction()
{
  //clear any pending alarms
  myRtc.armAlarm(1, false);
  myRtc.clearAlarm(1);
  myRtc.alarmInterrupt(1, false);
  myRtc.armAlarm(2, false);
  myRtc.clearAlarm(2);
  myRtc.alarmInterrupt(2, false);
}


