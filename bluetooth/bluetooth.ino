#include <Wire.h>
#include <sx1509_library.h>
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

#define SHIFTPWM_NOSPI
//#define CHANGE 1

const byte SX1509_ADDRESS = 0x3E;
const int interruptPin = 3;
sx1509Class expander(SX1509_ADDRESS, 0, interruptPin);
byte message[4];
byte bottom = B00001111;
bool enter;

const int ShiftPWM_latchPin = 4;
const int ShiftPWM_dataPin = 6;
const int ShiftPWM_clockPin = 5;
const bool ShiftPWM_invertOutputs = false;
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h>

// Function prototypes (telling the compiler these functions exist).
void oneFade(int cup);
void inOutTwoLeds(void);
void inOutAll(void);
void alternatingColors(void);
void hueShiftAll(void);
void randomColors(void);
void fakeVuMeter(void);
void rgbLedRainbow(unsigned long cycleTime, int rainbowWidth);
void printInstructions(void);
void setColor(int cup, int color);
void checkAllCups();

unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 65;
unsigned int numRegisters = 4;
unsigned int numOutputs = numRegisters*8;
unsigned int numRGBLeds = numRegisters*8/3;
unsigned int fadingMode = 0; //start with all LED's off.

int cup1 = 0;
int cup2 = 1;
int cup3 = 2;
int cup4 = 3;
int cup5 = 4;
int cup6 = 5;
int cup7 = 6;
int cup8 = 7;
int cup9 = 8;
int cup10 = 9;
int cupall = 10;

int cup1color[4];
int cup2color[4];
int cup3color[4];
int cup4color[4];
int cup5color[4];
int cup6color[4];
int cup7color[4];
int cup8color[4];
int cup9color[4];
int cup10color[4];
int cupallcolor[4];

int cupChosen;
int colorChosen;
int redValue;
int greenValue;
int blueValue;

volatile boolean cupStatus[10] = {0,0,0,0,0,0,0,0,0,0};

unsigned long startTime = 0; // start time for the chosen fading mode
unsigned long checkCupTimer = millis();
boolean manualCheck = false;

String readString;

void setup(){
  
  Serial.begin(9600);

  expander.init();
  expander.pinDir(0, INPUT);
  expander.pinDir(1, INPUT);
  expander.pinDir(2, INPUT);
  expander.pinDir(3, INPUT);
  expander.pinDir(4, INPUT);
  expander.pinDir(5, INPUT);
  expander.pinDir(6, INPUT);
  expander.pinDir(7, INPUT);
  expander.pinDir(8, INPUT);
  expander.pinDir(9, INPUT);
  
  expander.enableInterrupt(0, 1);
  expander.enableInterrupt(1, 1);
  expander.enableInterrupt(2, 1);
  expander.enableInterrupt(3, 1);
  expander.enableInterrupt(4, 1);
  expander.enableInterrupt(5, 1);
  expander.enableInterrupt(6, 1);
  expander.enableInterrupt(7, 1);
  expander.enableInterrupt(8, 1);
  expander.enableInterrupt(9, 1);
  
  ShiftPWM.SetAmountOfRegisters(numRegisters);
  ShiftPWM.SetPinGrouping(1); //This is the default, but I added here to demonstrate how to use the funtion
  ShiftPWM.Start(pwmFrequency,maxBrightness);

  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  Mirf.setRADDR((byte *)"serv1");
  Mirf.setTADDR((byte *)"cliel");
  
  /*
   * Set the payload length to sizeof(unsigned long) the
   * return type of millis().
   *
   * NB: payload on client and server must be the same.
   */
   
  Mirf.payload = sizeof(char);
  
  /*
   * Write channel and payload config then power up reciver.
   */
   
  Mirf.config();
  Serial.println("Listening..."); 
  
   cup1color[0] = 1;
   cup2color[0] = 1;
   cup3color[0] = 1;
   cup4color[0] = 1;
   cup5color[0] = 1;
   cup6color[0] = 1;
   cup7color[0] = 1;
   cup8color[0] = 1;
   cup9color[0] = 1;
   cup10color[0] = 1;
   
   enter = false;
  
  //attachInterrupt(1, checkAllCups, FALLING);
}

