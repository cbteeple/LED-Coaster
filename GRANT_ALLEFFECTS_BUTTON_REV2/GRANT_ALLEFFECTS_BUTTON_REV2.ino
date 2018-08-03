#include <EEPROM.h>
#include <EEPROMAnything.h>

// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <Adafruit_NeoPixel.h>
#include "PinChangeInterrupt.h"


#define BUTTON_PIN 2    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.

#define PIXEL_PIN    0    // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 14

const uint8_t BRIGHTNESS_DATA_START = 0;
const uint8_t SHOW_DATA_START = 4;

const int numBrightLevels = 8;
const float BRIGHT_LEVELS[numBrightLevels]={0.04,0.08,0.12,0.18,0.24,0.32,0.40,0.45};
uint8_t BrightnessIDX=0;

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

bool oldState = HIGH;
int showType = 0;
int numShows = 12;
bool switchShows = false;
float Brightness = BRIGHT_LEVELS[0];
bool requiresLoop=0;
bool newButton=false;



unsigned long timeBetweenPresses = 10000;  // the last time the output pin was toggled
unsigned long timeHeld =0;
unsigned long lastPressTime = 0;    // the debounce time; increase if the output flickers
unsigned long brightPressTime=1000;
unsigned long brightInitTime=250;
unsigned long brightEndTime=2500;
unsigned long startBrightCurr=0;

bool setBright = false;
bool firstcall=true;


void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIXEL_PIN, OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  attachPCINT(digitalPinToPCINT(BUTTON_PIN), debounce, CHANGE);
  
}

void loop() {
  if(firstcall){
    readSettings();
    firstcall=false;
  }
  else{
    if (setBright){
      startBlink(150);
      startBlink(150);
      showColor(128,128,128);
      while ((millis()-startBrightCurr)<brightEndTime){
        showColor(128,128,128);
      }
      setBright=false;
      }
    else{
      switchShows=false;
      startShow();
    }
  }
}

void startShow() {
  switch (showType){
      case 0: {showColorAnimate(255, 46, 0,30);
          requiresLoop=0;
        } //Red
        break;
      case 1:{showColorAnimate(255, 255, 0,30);
          requiresLoop=0;
        }//Yellow
        break;
      case 2:{showColorAnimate(0, 255, 0,30);
          requiresLoop=0;
        }//Green
        break;
      case 3:{ showColorAnimate(0, 30, 255,30); //Blue
          requiresLoop=0;
        }
        break;
      case 4:{ showColorAnimate(120, 0, 255,30); //Purple
          requiresLoop=0;
        }
        break;
      case 5:{ showColorAnimate(255, 0, 255,30); //Pink
          requiresLoop=0;
        }
        break;
      case 6: {showRainbow(30); //Rainbow
        requiresLoop=0;
        }
        break;
        //Michigan
      case 7: halfAndHalf(strip.Color(255*Brightness,255*Brightness,0),strip.Color(0,31*Brightness,173*Brightness),30);
        break;
        //Michigan State
      case 8: halfAndHalf(strip.Color(255*Brightness,255*Brightness,255*Brightness),strip.Color(50*Brightness,200*Brightness,100*Brightness),30);
        break;
      case 9: rainbowCycleDim(1,1);
        break;
      case 10: rainbowCycleDim(5,1);
        break;
      case 11: rainbowCycleDim(20,1);
        break;
  }
}


//================================

// Same as RainbowCycle, except Dimmer
void rainbowCycleDim(uint8_t wait, uint8_t reps) {
  uint16_t i, j;

  for(j=0; j<256*reps; j++) { // 5 cycles of all colors on wheel
    if (setBright | switchShows){
      break;
    }
    
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}




void halfAndHalf(uint32_t color1,uint32_t color2,uint16_t wait){
  uint16_t i;
    for(i=0; i< strip.numPixels(); i++) {
      if (i<strip.numPixels()/2){
        strip.setPixelColor(i, color1);
      }
      else if ((i>=strip.numPixels()/2)){
        strip.setPixelColor(i, color2);
      } 
      strip.show(); 
      delay(wait);   
    }
}




//================================
//HELPER FUNCTIONS

void showColor(uint8_t red,uint8_t green,uint8_t blue){
  for(int i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(red*Brightness,green*Brightness,blue*Brightness));
  }
    strip.show();  
}


void showColorAnimate(uint8_t red,uint8_t green,uint8_t blue,uint16_t wait){
  for(int i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(red*Brightness,green*Brightness,blue*Brightness));
      strip.show(); 
      delay(wait);
  }
     
}


void showRainbow(uint16_t wait){
  uint16_t j=0;
  uint16_t i=0;
  for(i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    strip.show(); 
    delay(wait); 
  }
}


void startBlink(uint16_t wait){
  showColor(128,128,128);
  delay(wait);
  showColor(0,0,0);
  delay(wait);
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color((255 - WheelPos * 3)*Brightness, 0, (WheelPos * 3)*Brightness);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, (WheelPos * 3)*Brightness, (255 - WheelPos * 3)*Brightness);
  }
  WheelPos -= 170;
  return strip.Color((WheelPos * 3)*Brightness, (255 - WheelPos * 3)*Brightness, 0);
}



//ISR
boolean debounce(){
cli();
delay(20);
int reading = digitalRead(BUTTON_PIN);

if (reading==LOW){ 
  timeBetweenPresses=millis()-lastPressTime;
  lastPressTime=millis();
  
    while (digitalRead(BUTTON_PIN)==LOW & setBright==false){
      timeHeld=millis()-lastPressTime;
      if (timeHeld>brightPressTime){
        setBright=true;
        break;
      }
      delay(5);
    }
  
  if (setBright){
    incrementBrightness();
    startBrightCurr=millis();
    }
  else{
      showType++;
      if (showType >= numShows){
        showType=0;
      }
      switchShows=true;
    }
    saveSettings();
    sei();
  }
}






void incrementBrightness(){
//Update the brightness
  BrightnessIDX++;
  if (BrightnessIDX>=numBrightLevels){
    BrightnessIDX=0;
  }
  Brightness=BRIGHT_LEVELS[BrightnessIDX];

}


void saveSettings(){
  //EEPROM_writeAnything(BRIGHTNESS_DATA_START, Brightness);
  EEPROM.write(SHOW_DATA_START, showType);
  EEPROM.write(BRIGHTNESS_DATA_START, BrightnessIDX);
  //EEPROM_writeAnything(, showType);
}

void readSettings(){
    //Read old settings from EEPROM
  //EEPROM_readAnything(BRIGHTNESS_DATA_START, Brightness);
  uint8_t tmpShow= EEPROM.read(SHOW_DATA_START);
  uint8_t tmpBright=EEPROM.read(BRIGHTNESS_DATA_START);

  
  //If show number is unreasonable, go back to default
  if (tmpShow<numShows & tmpShow>=0){
    showType = tmpShow;
  }
  if(tmpBright<numBrightLevels & tmpBright>=0){
    BrightnessIDX = tmpBright;
    Brightness=BRIGHT_LEVELS[BrightnessIDX];
  }
}
