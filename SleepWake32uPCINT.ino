/*
 * Author: Chet Udell
 * CC License 3.0, share-alike, attribution
 * 
 * For Use with Feather 32u4
 * 
 * wake from sleep 
 * from a PCINT pin 
 * 
 * Use Sparkfun Low Power library
 * Sleep until Interrupt
 * Use 11 pin as Interrupt
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

// **** INCLUDES *****
#include "LowPower.h"

#define LEDPIN    13

volatile bool TakeSampleFlag = false; // Flag is set with external Pin A0 Interrupt by RTC
volatile bool LEDState = false; // flag t toggle LED

volatile byte PCINT_PIN = 11; // Pin to attach SQW from RTC

void setup()
{
  pinMode(LEDPIN, OUTPUT); // LED 13
  Serial.begin(9600);
  delay(5000);
  Serial.println("Sleep wake from PCINT as interrupt test");
  pinMode(PCINT_PIN, INPUT_PULLUP);     // Pin P12, PCINT7 is input, pullup
}

void loop() {
   
    // enable interrupt for PCINT11...
     pciSetup(PCINT_PIN);
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
     LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    // <----  Wait in sleep here until pin interrupt
    
    
    
    if(TakeSampleFlag)
    {
      // On Wakeup, proceed from here:
      PCICR =0x00;          // Disable PCINT1 interrupt
      //We resume here after the Interrupt
      Blink(LEDPIN,100,3);
      Serial.println("I took a sample!");
      delay(50); //wait for serial to print
      digitalWrite(13, LEDState);
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