void loop()
{    
  
  if(digitalRead(interruptPin) == LOW) checkAllCups(); // interruptPin goes low when a cup state changes from IOexpander, therefore check cups and update 
  
  int index = 0;
  while(Serial.available()){ // while there is data available on the serial monitor
    message[index] = Serial.read(); // store string from serial command
    //Serial.println(message[index]);
    index = index + 1;
    enter = false;
    Serial.println("set enter to false");
  }
  if(!Serial.available()){
    Serial.println(enter);
    if(message[0] != 0 || message[1] != 0 || message[2] != 0 || message[3] != 0  || enter){ // if data is available or if need to not block function
  Serial.println("entered if");
  /* message should be 4 bytes long
  byte 1: 4 bits for cup number (0-A), A is all cups
          4 bits for cup color (numbers will represent standard colors in the list as well as fading patterns. if this number is 0 then check the next 3 bytes for a custom color)
  byte 2: 8 bits for red value
  byte 3: 8 bits for green value
  byte 4: 8 bits for blue value
  */
  
      cupChosen = message[0] >> 4;     
      colorChosen = message[0] & bottom;
      redValue = message[1];
      greenValue = message[2];
      blueValue = message[3];
      
      Serial.print("Cup: ");
      Serial.println(cupChosen);
      Serial.print("Color: ");
      Serial.println(colorChosen);
      Serial.print("Red: ");
      Serial.println(redValue);
      Serial.print("Green: ");
      Serial.println(greenValue);
      Serial.print("Blue: ");
      Serial.println(blueValue);
 
      if(cupChosen != 10){ // if command is for just one cup
        if(colorChosen){ // if command is for an actual color, set the cup to the color
          switch(cupChosen){
           case 0: cup1color[0] = colorChosen;
                   break;
           case 1: cup2color[0] = colorChosen;
                   break;
           case 2: cup3color[0] = colorChosen;
                   break;
           case 3: cup4color[0] = colorChosen;
                   break;
           case 4: cup5color[0] = colorChosen;
                   break;
           case 5: cup6color[0] = colorChosen;
                   break;
           case 6: cup7color[0] = colorChosen;
                   break;
           case 7: cup8color[0] = colorChosen;
                   break;
           case 8: cup9color[0] = colorChosen;
                   break;
           case 9: cup10color[0] = colorChosen;
                   break;     
          }
          if(cupStatus[cupChosen]) setColor(cupChosen, colorChosen);
        } 
        else{ // if command is not for an actual color, set the cup to the RGB value
          switch(cupChosen){
             case 0: cup1color[0] = 0;
                     cup1color[1] = redValue;
                     cup1color[2] = greenValue;
                     cup1color[3] = blueValue;
                     break;
             case 1: cup2color[0] = 0;
                     cup2color[1] = redValue;
                     cup2color[2] = greenValue;
                     cup2color[3] = blueValue;
                     break;
             case 2: cup3color[0] = 0;
                     cup3color[1] = redValue;
                     cup3color[2] = greenValue;
                     cup3color[3] = blueValue;
                     break;
             case 3: cup4color[0] = 0;
                     cup4color[1] = redValue;
                     cup4color[2] = greenValue;
                     cup4color[3] = blueValue;
                     break;
             case 4: cup5color[0] = 0;
                     cup5color[1] = redValue;
                     cup5color[2] = greenValue;
                     cup5color[3] = blueValue;
                     break;
             case 5: cup6color[0] = 0;
                     cup6color[1] = redValue;
                     cup6color[2] = greenValue;
                     cup6color[3] = blueValue;
                     break;
             case 6: cup7color[0] = 0;
                     cup7color[1] = redValue;
                     cup7color[2] = greenValue;
                     cup7color[3] = blueValue;
                     break;
             case 7: cup8color[0] = 0;
                     cup8color[1] = redValue;
                     cup8color[2] = greenValue;
                     cup8color[3] = blueValue;
                     break;
             case 8: cup9color[0] = 0;
                     cup9color[1] = redValue;
                     cup9color[2] = greenValue;
                     cup9color[3] = blueValue;
                     break;
             case 9: cup10color[0] = 0;
                     cup10color[1] = redValue;
                     cup10color[2] = greenValue;
                     cup10color[3] = blueValue;
                     break;       
            }
          if(cupStatus[cupChosen]) ShiftPWM.SetRGB(cupChosen, redValue, greenValue, blueValue);
        } 
      }
      else{  // if command is for all cups
        if(!colorChosen){  // if command has colorChosen = 0, then custom color was selected
                     cup1color[0] = 0;
                     cup1color[1] = redValue;
                     cup1color[2] = greenValue;
                     cup1color[3] = blueValue;
                     
                     cup2color[0] = 0;
                     cup2color[1] = redValue;
                     cup2color[2] = greenValue;
                     cup2color[3] = blueValue;
                    
                     cup3color[0] = 0;
                     cup3color[1] = redValue;
                     cup3color[2] = greenValue;
                     cup3color[3] = blueValue;
                    
                     cup4color[0] = 0;
                     cup4color[1] = redValue;
                     cup4color[2] = greenValue;
                     cup4color[3] = blueValue;
                    
                     cup5color[0] = 0;
                     cup5color[1] = redValue;
                     cup5color[2] = greenValue;
                     cup5color[3] = blueValue;
                    
                     cup6color[0] = 0;
                     cup6color[1] = redValue;
                     cup6color[2] = greenValue;
                     cup6color[3] = blueValue;
                     
                     cup7color[0] = 0;
                     cup7color[1] = redValue;
                     cup7color[2] = greenValue;
                     cup7color[3] = blueValue;
                    
                     cup8color[0] = 0;
                     cup8color[1] = redValue;
                     cup8color[2] = greenValue;
                     cup8color[3] = blueValue;

                     cup9color[0] = 0;
                     cup9color[1] = redValue;
                     cup9color[2] = greenValue;
                     cup9color[3] = blueValue;

                     cup10color[0] = 0;
                     cup10color[1] = redValue;
                     cup10color[2] = greenValue;
                     cup10color[3] = blueValue;

              for(unsigned int led=0;led<numRGBLeds;led++){
                if(cupStatus[led]) ShiftPWM.SetRGB(led, redValue, greenValue, blueValue);
            }
        }
        else if(colorChosen <= 7){ // if command has colorChosen for a simple color, set all cups to that color
             cup1color[0] = colorChosen;
             
             cup2color[0] = colorChosen;
            
             cup3color[0] = colorChosen;
            
             cup4color[0] = colorChosen;
            
             cup5color[0] = colorChosen;
            
             cup6color[0] = colorChosen;
             
             cup7color[0] = colorChosen;
            
             cup8color[0] = colorChosen;

             cup9color[0] = colorChosen;

             cup10color[0] = colorChosen;
             
              for(unsigned int led=0;led<numRGBLeds;led++){
              if(cupStatus[led]) setColor(led, colorChosen);
              }
        }
        else{  // else, command is for a function with all cups, so don't block
          switch(colorChosen){
            case 8: alternatingColors();
                    enter = true;
                    break;
            case 9: hueShiftAll();
                    enter = true;
                    break;
            case 10: randomColors();
                     enter = true;
                     break;
            case 11: rgbLedRainbow(3000,numRGBLeds);
                     enter = true;
                     break;
            case 12: rgbLedRainbow(10000,5*numRGBLeds);
                     enter = true;
                     break;
          }
        }
      }
    }
  }
}

