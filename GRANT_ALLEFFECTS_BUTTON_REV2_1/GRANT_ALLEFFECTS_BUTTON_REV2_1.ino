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
int showType = 1;
int numShows = 18;
bool switchShows = false;
float Brightness = BRIGHT_LEVELS[3];
bool requiresLoop=0;
bool newButton=false;



unsigned long timeBetweenPresses = 10000;  // the last time the output pin was toggled
unsigned long timeHeld =0;
unsigned long lastPressTime = 0;    // the debounce time; increase if the output flickers
unsigned long brightPressTime=1000;
unsigned long brightInitTime=250;
unsigned long brightEndTime=1500;
unsigned long startBrightCurr=0;

bool setBright = false;
bool firstcall=true;


void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIXEL_PIN, OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  #ifdef DEBUG
    Serial.begin(115200);
  #endif
  
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
      case 1: {genPulse(255, 46, 0,20);
          requiresLoop=0;
        } //Red
        break;
      case 2:{showColorAnimate(255, 255, 0,30);
          requiresLoop=0;
        }//Yellow
        break;
      case 3: {genPulse(255, 255, 0,20);
          requiresLoop=0;
        } //Red
        break;
      case 4:{showColorAnimate(0, 255, 0,30);
          requiresLoop=0;
        }//Green
        break;
      case 5: {genPulse(0, 255, 0,20);
          requiresLoop=0;
        } //Red
        break;
      case 6:{ showColorAnimate(0, 30, 255,30); //Blue
          requiresLoop=0;
        }
        break;
      case 7: {genPulse(0, 30, 255,20);
          requiresLoop=0;
        } //Red
        break;
      case 8:{ showColorAnimate(120, 0, 255,30); //Purple
          requiresLoop=0;
        }
        break;
      case 9: {genPulse(120, 0, 255,20);
          requiresLoop=0;
        } //Red
        break;
      case 10:{ showColorAnimate(255, 0, 255,30); //Pink
          requiresLoop=0;
        }
        break;
      case 11: {genPulse(255, 0, 255,20);
          requiresLoop=0;
        } //Red
        break;
      case 12: {showRainbow(30); //Rainbow
        requiresLoop=0;
        }
        break;
        //Michigan
      case 14: halfAndHalfAnimated(255,255,0,0,31,173,20);
        break;
        //Michigan State
      case 13: halfAndHalfAnimated(255,255,255,50,200,100,20);
        break;
      case 15: rainbowCycleDim(40,1,true);
        break;
      case 16: rainbowCycleDim(5,1,false);
        break;
      case 17: rainbowCycleDim(20,1,false);
        break;
  }
}


//================================

// Same as RainbowCycle, except Dimmer
void rainbowCycleDim(uint8_t wait, uint8_t reps, bool FullColor) {
  uint16_t i, j;

  for(j=0; j<256*reps; j++) { // 5 cycles of all colors on wheel
    if (setBright || switchShows){
      break ;
    }
    
    for(i=0; i< strip.numPixels(); i++) {
      if (FullColor){
        strip.setPixelColor(i, Wheel(j & 255));
        }
      else{
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      }
    }
    strip.show();
    delay(wait);
  }
}




void halfAndHalf(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2,uint16_t wait){
  uint16_t i;
    for(i=0; i< strip.numPixels(); i++) {
      if (i<strip.numPixels()/2){
        strip.setPixelColor(i, strip.Color(r1*Brightness,g1*Brightness,b1*Brightness));
      }
      else if ((i>=strip.numPixels()/2)){
        strip.setPixelColor(i, strip.Color(r2*Brightness,g2*Brightness,b2*Brightness));
      } 
      strip.show(); 
      delay(wait);   
    }
}





void halfAndHalfAnimated(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2,uint16_t wait){
  uint16_t i, j, k;
    //Set inital configuration
    halfAndHalf(r1, g1, b1, r2, g2, b2, 30);
    
    //Slowly rotate   
    for (k=0; k<strip.numPixels(); k++){
      for (j=0; j<128; j++){
        if (setBright || switchShows){
          return;
        }
        float ratio=float(j/128.0);
        strip.setPixelColor((k+strip.numPixels()/2)%(strip.numPixels()), strip.Color(((ratio*r1)+((1.0-ratio)*r2))*Brightness,((ratio*g1)+((1.0-ratio)*g2))*Brightness,((ratio*b1)+((1.0-ratio)*b2))*Brightness));
        strip.setPixelColor(k, strip.Color(((ratio*r2)+((1.0-ratio)*r1))*Brightness,((ratio*g2)+((1.0-ratio)*g1))*Brightness,((ratio*b2)+((1.0-ratio)*b1))*Brightness));
        strip.show(); 
        delay(wait);   
      }
    }
    
}




void genPulse(uint8_t red, uint8_t green,uint8_t blue,uint16_t wait){
  
  bool flip = true;
  float bright1=Brightness;
  float bright2=Brightness/3;
  float bright_tmp = bright1;
  int maxSteps=127;
  float progress=0.0;

  showColorAnimate(red/float(3), green/float(3), blue/float(3),30);

  for (uint8_t k=0;k<2;k++){
    for (uint16_t i=0; i<maxSteps+1; i++){
      if (setBright || switchShows){
        break;
      }      
      progress=BezierBlend(float(i)/float(maxSteps));
       
      //Calculate the brightness to use;
      if (flip){
        bright_tmp=(progress)*bright1+(1-progress)*bright2;
      }
      else{
        bright_tmp=(progress)*bright2+(1-progress)*bright1;
      }
  
      //Apply that brightness to all pixels
      for (uint8_t j=0; j<strip.numPixels(); j++){
          strip.setPixelColor(j, strip.Color(red*bright_tmp,green*bright_tmp,blue*bright_tmp));
      }
  
      //update values to al pixels
      strip.show(); 
  
      //Wait the correct amount of time
      delay(wait);
    }
    flip=!flip;
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
   
  if (setBright){
    incrementBrightness();
    startBrightCurr=millis();
    }
  }
  else{
    timeHeld=millis()-lastPressTime;
    if (timeHeld>brightPressTime){
      setBright=true;
      startBrightCurr=millis();
    }
    else if(!setBright){
    showType++;
      if (showType >= numShows){
        showType=0;
      }
      switchShows=true;  
    }
  }
  saveSettings();
  sei();
}






void incrementBrightness(){
//Update the brightness
  BrightnessIDX++;
  if (BrightnessIDX>=numBrightLevels){
    BrightnessIDX=0;
  }
  Brightness=BRIGHT_LEVELS[BrightnessIDX];

}



/* InOutQuadBlend takes in elements of a vector of times [0;1]
 *  and returns wait times. 
 */

float InOutQuadBlend(float t)
{
    if(t <= 0.5f)
        return (2.0f * t*t);
    t -= 0.5;
    return (2.0f * t * (1.0f - t) + 0.5);
}




float BezierBlend(float t)
{
    return (t*t) * (3.0f - 2.0f * t);
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




