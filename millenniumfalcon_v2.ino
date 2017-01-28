

// Get this show started, we will use Neopixels in this version
#include "application.h"        // Not 100% sure what this is..
#include "neopixel/neopixel.h"  // Neopixel Library

// Set Photon to correct mode
SYSTEM_MODE(SEMI_AUTOMATIC);

// Pin Definitions
// Audio Triggers
#define audioPin00  D6  // Left Right Left Right
#define audioPin01  D5  // Some random Falcon sound
#define audioPin02  D4  // Short takeoff
#define audioPin03  D3  //
#define audioPin04  D2  //
#define audioPin05  D1  //
#define audioPin06  D0  //
#define audioPin07  A3  //
#define audioPin08  A2  //  
#define audioPin09  A1  //
#define audioPin10  A0  //

// LED Strip
#define pixelPin    DAC // Plan tao use juse a long LED strip, and assign a certain portion to the engine, cockpit etc. 

// Misc pins
#define relPin      WKP // Controlling the relay
#define irSensor    RX  // For when I get there.. 

// Volume - issues..looking at other option
#define volumeUp    A5
#define volumeDown  A4

// Audio Reset
#define audioRst    D7  // To give me the possibilty to reset the board as it tends to get "weird"

// Define Constants
#define numPix    4  // Number of LEDs the engine

// Defining the pixel type on the strips (same on both..)
#define pixType WS2812B

// Define the Neopixel strip
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numPix, pixelPin, pixType);

String prevCmd;                 // Right..gotta set the previously used pins low again..
int wait = 1000;
int looper = 0;               // So we don't enter any if statements in the void loop :)
int audio = 0;                  // Used to set state so audio files are only triggered once
int ledState = 1;
String lowPin;
unsigned long startTime = 0;    // For timing lights and sound
unsigned long sequenceStart;    // For timing lights and sound

unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long interval = 1000;



void setup(){
    Particle.function("falcon", falcon);    // The cloud exposed function
    
    pinMode(pixelPin, OUTPUT);              // Defining the neopixel pin as output
   
    pinMode(audioRst, OUTPUT);              // Defining the audio reset pin as output
    digitalWrite(audioRst, HIGH);           // Setting it high so as to not trigger, yet.. :) 
   
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

    pinMode(volumeUp, OUTPUT);              // Volume pin defined as output
    digitalWrite(volumeUp, HIGH);           // Again, the nature of the audio board requires "off" pins to be high. 
    pinMode(volumeDown, OUTPUT);
    digitalWrite(volumeDown, HIGH);
    
    pinMode(relPin, OUTPUT);
    digitalWrite(relPin, LOW);
    
    pixels.begin();                       // Initialize pins for output
    pixels.show();                        // Turn all LEDs off ASAP, this does not allways work for some reason
}