void alternatingColors(void){ // Alternate LED's in 6 different colors
  unsigned long holdTime = 2000;
  unsigned long time = millis()-startTime;
  unsigned long shift = (time/holdTime)%6;
  for(unsigned int led=0; led<numRGBLeds; led++){
    switch((led+shift)%6){
    case 0:
      if(cupStatus[led]) ShiftPWM.SetRGB(led,255,0,0);    // red
      break;
    case 1:
      if(cupStatus[led]) ShiftPWM.SetRGB(led,0,255,0);    // green
      break;
    case 2:
      if(cupStatus[led]) ShiftPWM.SetRGB(led,0,0,255);    // blue
      break;
    case 3:
      if(cupStatus[led]) ShiftPWM.SetRGB(led,255,128,0);  // orange
      break;
    case 4:
      if(cupStatus[led]) ShiftPWM.SetRGB(led,0,255,255);  // turqoise
      break;
    case 5:
      if(cupStatus[led]) ShiftPWM.SetRGB(led,255,0,255);  // purple
      break;
    }
  }
}

void hueShiftAll(void){  // Hue shift all LED's
  unsigned long cycleTime = 10000;
  unsigned long time = millis()-startTime;
  unsigned long hue = (360*time/cycleTime)%360;
  for(unsigned int led=0; led<numRGBLeds; led++){
    if(cupStatus[led]) ShiftPWM.SetHSV(led, hue, 255, 255); 
  }
}

