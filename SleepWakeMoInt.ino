/*
 * Author: Chet Udell
 * CC License 3.0, share-alike, attribution
 * 
 * Use DS3231 RTC Adafruit breakout and RTCLibExtended for power management
 * Because Adafruit ProTrinket only has Pin 3 onterrupt available, we need to be able to wake from sleep 
 * this from an additional INT pin 
 * 
 * For Use with Feather MO
 * 
 * Use Sparkfun Low Power library
 * Sleep until Interrupt
 * Use Digital pin 11 as Interrupt
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

#define LEDPIN    13

// **** INCLUDES *****
#include "LowPower.h"

volatile bool TakeSampleFlag = false; // Flag is set with external Pin A0 Interrupt by RTC
volatile bool LEDState = false; // flag t toggle LED

const byte INT_PIN = 11;


void setup()
{
  pinMode(LEDPIN, OUTPUT); // LED 13
  Serial.begin(9600);
  Serial.println("RTC Sleep Wake Pin Interrupt Test");
  InitializeIO();
  attachInterrupt(INT_PIN, wake, FALLING);
  delay(5000);
}

void loop() {
   
    attachInterrupt(digitalPinToInterrupt(INT_PIN), wake, FALLING);  
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    LowPower.idle(IDLE_2); 
    // <----  Wait in sleep here until pin interrupt
    
    // On Wakeup, proceed from here:
    detachInterrupt(digitalPinToInterrupt(INT_PIN)); //
    
    if(TakeSampleFlag)
    {
      // Stuff I'm doing on wake
      Blink(LEDPIN,100,3);
      Serial.println("I took a sample!");
      delay(50); //wait for serial to print
      digitalWrite(13, LEDState);
      delay(75);  // delay so serial stuff has time to print out all the way
      TakeSampleFlag = false; // Clear Sample Flag
    }
    
}  


void InitializeIO(){
  pinMode(INT_PIN, INPUT_PULLUP);
}

void wake()
{
  TakeSampleFlag = true;
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


