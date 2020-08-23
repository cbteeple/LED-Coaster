#include <EEPROM.h>
#include <EEPROMAnything.h>

// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <Adafruit_NeoPixel.h>
//#include "PinChangeInterrupt.h"


const uint8_t BRIGHTNESS_DATA_START = 0;
const uint8_t SHOW_DATA_START = 4;

const int numBrightLevels = 10;
uint8_t BrightnessIDX=0;


int showType = 1;
int numShows = 18;


void setup() {
  Serial.begin(115200);
  saveSettings();

  Serial.println("default settings saved");
  
}

void loop() {
  
  
}





void saveSettings(){
  //EEPROM_writeAnything(BRIGHTNESS_DATA_START, Brightness);
  EEPROM.write(SHOW_DATA_START, showType);
  EEPROM.write(BRIGHTNESS_DATA_START, BrightnessIDX);
  //EEPROM_writeAnything(, showType);
}