void randomColors(void){  // Update random LED to random color. Funky!
  unsigned long updateDelay = 100;
  static unsigned long previousUpdateTime;
  if(millis()-previousUpdateTime > updateDelay){
    previousUpdateTime = millis();
    unsigned int led = random(numRGBLeds);
    if(cupStatus[led]) ShiftPWM.SetHSV(led,random(360),255,255);
  }
}

void rgbLedRainbow(unsigned long cycleTime, int rainbowWidth){
  // Displays a rainbow spread over a few LED's (numRGBLeds), which shifts in hue. 
  // The rainbow can be wider then the real number of LED's.
  unsigned long time = millis()-startTime;
  unsigned long colorShift = (360*time/cycleTime)%360; // this color shift is like the hue slider in Photoshop.

  for(unsigned int led=0;led<numRGBLeds;led++){ // loop over all LED's
    int hue = ((led)*360/(rainbowWidth-1)+colorShift)%360; // Set hue from 0 to 360 from first to last led and shift the hue
    if(cupStatus[led]) ShiftPWM.SetHSV(led, hue, 255, 255); // write the HSV values, with saturation and value at maximum
  }
}

void printInstructions(void){
  Serial.println("---- ShiftPWM Non-blocking fades demo ----");
  Serial.println("");
  
  Serial.println("Type 'l' to see the load of the ShiftPWM interrupt (the % of CPU time the AVR is busy with ShiftPWM)");
  Serial.println("");
  Serial.println("Type any of these numbers to set the demo to this mode:");
  Serial.println("  0. All LED's off");
  Serial.println("  1. Fade in and out one by one");
  Serial.println("  2. Fade in and out all LED's");
  Serial.println("  3. Fade in and out 2 LED's in parallel");
  Serial.println("  4. Alternating LED's in 6 different colors");
  Serial.println("  5. Hue shift all LED's");
  Serial.println("  6. Setting random LED's to random color");
  Serial.println("  7. Fake a VU meter");
  Serial.println("  8. Display a color shifting rainbow as wide as the LED's");
  Serial.println("  9. Display a color shifting rainbow wider than the LED's");  
  Serial.println("");
  Serial.println("Type 'm' to see this info again");  
  Serial.println("");
  Serial.println("----");
}

