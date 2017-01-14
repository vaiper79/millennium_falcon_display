// Code put together by Ole Andre (ole@olendre.net), mode details to be found at https://oleandre.net
// Code snippets used: 
// - Adafruit: https://github.com/adafruit/Adafruit_DotStar
// - Adafruit: https://learn.adafruit.com/multi-tasking-the-arduino-part-1/overview
// - Technobly: https://github.com/technobly/Particle-DotStar
//
// Code is Work In Progress, and by no means finished


// Includes
#include "application.h"
#include "dotstar/dotstar.h"

// Pin Definitions
// LED Strip
#define DATAPIN   DAC // Dotstar
#define CLOCKPIN  A5 // Dotstar
// Audio Triggers
#define audioPin00   D0   // Left Right Left Right
#define audioPin01   D1   //
#define audioPin02   D2   //
#define audioPin03   D3   //
#define audioPin04   D4
#define audioPin05   D5
#define audioPin06   D6
#define audioPin07   D7
#define audioPin08   A0
#define audioPin09   A1
#define audioPin10  A2
// Volume - issues..looking at other option
#define volumeUp    A4
#define volumeDown  A3

// Define Constants
#define NUMPIXELS 72 // Number of LEDs in dotstar strip

// Globally valid variables
/* Because I forget.. 
int8_t      =   char    :   -128 to 127
uint8_t     =   byte    :   0 to 255
int16_t     =   int     :   -32,768 to 32,767
uint16_t    =   word    :   0 to 65,535
*/

int looper = 999;               // So we don't enter any if statements in the void loop :)
int audio = 0;                  // Used to set state so audio files are only triggered once
int ledState = 1;
unsigned long startTime = 0;    // For timing lights and sound
unsigned long sequenceStart;    // For timing lights and sound

long previousMillis = 0;        // will store last time LED was updated
long interval = 1000;

// Define the dotstar strip
Adafruit_DotStar dotStrip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_RBG);

// Set Photon to correct mode
SYSTEM_MODE(AUTOMATIC);

void setup(){

    //pinMode(DATAPIN, OUTPUT);               // These three, while untested, might be the solution to running both dotstar and neopixels in the same circuit
    //pinMode(CLOCKPIN, OUTPUT);              // and might also explain why I was having such erratic behavior with the dotstar. 
    //pinMode(neopixel_pin);                // I have rebuilt the circuit using neopixels and will not have a chance to test this right away

    Serial.begin(9600);                     // Start serial interface, in case we need some output for debuggig.
    Particle.function("falcon", falcon);   // The cloud exposed function
    pinMode(audioPin00, OUTPUT);            // Pins defined as outputs
    digitalWrite(audioPin00, HIGH);         // The set high due to the nature of the audio board
    pinMode(audioPin01, OUTPUT);
    digitalWrite(audioPin01, HIGH);
    pinMode(audioPin02, OUTPUT);
    digitalWrite(audioPin02, HIGH);
    pinMode(audioPin03, OUTPUT);
    digitalWrite(audioPin03, HIGH);
    pinMode(audioPin04, OUTPUT);
    digitalWrite(audioPin04, HIGH);
    pinMode(audioPin05, OUTPUT);
    digitalWrite(audioPin05, HIGH);
    pinMode(audioPin06, OUTPUT);
    digitalWrite(audioPin06, HIGH);
    pinMode(audioPin07, OUTPUT);
    digitalWrite(audioPin07, HIGH);
    pinMode(audioPin08, OUTPUT);
    digitalWrite(audioPin08, HIGH);
    pinMode(audioPin09, OUTPUT);
    digitalWrite(audioPin09, HIGH);
    pinMode(audioPin10, OUTPUT);
    digitalWrite(audioPin10, HIGH);
/*
    pinMode(volumeUp, OUTPUT);              // Volume pin defined as output
    digitalWrite(volumeUp, HIGH);           // Again, the nature of the audio board requires "off" pins to be high. 
    pinMode(volumeDown, OUTPUT);
    digitalWrite(volumeDown, HIGH);
*/
    dotStrip.begin();                       // Initialize pins for output
    dotStrip.show();                        // Turn all LEDs off ASAP, this does not allways work for some reason
}

int falcon(String cmd) {
    if (cmd == "off") {         // All Stop..mostly meant for use during development
        looper = 0;
    }    
    if (cmd == "onBasic") {     // This will become the default when switched on. Enigne on, some random sounds
        //resetter();
        looper = 1;
    }
    if (cmd == "onAdvanced"){   // This will become the default when switched on. Enigne on, some random sounds
        //resetter();
        looper = 2;
    }
    if (cmd == "takeOff"){      // One of the special sequences requiring timing to work as indended
        resetter();
        looper = 3;
        sequenceStart = millis();
    }
    if (cmd == "hyperdrive"){   // One of the special sequences requiring timing to work as indended 
        resetter();
        looper = 4;
    }
    if (cmd == "up"){           // Volume UP by 5
        volumeU(5);
    }
    if (cmd == "down"){         // Volume DOWN by 5
        volumeD(5);
    }
    if (cmd == "annoying"){     // Silly test sequence..for development only 
        resetter();
        looper = 5;
        sequenceStart = millis();
    }
return 1;   // Returns 1 so we know all went well.. 
}