int falcon(String cmd) {
    
    // Management Commands
    if (cmd == "relayOn") relay("on");
    if (cmd == "relayOff") relay("off");
    if (cmd == "reset") resetAudio();
    if (cmd == "lightsOff") lightsOff();
    if (cmd == "up"){           // Volume UP by 5
        volumeU(5);
    }
    if (cmd == "down"){         // Volume DOWN by 5
        volumeD(5);
    }
    if (cmd == "allReset") {         // All Stop..mostly meant for use during development
        looper = 0;
        pinFixer(lowPin);
        lightsOff();            // Lights out!
        relay("off");           // Stop power to amp
        resetAudio();           // Stop sound
    }    
    
    // Display Commands
    if (cmd == "onBasic") {     // This will become the default when switched on. Enigne on, no sound.
        relay("off");             
        onBasic();
    }
    if (cmd == "onAdvanced"){   // This will become the default when switched on. Enigne on, some random sounds
        relay("on");
        onAdvanced();
    }
    if (cmd == "takeOff"){      // One of the special sequences requiring timing to work as indended
        audio = 0;
        prevCmd = cmd;
        //cmd = "";
        relay("on");
        looper = 3;
        sequenceStart = millis(); 
        pinFixer(lowPin);
    }
    if (cmd == "hyperdrive"){   // One of the special sequences requiring timing to work as indended 
        looper = 4;
    }
    if (cmd == "annoying"){     // Silly test sequence..for development only 
        prevCmd = cmd;
        relay("on");
        looper = 5;
        sequenceStart = millis();
    }
    
    //Test    
    if (cmd == "00") { 
        digitalWrite(audioPin00, LOW);
        lowPin = "00";
        delay(wait);
        digitalWrite(audioPin00, HIGH);
    } 
    if (cmd == "01") {   
        digitalWrite(audioPin01, LOW);
        lowPin = "01";
        delay(wait);
        digitalWrite(audioPin01, HIGH);
    }    
    if (cmd == "02") {        
        digitalWrite(audioPin02, LOW);
        lowPin = "02";
        delay(wait);
        digitalWrite(audioPin02, HIGH);
    }    
    if (cmd == "03") {      
        digitalWrite(audioPin03, LOW);
        lowPin = "03";
        delay(wait);
        digitalWrite(audioPin03, HIGH);
    }    
    if (cmd == "04") {      
        digitalWrite(audioPin04, LOW);
        lowPin = "04";
        delay(wait);
        digitalWrite(audioPin04, HIGH);
    }    
    if (cmd == "05") {     
        digitalWrite(audioPin05, LOW);
        lowPin = "05";
        delay(wait);
        digitalWrite(audioPin05, HIGH);
    }    
    if (cmd == "06") {       
        digitalWrite(audioPin06, LOW);
        lowPin = "06";
        delay(wait);
        digitalWrite(audioPin06, HIGH);
    }    
    if (cmd == "07") {        
        digitalWrite(audioPin07, LOW);
        lowPin = "07";
        delay(wait);
        digitalWrite(audioPin07, HIGH);
    }    
    if (cmd == "08") {         
        digitalWrite(audioPin08, LOW);
        lowPin = "08";
        delay(wait);
        digitalWrite(audioPin08, HIGH);
    }    
    if (cmd == "09") {         
        digitalWrite(audioPin09, LOW);
        lowPin = "09";
        delay(wait);
        digitalWrite(audioPin09, HIGH);
    }    
    if (cmd == "10") {         
        digitalWrite(audioPin10, LOW);
        lowPin = "10";
        delay(wait);
        digitalWrite(audioPin10, HIGH);
    }
return 1;   // Returns 1 so we know all went well.. 
}


void loop(){                    // The loop that runs all the time.. looper = 0 reserved
    connect();

    if (looper == 3){           // takeOff, some work done here. Not done.
        takeOff();
    }

    if (looper == 5){           // The annoying sequence used for testing.. 
        leftrightleft();        // Development only, this triggers the leftrightleftright audio file
        flashybit();            // Silly light sequence
    }
}


// Display Functions
void leftrightleft(){               // Voice going left right left, but I got mono so left left left left...annoying!
    pinResetFast(audioPin00);       // Pin triggered
}

void flashybit(){
    unsigned long currentMillis = millis();
    
    if(currentMillis - previousMillis > interval) {
    
    // save the last time you changed color
    previousMillis = currentMillis;   
    
    if (ledState == 1){
        pixels.setBrightness(170);
        colorEngine(255,0,0); // Red
        ledState = 2;
    }else if (ledState == 2){
        pixels.setBrightness(170);
        colorEngine(0,0,255); // Blue?
        ledState = 1;
    }
  }
}

void onBasic(){
    colorEngine(0,0,255);           // The initial blue
}

void onAdvanced(){
    colorEngine(0,0,255);           // The initial blue
}