void setColor(int cup, int color){
   switch(color){
     case 1: // red
          if(cup == 10){
            for(unsigned int led=0;led<numRGBLeds;led++){
              if(cupStatus[led]) ShiftPWM.SetRGB(led, 255, 0, 0);
            }
            cup1color[0] = cup2color[0] = cup3color[0] = cup4color[0] = cup5color[0] = cup6color[0] = cup7color[0] = cup8color[0] = cup9color[0] = cup10color[0] = color;
          }
          else ShiftPWM.SetRGB(cup, 255, 0, 0);
          break;
     case 2: // green
          if(cup == 10){
            for(unsigned int led=0;led<numRGBLeds;led++){
              if(cupStatus[led]) ShiftPWM.SetRGB(led, 0, 255, 0);
            }
            cup1color[0] = cup2color[0] = cup3color[0] = cup4color[0] = cup5color[0] = cup6color[0] = cup7color[0] = cup8color[0] = cup9color[0] = cup10color[0] = color;
          }
          else ShiftPWM.SetRGB(cup, 0, 255, 0);
          break;
     case 3: // blue
          if(cup == 10){
            for(unsigned int led=0;led<numRGBLeds;led++){
              if(cupStatus[led]) ShiftPWM.SetRGB(led, 0, 0, 255);
            }
            cup1color[0] = cup2color[0] = cup3color[0] = cup4color[0] = cup5color[0] = cup6color[0] = cup7color[0] = cup8color[0] = cup9color[0] = cup10color[0] = color;
          }
          else ShiftPWM.SetRGB(cup, 0, 0, 255);
          break;
     case 4: // orange
          if(cup == 10){
            for(unsigned int led=0;led<numRGBLeds;led++){
              if(cupStatus[led]) ShiftPWM.SetRGB(led, 255, 128, 0);
            }
            cup1color[0] = cup2color[0] = cup3color[0] = cup4color[0] = cup5color[0] = cup6color[0] = cup7color[0] = cup8color[0] = cup9color[0] = cup10color[0] = color;
          }
          else ShiftPWM.SetRGB(cup, 255, 128, 0);
          break;
     case 5: // turquoise
          if(cup == 10){
            for(unsigned int led=0;led<numRGBLeds;led++){
              if(cupStatus[led]) ShiftPWM.SetRGB(led, 0, 255, 255);
            }
            cup1color[0] = cup2color[0] = cup3color[0] = cup4color[0] = cup5color[0] = cup6color[0] = cup7color[0] = cup8color[0] = cup9color[0] = cup10color[0] = color;
          }
          else ShiftPWM.SetRGB(cup, 0, 255, 255);
          break;
     case 6: // purple
          if(cup == 10){
            for(unsigned int led=0;led<numRGBLeds;led++){
              if(cupStatus[led]) ShiftPWM.SetRGB(led, 255, 0, 255);
            }
            cup1color[0] = cup2color[0] = cup3color[0] = cup4color[0] = cup5color[0] = cup6color[0] = cup7color[0] = cup8color[0] = cup9color[0] = cup10color[0] = color;
          }
          else ShiftPWM.SetRGB(cup, 255, 0, 255);
          break;
     case 7: // white
          if(cup == 10){
            for(unsigned int led=0;led<numRGBLeds;led++){
              if(cupStatus[led]) ShiftPWM.SetRGB(led, 255, 255, 255);
            }
            cup1color[0] = cup2color[0] = cup3color[0] = cup4color[0] = cup5color[0] = cup6color[0] = cup7color[0] = cup8color[0] = cup9color[0] = cup10color[0] = color;
          }
          else ShiftPWM.SetRGB(cup, 255, 255, 255);
          break;  
     }
}