void loop(){                    // The loop that runs all the time.. 
    if (looper == 0){
        resetter();             // Think this will be mostly used now during development..
    }
    if (looper == 1){           // onBasic; haven't done anything here yet 
        onBasic();
    }
    if (looper == 2){           // onAdvanced; same as above..
        onAdvanced();
    }
    if (looper == 3){           // takeOff, some work done here. Not done.
        takeOff();
    }
    if (looper == 4){           // hyperdrive; haven't done anything here yet 
    }
    if (looper == 5){           // The annoying sequence used for testing.. 
        leftrightleft();        // Development only, this triggers the leftrightleftright audio file
        flashybit();            // Silly light sequence
    }
}

void leftrightleft(){               // Voice going left right left, but I got mono so left left left left...annoying!
    digitalWrite(audioPin00, LOW);  // Pin triggered
}

void flashybit(){
    unsigned long currentMillis = millis();
    
    if(currentMillis - previousMillis > interval) {
    
    // save the last time you changed color
    previousMillis = currentMillis;   
    
    if (ledState == 1){
        dotStrip.setBrightness(170);
        colorAllDot(255,0,0);
        ledState = 2;
    }else if (ledState == 2){
        dotStrip.setBrightness(170);
        colorAllDot(0,0,255);
        ledState = 3;
    }else if (ledState == 3){
        dotStrip.setBrightness(170);
        colorAllDot(0,0,255);
        ledState = 1;
    }
  }
}

void onBasic(){
    colorAllDot(0,0,255);           // The initial blue
    //dotStrip.show();   
}

void onAdvanced(){
    colorAllDot(0,255,0);           // The initial blue
    //dotStrip.show();     
}

void takeOff (){                            // Very early attempt at a take off sequence..basically just found out how to do the timing
    unsigned long currentMillis = millis();
    if (audio == 0){                        // I just want the audio pin to trigger ONCE..so I have to create some sort of controller. audio was set to 0 during power up
        digitalWrite(audioPin02, LOW);      // Trigger audio
        audio = 1;                          // Set audio to 1 as the sound has been triggered..no need to do it again
    }
    if (audio == 1){                        // Since audio has started, we need to start the lights
        if (currentMillis - sequenceStart <= 5610){     // The MF engine lights are darker blue for a bit, then as the engines "explode" to life, they turn a different color..the audio file is about 5610 ms in when this happens..
            colorAllDot(0,0,255);           // The initial blue
            dotStrip.show();                // Send info to LEDs..
        }else{
            colorAllDot(67,67,255);         // After a set amount of time we need a different color as the engine lights should change
            dotStrip.setBrightness(100);    // Reduce brightness..not sure I need this in the end.
            dotStrip.show();                // Send info to LEDs.. 
        }    
    }
}

void colorAllDot(int r, int g, int b) {         // Function to color the entire strip a certain color                                     //
    //for(i=0; i<dotStrip.numPixels(); i++) {   // Having some intermittent issues where the strip leaves one LED in a different color..debugging
    for(int i=0; i<73; i++) {                   // Basically it runs through all the LEDs one by one and assigns it a color..
        dotStrip.setPixelColor(i, r, g, b);
    }
    dotStrip.show();                            // Send info to LEDs
}


void volumeU(int v){                            // Volume up function
    for(int y=0; y<v; y++) {
        digitalWrite(volumeUp, LOW);            // Basically this sets the volume up pin low for 5 ms, then sets it high again
        delay(5);
        digitalWrite(volumeUp, HIGH);
    }
}

void volumeD(int v){ // Fade out
    for(int y=v; y>0; y--) {
        digitalWrite(volumeDown, LOW);          // Basically this sets the volume down pin low for 5 ms, then sets it high again
        delay(5);
        digitalWrite(volumeDown, HIGH);
    }
}


void lightsOff(){                               // Function to clear the lights..having issues with this as well, and that indicates issues with the library..
    dotStrip.clear();                           // Initialize pins for output
    dotStrip.show();                            // Turn all LEDs off ASAP
}

void resetter(){                                // Mostly used now for development to have a way to kill all..
    lightsOff();                                // Off with all lights
    setAudioPinsHigh();                         // Off with all sound
    looper = 999;                               // Bring us out of any if statements in void loop.. 
}

void setAudioPinsHigh(){                        // To kill all sound, just set all audio pins HIGH..
    digitalWrite(audioPin00, HIGH);             // Wanted to do this in a for loop, but wasn't able to..might revisit.. 
    digitalWrite(audioPin01, HIGH);
    digitalWrite(audioPin02, HIGH);
    digitalWrite(audioPin03, HIGH);
    digitalWrite(audioPin04, HIGH);
    digitalWrite(audioPin05, HIGH);
    digitalWrite(audioPin06, HIGH);
    digitalWrite(audioPin07, HIGH);
    digitalWrite(audioPin08, HIGH);
    digitalWrite(audioPin09, HIGH);
    digitalWrite(audioPin10, HIGH);
}