void takeOff (){                            // Very early attempt at a take off sequence..basically just found out how to do the timing
    unsigned long currentMillis = millis();
    if (audio == 0){                        // I just want the audio pin to trigger ONCE..so I have to create some sort of controller. audio was set to 0 during power up
        digitalWrite(audioPin02, LOW);      // Trigger audio
        lowPin = "02";
        audio = 1;                          // Set audio to 1 as the sound has been triggered..no need to do it again
        if (currentMillis - sequenceStart >1000){
            digitalWrite(audioPin02, HIGH);
        }
    }
    if (audio == 1){                        // Since audio has started, we need to start the lights
        if (currentMillis - sequenceStart <= 5780){     // The MF engine lights are darker blue for a bit, then as the engines "explode" to life, they turn a different color..the audio file is about 5610 ms in when this happens..
            colorEngine(0,0,255);           // The initial blue
            pixels.show();                  // Send info to LEDs..
        }else if((currentMillis - sequenceStart > 5780) && (currentMillis - sequenceStart < 10000)){
            colorEngine(67,67,255);         // After a set amount of time we need a different color as the engine lights should change
            pixels.setBrightness(100);    // Reduce brightness..not sure I need this in the end.
            pixels.show();                // Send info to LEDs.. 
        } else if(currentMillis - sequenceStart > 10000){
            relay("off");
        }    
    }
}

void colorEngine(int r, int g, int b) {         // Function to color the engine portion of the strip
    for(int i=0; i<4; i++) {                    // Set aside the first 5 pixels for the engine during testing, think this will be 17 in the end.. 
        pixels.setPixelColor(i, r, g, b);
    }
    pixels.show();                            // Send info to LEDs
}

void colorCockpit(){
    // Not sure what to put here yet. Should cycle through red, blue, white, yellow, green..random..at a set speed
}

void externalLights(){
    // Again, just a placeholder for future use..this time the external lighting. 
}


// Management Functions
void pinFixer(String lowPin){ // Remember to add the pins here that are used for audio..
    if (lowPin == "00") pinSetFast(audioPin00);
    if (lowPin == "01") pinSetFast(audioPin01);
    if (lowPin == "02") pinSetFast(audioPin02);
    if (lowPin == "03") pinSetFast(audioPin03);
    if (lowPin == "04") pinSetFast(audioPin04);
    if (lowPin == "05") pinSetFast(audioPin05);
    if (lowPin == "06") pinSetFast(audioPin06);
    if (lowPin == "07") pinSetFast(audioPin07);
    if (lowPin == "08") pinSetFast(audioPin08);
    if (lowPin == "09") pinSetFast(audioPin09);
    if (lowPin == "10") pinSetFast(audioPin10);
}

void volumeU(int v){                            // Volume up function
    for(int y=0; y<v; y++) {          // Basically this sets the volume up pin low for 5 ms, then sets it high again
        pinResetFast(volumeUp);
        delay(5);
        pinSetFast(volumeUp);
    }
}

void volumeD(int v){                            // Volume down function
    for(int y=v; y>0; y--) {
        pinResetFast(volumeDown);        // Basically this sets the volume down pin low for 5 ms, then sets it high again
        delay(5);
        pinSetFast(volumeDown);
    }
}

void lightsOff(){                               // Function to clear the lights..having issues with this as well, and that indicates issues with the library..
    pixels.clear();                           // Initialize pins for output
    pixels.show();                        // Turn all LEDs off ASAP, this does not allways work for some reason
}

void relay(String cmd){                 // The relay that controls the power to the amp takes a string command, on or off
    if (cmd == "off"){                   // Reason for this is to remove static hum when displaying only lights
        pinResetFast(relPin);
    }else if(cmd == "on"){
        pinSetFast(relPin);
    }
}


void resetAudio(){                  // Trips the rst pin on the audio fx board. Used to stop playing of files mid-flight
    pinResetFast(audioRst);
    delay(wait);
    pinSetFast(audioRst);
}

void connect() {
  if (Particle.connected() == false) {  // If not connected, then.. 
    Particle.connect();                 // connect!
  }
}
