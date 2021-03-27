#include <EEPROM.h>
#include <EEPROMAnything.h>

// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <Adafruit_NeoPixel.h>


#define BUTTON_PIN 2    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.

#define STRIP1_PIN    5    // Digital IO pin connected to the NeoPixels.
#define STRIP2_PIN    6    // Digital IO pin connected to the NeoPixels.

#define STRIP1_PIXEL_COUNT 22
#define STRIP2_PIXEL_COUNT 11

bool STRIP1_INVERT = true;
bool STRIP2_INVERT = false;

const uint8_t BRIGHTNESS_DATA_START = 0;
const uint8_t SHOW_DATA_START = 1;
const uint8_t SHOW_ON_START = 2;
const uint8_t ANIMATION_START = 3;
const uint8_t PULSE_START = 4;
const uint8_t CYCLE_START = 5;

const int numBrightLevels = 10;
const float BRIGHT_LEVELS[numBrightLevels]={0.10,0.20,0.30,0.40,0.50,0.60,0.70,0.80,0.90,1.00};
uint8_t BrightnessIDX=0;

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP1_PIXEL_COUNT, STRIP1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(STRIP2_PIXEL_COUNT, STRIP2_PIN, NEO_GRB + NEO_KHZ800);

bool oldState = HIGH;
bool showOn= true;
int showType = 1;
int numShows = 19;
bool switchShows = false;
float Brightness = BRIGHT_LEVELS[3];
bool requiresLoop=0;
bool newButton=false;
bool transition = true;



unsigned long timeBetweenPresses = 10000;  // the last time the output pin was toggled
unsigned long timeHeld =0;
unsigned long lastPressTime = 0;    // the debounce time; increase if the output flickers
unsigned long lastPressDownTime=0;
unsigned long brightPressTime=1000;
unsigned long brightInitTime=250;
unsigned long brightEndTime=1500;
unsigned long startBrightCurr=0;
unsigned long debouncing_time = 400; // [ms] Debouncing time
volatile unsigned long last_micros;

uint8_t global_animation_time=30;
uint8_t global_pulse_time=20;
uint8_t global_cycle_time=5;

bool setBright = false;
bool firstcall=true;


#define DEBUG

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(STRIP1_PIN, OUTPUT);
  pinMode(STRIP2_PIN, OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  strip2.begin();
  strip2.show(); // Initialize all pixels to 'off'

  #ifdef DEBUG
    Serial.begin(115200);
  #endif
  
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), debounceInterrupt, CHANGE);

  readSettings();
  
}

