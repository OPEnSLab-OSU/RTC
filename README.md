# RTC
Real Time Clock alarms and interrupts and sleep examples
### RTC and Low Power Functionality

The OPEnS Lab currently supports sleep functionality for both the Adafruit Feather M0 and
the Adafruit Feather 32u4.  The RTC used to wake both the M0 and the 32u4 is the 
[Adafruit DS3231 Precision RTC Breakout](https://learn.adafruit.com/adafruit-ds3231-precision-rtc-breakout/).

#### RTC and Low Power Dependencies

* [DS3231 Extended Library](https://github.com/FabioCuomo/FabioCuomo-DS3231)
* [Low Power Library](https://github.com/rocketscream/Low-Power)
* [Enable Interrupt](https://github.com/GreyGnome/EnableInterrupt)

**NOTE:** To use the DS3231 extended library with the Feather M0,
the following line must be added to `RTClibExtended.h`:

``` cpp
#define _BV(bit) (1 << (bit))
```

#### Sleep Modes

Project Loom supports two sleep modes for the Feather M0 and one sleep mode for the Feather 32u4.
Here are some details on the various modes:

| Mode           | Supported board      | Current Draw           |
| -------------- | -------------------- | ---------------------- |
| Idle\_2        | Feather M0           | ~5 mA                  |
| Standby        | Feather M0           | ~0.7 mA                |
| SLEEP\_FOREVER | Feather 32U4         | Untested               |

#### Standby Operation

Due to some incompatibilities between Standby mode and falling interrupts, a very particular
scheme must be followed to use Standby mode on the Feather M0.  The following code is an
example of how standby mode can be set up on the M0 with a wakeup interrupt on pin 11:

``` cpp
void setup() {
    pinMode(11, INPUT_PULLUP);
    bool OperationFlag = false;
    delay(10000); //It's important to leave a delay so the board can more easily
                  //be reprogrammed
}

void loop() {
    if (OperationFlag) {

        // Whatever you want the board to do while awake goes here

        OperationFlag = false; //reset the flag
    }

    attachInterrupt(digitalPinToInterrupt(11), wake, LOW);

    LowPower.standby();
}

void wake() {
    OperationFlag = true;
    detachInterrupt(digitalPinToInterrupt(11)); //detach the interrupt in the ISR so that
                                                //multiple ISRs are not called
}
```