void checkAllCups(){
  Serial.println("here");
  volatile unsigned int sources = expander.interruptSource();
  if((sources & (1<<0)) || manualCheck){
    if(expander.readPin(0) == LOW){
      cupStatus[0] = true;
      if(cup1color[0]!=0) setColor(cup1, cup1color[0]);
      else ShiftPWM.SetRGB(cup1, cup1color[1], cup1color[2], cup1color[3]);
      Serial.println("cup 1 on");
    }
    else{
      cupStatus[0] = false;
      ShiftPWM.SetRGB(cup1, 0, 0, 0);
      Serial.println("cup 1 off");
    }
  }
  if((sources & (1<<1)) || manualCheck){
    if(expander.readPin(1) == LOW){
      cupStatus[1] = true;
      if(fadingMode!=10) setColor(cup2, cup2color[0]);
      else ShiftPWM.SetRGB(cup2, cup2color[1], cup2color[2], cup2color[3]);
      Serial.println("cup 2 on");
    }
    else{
      cupStatus[1] = false;
      ShiftPWM.SetRGB(cup2, 0, 0, 0);
      Serial.println("cup 2 off");
    }
  }
  if((sources & (1<<2)) || manualCheck){
    if(expander.readPin(2) == LOW){
      cupStatus[2] = true;
      if(fadingMode!=10) setColor(cup3, cup3color[0]);
      else ShiftPWM.SetRGB(cup3, cup3color[1], cup3color[2], cup3color[3]);
      Serial.println("cup 3 on");
    }
    else{
      cupStatus[2] = false;
      ShiftPWM.SetRGB(cup3, 0, 0, 0);
      Serial.println("cup 3 off");
    }
  }
  if((sources & (1<<3)) || manualCheck){
    if(expander.readPin(3) == LOW){
      cupStatus[3] = true;
      if(fadingMode!=10) setColor(cup4, cup4color[0]);
      else ShiftPWM.SetRGB(cup4, cup4color[1], cup4color[2], cup4color[3]);
      Serial.println("cup 4 on");
    }
    else{
      cupStatus[3] = false;
      ShiftPWM.SetRGB(cup4, 0, 0, 0);
      Serial.println("cup 4 off");
    }
  }
  if((sources & (1<<4)) || manualCheck){
    if(expander.readPin(4) == LOW){
      cupStatus[4] = true;
      if(fadingMode!=10) setColor(cup5, cup5color[0]);
      else ShiftPWM.SetRGB(cup5, cup5color[1], cup5color[2], cup5color[3]);
      Serial.println("cup 5 on");
    }
    else{
      cupStatus[4] = false;
      ShiftPWM.SetRGB(cup5, 0, 0, 0);
      Serial.println("cup 5 off");
    }
  }
  if((sources & (1<<5)) || manualCheck){
    if(expander.readPin(5) == LOW){
      cupStatus[5] = true;
      if(fadingMode!=10) setColor(cup6, cup6color[0]);
      else ShiftPWM.SetRGB(cup6, cup6color[1], cup6color[2], cup6color[3]);
      Serial.println("cup 6 on");
    }
    else{
      cupStatus[5] = false;
      ShiftPWM.SetRGB(cup6, 0, 0, 0);
      Serial.println("cup 6 off");
    }
  }
  if((sources & (1<<6)) || manualCheck){
    if(expander.readPin(6) == LOW){
      cupStatus[6] = true;
      if(fadingMode!=10) setColor(cup7, cup7color[0]);
      else ShiftPWM.SetRGB(cup7, cup7color[1], cup7color[2], cup7color[3]);
      Serial.println("cup 7 on");
    }
    else{
      cupStatus[6] = false;
      ShiftPWM.SetRGB(cup7, 0, 0, 0);
      Serial.println("cup 7 off");
    }
  }
  if((sources & (1<<7)) || manualCheck){
    if(expander.readPin(7) == LOW){
      cupStatus[7] = true;
      if(fadingMode!=10) setColor(cup8, cup8color[0]);
      else ShiftPWM.SetRGB(cup8, cup8color[1], cup8color[2], cup8color[3]);
      Serial.println("cup 8 on");
    }
    else{
      cupStatus[7] = false;
      ShiftPWM.SetRGB(cup8, 0, 0, 0);
      Serial.println("cup 8 off");
    }
  }
  if((sources & (1<<8)) || manualCheck){
    if(expander.readPin(8) == LOW){
      cupStatus[8] = true;
      if(fadingMode!=10) setColor(cup9, cup9color[0]);
      else ShiftPWM.SetRGB(cup9, cup9color[1], cup9color[2], cup9color[3]);
      Serial.println("cup 9 on");
    }
    else{
      cupStatus[8] = false;
      ShiftPWM.SetRGB(cup9, 0, 0, 0);
      Serial.println("cup 9 off");
    }
  }
  if((sources & (1<<9)) || manualCheck){
    if(expander.readPin(9) == LOW){
      cupStatus[9] = true;
      if(fadingMode!=10) setColor(cup10, cup10color[0]);
      else ShiftPWM.SetRGB(cup10, cup10color[1], cup10color[2], cup10color[3]);
      Serial.println("cup 10 on");
    }
    else{
      cupStatus[9] = false;
      ShiftPWM.SetRGB(cup10, 0, 0, 0);
      Serial.println("cup 10 off");
    }
  }
  manualCheck = false;
  sources = 0;
}