void loop() {
  if(firstcall){
    readSettings();
    firstcall=false;
  }
  else{
    if (!showOn){
      showColorAnimate(0,0,0, global_animation_time);
    }
    else if (setBright){
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
  handleSerial();
}

void startShow() {
  switch (showType){
      case 0: {showColorAnimate(255, 46, 0,global_animation_time);
          requiresLoop=0;
        } //Red
        break;
      case 1: {genPulse(255, 46, 0,global_pulse_time);
          requiresLoop=0;
        } //Red
        break;
      case 2:{showColorAnimate(255, 255, 0,global_animation_time);
          requiresLoop=0;
        }//Yellow
        break;
      case 3: {genPulse(255, 255, 0,global_pulse_time);
          requiresLoop=0;
        } //Red
        break;
      case 4:{showColorAnimate(0, 255, 0,global_animation_time);
          requiresLoop=0;
        }//Green
        break;
      case 5: {genPulse(0, 255, 0,global_pulse_time);
          requiresLoop=0;
        } //Red
        break;
      case 6:{ showColorAnimate(0, 30, 255,global_animation_time); //Blue
          requiresLoop=0;
        }
        break;
      case 7: {genPulse(0, 30, 255,global_pulse_time);
          requiresLoop=0;
        } //Red
        break;
      case 8:{
          showColorAnimate(120, 0, 255,global_animation_time); //Purple
          requiresLoop=0;
        }
        break;
      case 9: {
          genPulse(120, 0, 255,global_pulse_time);
          requiresLoop=0;
        } //Red
        break;
      case 10:{
          showColorAnimate(255, 0, 255,global_animation_time); //Pink
          requiresLoop=0;
        }
        break;
      case 11: {
          genPulse(255, 0, 255,global_pulse_time);
          requiresLoop=0;
        } //Red
        break;
      //White
      case 12:{
          showColorAnimate(255, 250, 245,global_animation_time); //White
          requiresLoop=0;
        }
        break;
      case 13: {showRainbowJustified(global_animation_time); //Rainbow
        requiresLoop=0;
        }
        break;
        //Michigan
      case 14: halfAndHalf(255,255,0,0,31,173,global_animation_time); // halfAndHalfAnimated(255,255,0,0,31,173,20);
        break;
      case 15: rainbowCycleDim(0.75,1,true);
        break;
      case 16: rainbowCycleDim(3,1,true);
        break;
      case 17: rainbowCycleDim(0.25,1,false);
        break;
      case 18: rainbowCycleDim(1,1,false);
        break;
  }
}


//================================

// Same as RainbowCycle, except Dimmer
void rainbowCycleDim_OLD(float wait, uint8_t reps, bool FullColor) {
  uint16_t i, j;
  int idx1 = 0;
  int idx2 = 0;
  int color_val1 = 0;
  int color_val2 = 0;

  uint32_t num_frames = 256;
  uint32_t total_reps = num_frames*reps;

  for(j=0; j<total_reps; j++) { // 5 cycles of all colors on wheel
    handleSerial();
    
    if (setBright || switchShows){
      break ;
    }
    
    for(i=0; i< strip.numPixels(); i++) {
      if (FullColor){
        strip.setPixelColor(i, Wheel(j & 255));
        }
      else{
        idx1 = strip.numPixels() -1 -i;
        idx2 = i;
        color_val1 = j;
        if (STRIP1_INVERT){
          idx1 = i;
          idx2 = strip.numPixels() -1 -i;
          color_val1=total_reps-1-j;
        }
        strip.setPixelColor(idx1, Wheel(((idx2 * 256 / strip.numPixels()) + color_val1) & 255));
      }

      if (j==0 && transition){
        strip.show();
        delay(global_animation_time);
       }

    }

    //Serial.print('\n');
    for(i=0; i< strip2.numPixels(); i++){
      if (FullColor){
        strip2.setPixelColor(i, Wheel(j & 255));
        }
      else{
        idx1 = strip2.numPixels() -1 -i;
        idx2 = i;
        color_val2 = j;
        if (STRIP2_INVERT){
          idx1 = i;
          idx2 = strip2.numPixels() -1 -i;
          color_val2=total_reps-1-j;
        }
        strip2.setPixelColor(idx1, Wheel(((idx2 * 256 / strip2.numPixels()) + color_val2) & 255));
      }
      if (j==0 &&transition){
        strip2.show();
        delay(global_animation_time);
       }
    }
    transition = false;
    strip.show();
    strip2.show();
    delay(wait);
    
  }
}


// Same as RainbowCycle, except Dimmer
void rainbowCycleDim(float wait, uint8_t reps, bool FullColor) {
  uint8_t real_wait = int(wait*float(global_cycle_time));
  uint16_t i, j;
  int idx1 = 0;
  int idx2 = 0;
  int color_val1 = 0;
  int color_val2 = 0;

  int num_frames = 256;
  int total_reps = num_frames*reps;

  uint8_t num_pixels_longest = max(strip.numPixels(),strip2.numPixels());

  for(j=0; j<total_reps; j++) { // 5 cycles of all colors on wheel
    handleSerial();
    
    if (setBright || switchShows){
      break ;
    }
    
    for(i=0; i< num_pixels_longest; i++) {      
      if (FullColor){
        strip.setPixelColor(i, Wheel(j & 255));
        strip2.setPixelColor(i, Wheel(j & 255));
      }
      else{
        idx1 = i;
        idx2 = i;
        color_val1 = j;
        if (STRIP1_INVERT){
          idx1 = num_pixels_longest -1 -i;
          idx2 = i;
          //color_val1=total_reps-1-j;
        }
        if(idx1>=0 & idx1<strip.numPixels()){
          strip.setPixelColor(idx1, Wheel(((idx2 * 256 / num_pixels_longest) + color_val1) & 255));
        }


        idx1 = i;
        idx2 = i;
        color_val2 = j;
        if (STRIP2_INVERT){
          idx1 = num_pixels_longest -1 -i;
          idx2 = i;
          //color_val2=total_reps-1-j;
        }
        if(idx1>=0 & idx1<strip2.numPixels()){
          strip2.setPixelColor(idx1, Wheel(((idx2 * 256 / num_pixels_longest) + color_val2) & 255));
        }

        if (j==0 && transition){
          strip.show();
          strip2.show();
          delay(global_animation_time);
         }
      }
    }
       
    transition = false;
    strip.show();
    strip2.show();
    delay(real_wait);
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

    for(i=0; i< strip2.numPixels(); i++) {
      if (i<strip2.numPixels()/2){
        strip2.setPixelColor(i, strip.Color(r1*Brightness,g1*Brightness,b1*Brightness));
      }
      else if ((i>=strip2.numPixels()/2)){
        strip2.setPixelColor(i, strip2.Color(r2*Brightness,g2*Brightness,b2*Brightness));
      } 
      strip2.show(); 
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
        handleSerial();
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

  showColorAnimate(red/float(3), green/float(3), blue/float(3),global_animation_time);

  for (uint8_t k=0;k<2;k++){
    for (uint16_t i=0; i<maxSteps+1; i++){
      handleSerial();
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

      for (uint8_t j=0; j<strip2.numPixels(); j++){
          strip2.setPixelColor(j, strip2.Color(red*bright_tmp,green*bright_tmp,blue*bright_tmp));
      }
  
      //update values to al pixels
      strip.show(); 
      strip2.show(); 
  
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
  for(int i=0; i< strip2.numPixels(); i++) {
      strip2.setPixelColor(i, strip2.Color(red*Brightness,green*Brightness,blue*Brightness));
  }
  strip.show();  
  strip2.show();
  transition=false;
}


void showColorAnimate(uint8_t red,uint8_t green,uint8_t blue,uint16_t wait){
  uint8_t num_pixels_longest = max(strip.numPixels(),strip2.numPixels());
  int idx1 = 0;
  int idx2 = 0;
  for(int i=0; i< num_pixels_longest; i++) {
      idx1 = i;
      if (STRIP1_INVERT){
        idx1 = num_pixels_longest -1 -i;
      }
      if(idx1>=0 & idx1<strip.numPixels()){
        strip.setPixelColor(idx1, strip.Color(red*Brightness,green*Brightness,blue*Brightness));
      }
      
      idx2 = i;
      if (STRIP2_INVERT){
        idx2 = num_pixels_longest -1 -i;
      }
      if(idx2>=0 & idx2<strip2.numPixels()){  
        strip2.setPixelColor(idx2, strip.Color(red*Brightness,green*Brightness,blue*Brightness));
      }

      strip.show();
      strip2.show(); 
      delay(wait);
  }
  transition=false;
     
}


void showRainbow(uint16_t wait){
  uint16_t j=0;
  uint16_t i=0;
  int idx = 0;
  for(i=0; i< strip.numPixels(); i++) {
    idx=i;
    if (STRIP1_INVERT){
      idx = strip.numPixels() -1 -i;
    }
    strip.setPixelColor(idx, Wheel(((idx * 256 / strip.numPixels()) + j) & 255));
    strip.show(); 
    delay(wait); 
  }
  for(i=0; i< strip2.numPixels(); i++) {
    idx=i;
    if (STRIP2_INVERT){
      idx = strip2.numPixels() -1 -i;
    }
    strip2.setPixelColor(idx, Wheel(((idx * 256 / strip2.numPixels()) + j) & 255));
    strip2.show(); 
    delay(wait); 
  }
  transition=false;
}


void showRainbowJustified(uint16_t wait){
  uint16_t j=0;
  uint16_t i=0;
  int idx1 = 0;
  int idx2 = 0;
  for(i=0; i< strip.numPixels(); i++) {
    idx1=i;
    idx2=i;
    if (STRIP1_INVERT){
      idx1 = strip.numPixels() -1 -i;
      idx2 = i;
      //j=256/2;
    }
    strip.setPixelColor(idx1, Wheel(((idx2 * 256 / strip.numPixels()) + j) & 255));
    strip.show(); 
    delay(wait); 
  }
  for(i=0; i< strip2.numPixels(); i++) {
    idx1=i;
    idx2=i;
    j=0;
    if (STRIP2_INVERT){
      idx1 = strip2.numPixels() -1 -i;
      idx2 = i;
      //j=256/2;
    }
    strip2.setPixelColor(idx1, Wheel(((idx2 * 256 / strip2.numPixels()) + j) & 255));
    strip2.show(); 
    delay(wait); 
  }
  transition=false;
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
    return strip.Color((255 - WheelPos * 3)*Brightness,
                        0,
                        (WheelPos * 3)*Brightness
                        );
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0,
                       (WheelPos * 3)*Brightness,
                       (255 - WheelPos * 3)*Brightness
                       );
  }
  WheelPos -= 170;
  return strip.Color((WheelPos * 3)*Brightness,
                     (255 - WheelPos * 3)*Brightness,
                     0
                     );
}


//ISR
void debounceInterrupt() {
  if((long)(micros() - last_micros) >= debouncing_time * 1000) {
    bool reading = digitalRead(BUTTON_PIN);
    if(reading==LOW){ //Falling
      lastPressDownTime = millis();
    }
    else{   
      switchShow();
      last_micros = micros();
    }
  }
}



boolean switchShow(){
  unsigned long currTime = millis();

  if(setBright){
    incrementBrightness();
    startBrightCurr=millis();
  }
  else{
    if((currTime-lastPressDownTime) > brightPressTime){
      setBright=true;
      //Serial.println("Set Brightness");
      startBrightCurr=millis();
    }
    else{
      showType++;
      if (showType >= numShows){
        showType=0;
      }
      switchShows=true;
      //Serial.println("Switch Shows");
    }

    lastPressTime=currTime;
  }
  saveSettings();
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
  EEPROM.write(SHOW_ON_START, showOn);
  EEPROM.write(SHOW_DATA_START, showType);
  EEPROM.write(BRIGHTNESS_DATA_START, BrightnessIDX);
  EEPROM.write(ANIMATION_START, global_animation_time);
  EEPROM.write(PULSE_START, global_pulse_time);
  EEPROM.write(CYCLE_START, global_cycle_time);

  //EEPROM_writeAnything(, showType);
}

void readSettings(){
    //Read old settings from EEPROM
  //EEPROM_readAnything(BRIGHTNESS_DATA_START, Brightness);
  showOn = EEPROM.read(SHOW_ON_START);
  uint8_t tmpShow= EEPROM.read(SHOW_DATA_START);
  uint8_t tmpBright=EEPROM.read(BRIGHTNESS_DATA_START);
  uint8_t tmp_ani=EEPROM.read(ANIMATION_START);
  uint8_t tmp_pulse=EEPROM.read(PULSE_START);
  uint8_t tmp_cycle=EEPROM.read(CYCLE_START);

  
  //If show number is unreasonable, go back to default
  if (tmpShow<numShows & tmpShow>=0){
    showType = tmpShow;
  }
 
  if(tmpBright<numBrightLevels & tmpBright>=0){
    BrightnessIDX = tmpBright;
    Brightness=BRIGHT_LEVELS[BrightnessIDX];
  }

  if(tmp_ani<255 & tmp_ani>=0){
    global_animation_time = tmp_ani;
  }

  if(tmp_pulse<255 & tmp_pulse>=0){
    global_pulse_time = tmp_pulse;
  }
  if(tmp_cycle<255 & tmp_cycle>=0){
    global_cycle_time = tmp_cycle;
  }


}




void handleSerial(){

  String command = readCommand();

  if (command != ""){
    String cmdStr = getStringValue(command, ';', 0);
  
    if (cmdStr == "BRIGHT"){
      if (getStringValue(command, ';', 1).length()){
        int tmp_bright = constrain(getStringValue(command, ';', 1).toInt(), 0, 100);

        float tmp_brightness = tmp_bright/100.0;

        uint8_t closest_idx = 0;
        float last_diff = 10000.0;
        for (int i=0; i<numBrightLevels; i++){
          float curr_bright = BRIGHT_LEVELS[i];
          float curr_diff = abs(curr_bright-tmp_brightness);
          if (curr_diff<=last_diff){
            closest_idx = i;
          }
        }
         
        BrightnessIDX = closest_idx;
                
        Brightness=tmp_brightness;
        saveSettings();
        }
      sendCommand("BRIGHT: Brightness: "+String(Brightness*100)+"%");
      
    }
    else if(cmdStr == "SET"){
      if (getStringValue(command, ';', 1).length()){
        showType = constrain(getStringValue(command, ';', 1).toInt(), 0, numShows-1);
        saveSettings();
        switchShows=true;
        transition = true;
      }
      sendCommand("SET: Show: "+String(showType));
    }
    else if(cmdStr == "ON"){
      showOn = true;
      sendCommand("ON: Lights On");
      saveSettings();
      transition=true;
    }
    else if(cmdStr == "OFF"){
      showOn = false;
      sendCommand("OFF: Lights Off");
      saveSettings();
      switchShows=true;
      transition=true;
    }
    else if(cmdStr == "TIME"){
      if (getStringValue(command, ';', 1).length()){
        global_animation_time = constrain(getStringValue(command, ';', 1).toInt(), 0, 255);
        saveSettings();
      }
      sendCommand("TIME: Animation Time: " +String(global_animation_time));
    }

    else if(cmdStr == "PULSE"){
      if (getStringValue(command, ';', 1).length()){
        global_pulse_time = constrain(getStringValue(command, ';', 1).toInt(), 0, 255);
        saveSettings();
        switchShows=true;
      }
      sendCommand("PULSE: Pulse Time: " +String(global_pulse_time));
    }
    else if(cmdStr == "CYCLE"){
      if (getStringValue(command, ';', 1).length()){
        global_cycle_time = constrain(getStringValue(command, ';', 1).toInt(), 0, 255);
        saveSettings();
        switchShows=true;
      }
      sendCommand("CYCLE: Cycle Time: " +String(global_cycle_time));
    }
    else if(cmdStr == "FLIP"){
      bool strip1_flip = STRIP1_INVERT;
      bool strip2_flip = STRIP2_INVERT;
      if (getStringValue(command, ';', 2).length()){
        strip1_flip = constrain(getStringValue(command, ';', 1).toInt(), 0, 1);
        strip2_flip = constrain(getStringValue(command, ';', 2).toInt(), 0, 1);
        STRIP1_INVERT = strip1_flip;
        STRIP2_INVERT = strip2_flip;
        saveSettings();
        switchShows=true;
        transition=true;
      }
      sendCommand("FLIP: Flipped Animations: " +String(STRIP1_INVERT)+"\t"+String(STRIP2_INVERT));
    }
    else{
        sendCommand("UNREC: Unrecognized Command");
    }
  }

}


String getStringValue(String data, char separator, int index)
{
  if (data.indexOf(";") == -1 && index==0){
    return  data;
  }
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


String readCommand() {
  String command;
  //unsigned long start_time = micros();
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // If the incoming character is a newline, set a flag so we can process it
    if (inChar == '\n') {
      command.toUpperCase();
      //Serial.print("_SER: Line Complete");
      //Serial.print('\t');
      //Serial.println(micros() - start_time);
      return command;
    }
    // Add new byte to the inputString:
    command += inChar;
  }
  return "";
}

void sendCommand(String bc_string){
  Serial.println(bc_string);
}
