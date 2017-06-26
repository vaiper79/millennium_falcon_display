/*
The finished (version 1) sketch to command the Bandai Millennium Falcon 1/144th scale model, adding lights and sound.

A medley of code, some my own, much copied and modified from:

- How to avoid using delay while waiting for a certain point in time: https://www.arduino.cc/en/Tutorial/BlinkWithoutDelay
- Photon + Neopixel controls: https://github.com/technobly/Particle-NeoPixel
- Hold button code: https://playground.arduino.cc/Code/HoldButton

Credit where credit is due, hopefully I haven't forgotten anyone. For general ideas and very good how-to's I have often gone to http://adafruit.com. 

*/

// Get this show started, we will use Neopixels in this version
#include "application.h"        // Not 100% sure what this is..
#include <neopixel.h>

// Set Photon to correct mode
SYSTEM_MODE(SEMI_AUTOMATIC);

// Pin Definitions
// Audio Triggers
#define audioPin00  D6  // T00 full take off sound  125000 ms
#define audioPin01  D5  // T01 intro                386000 ms    
#define audioPin02  D4  // T02 medley               527000 ms
// LEDs
#define pixelPin    WKP // Plan tao use juse a long LED strip, and assign a certain portion to the engine, cockpit etc.
// Misc pins
#define relPin          DAC     // Controlling the relay
#define buttonPress     RX      // Input pin (pull_down)
#define buttonLED       TX      // Output pin
// Volume
#define volumeUp    A5
#define volumeDown  A4
// Audio Reset
#define audioRst    D7  // To give me the possibilty to reset the board as it tends to get "weird"
// Define Constants
#define numPix      11  // Number of LEDs the engine (0 = Headlights, 1-3 = Cockpit, 4-10 = Engine)
// Defining the pixel type on the strips (same on both..)
#define pixType     WS2812B

// Define the Neopixel strip
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numPix, pixelPin, pixType);

// Variables & constants
int looper = 0;                                     // To tell what sequence to run in the void loop function. 

// LED variables & constants
const int cockpitBrightness = 30;                     // 255 is waaay to bright. Go below 50 to be able to see details in the cockpit
const int minEngineBrightness = 20;
const int maxEngineBrightness = 50;
const int maxBrightness = 100;
int brightnessEven = maxEngineBrightness/2;     // how bright the LED is
int brightnessOdd = maxEngineBrightness/2;      // how bright the LED is
int fadeAmountEven = 1;                         // how many points to fade the LED by
int fadeAmountOdd = -1;
int fadeAmount = 1;
int brightness = 0;


// States and the like
int boot = 0;
int blink = 1;
int maxValueReached = 0;
byte cockpitLedState = 1;

// Timing variables & constants
unsigned long previousMillis = 0;
unsigned long previousButtonMillis = 0;
unsigned long previousEngineMillis = 0;       
unsigned long previousCockpitMillis = 0;
unsigned long cmdMillis = 0;
unsigned long takeOffStart;
const long cockpitInterval = 200; //500
const long buttonInterval = 500;    // How long the button is lit after being pressed
const long flashInterval = 5;
const long engineInterval = 20;
const long interval = 2000;
const int wait = 1000;

// Button Variables
long millis_held;    // How long the button was held (milliseconds)
long secs_held;      // How long the button was held (seconds)
long prev_secs_held; // How long the button was held in the previous check
unsigned long firstTime; // how long since the button was first pressed 
int currentButtonState;
byte previousButtonState = HIGH;

void setup(){
    Particle.function("falcon", falcon);    // The cloud exposed function
    pinMode(pixelPin, OUTPUT);              // Defining the neopixel pin as output
    pinMode(audioRst, OUTPUT);              // Defining the audio reset pin as output
    digitalWrite(audioRst, HIGH);           // Setting it high so as to not trigger, yet.. :) 
    pinMode(audioPin00, OUTPUT);            // Pins defined as outputs
    digitalWrite(audioPin00, HIGH);         // The set high due to the nature of the audio board (gnd triggers sound)
    pinMode(audioPin01, OUTPUT);
    digitalWrite(audioPin01, HIGH);
    pinMode(audioPin02, OUTPUT);
    digitalWrite(audioPin02, HIGH);
    pinMode(volumeUp, OUTPUT);              // Volume pin defined as output
    digitalWrite(volumeUp, HIGH);           // Again, the nature of the audio board requires "off" pins to be high. 
    pinMode(volumeDown, OUTPUT);
    digitalWrite(volumeDown, HIGH);
    pinMode(relPin, OUTPUT);                // LOW = relay is open and AMP has no power, HIGH = relay is closed and AMP has power
    digitalWrite(relPin, LOW);              // this is done to remove audio hum/hiss during non-audio display
    pinMode(buttonLED, OUTPUT);             // Setting up the button output LED pin
    pinMode(buttonPress, INPUT_PULLUP);     // Setting up the button switch input pin using PULLUP 
    pixels.begin();                         // Initialize pins for output
    pixels.show();                          // Turn all LEDs off ASAP, this does not allways work for some reason
    pinMode(buttonLED, OUTPUT);             // Setting up the button LED pin
}                           // Complete

int falcon(String cmd) {  // This function allows me to call commands as if they were sent via cloud..i.e. via the button
    // Control Commands...
    if (cmd == "basic"){            // Basic display, just the lights
        stopAll();                  // Function to stop all tasks
        resetter("partial");        // Function to reset all global variables
        looper = 1;                 // Determines what sequence to loop
    }
    if (cmd == "intro"){            // Basic display, but with the Star Wars theme playing
        stopAll();                  // Function to stop all tasks
        resetter("partial");        // Function to reset all global variables
        cmdMillis = millis();       // To get a time for when this started
        relay("on");                // We will need sound, so engage the amp relay
        pinResetFast(audioPin01);   // Start playing the correct sound
        looper = 2;                 // Determines what sequence to loop        
    }
    if (cmd == "medley"){           // Basic display but with a medley playing
        stopAll();                  // Function to stop all tasks
        resetter("partial");        // Function to reset all global variables
        cmdMillis = millis();       // To get a time for when this started
        relay("on");                // We will need sound, so engage the amp relay
        pinResetFast(audioPin02);   // Start playing the correct sound
        looper = 3;                 // Determines what sequence to loop        
    }
    if (cmd == "takeOff"){          // Lights animated to the sound of the MF taking off
        stopAll();                  // Function to stop all tasks
        resetter("partial");        // Function to reset all global variables
        takeOffStart = millis();    // To get a time for when this started
        looper = 4;                 // Determines what sequence to loop
    }
    if(cmd == "reset"){             // Need a quick way to kill the thing..
        stopAll();                  // Function to stop all tasks
        resetter("complete");       // Function to reset all global variables
    }
    
    
    // Admin's "special" Commands..shh..secret! Self explanatory?
    if(cmd == "resetAudio"){ 
        resetAudio();        
    }
    if(cmd == "resetLights"){  
        lightsOff();        
    }
    if(cmd == "relayOn"){
        relay("on");
    }
    if(cmd == "relayOff"){
        relay("off");
    }
    if(cmd == "up"){
        volumeU(5);
    }
    if(cmd == "down"){
        volumeD(5);
    }
    return 1;                   // Returns 1 so we know all went well..cloud stuff 
}



void loop(){                    // The loop that runs all the time.. looper = 0 reserved for nothing :) 
    connect();                  // Since we're using the SEMI_AUTOMATIC mode we need to manually connect to the wireless network and the cloud
    
    if (boot == 0){                                                     // Do these things only once, at boot (..oh and every time we loop around to the idle sequence)
        Particle.publish("Admin:" ,"Booted or reset, ready for duty");  // Just some text to publish, might refine this some day.. 
        //volumeD(25);                                                  // TEMPORARY, working at night config            
        boot = 1;                                                       // Setting this so we make sure we only excute this sequence once.. 
    }
    
    button();               // Check the button on the base; short press = switch program, long press = turn off sound/lights
    
    if (looper == 1){       //static display
        staticDisplay();    
    }
    if (looper == 2){       // intro
        soundDisplay("intro");
    }
    if (looper == 3){       // medley
        soundDisplay("medley");
    }
    if (looper == 4){       // take off
        takeOff();
    }
}                            // WIP

void playSound(String audioCmd){
    unsigned long currentMillis = millis();
    if (audioCmd == "takeOff") {
        relay("on");
        pinResetFast(audioPin00);
    } 
    if (audioCmd == "intro") { 
        if (currentMillis - cmdMillis >= 386000) { // 386000 Let audio play until finished, then shut down the relay..
            stopAll();
            resetter("complete");
        }
    }    
    if (audioCmd == "medley") {
        if (currentMillis - cmdMillis >= 527000) { // 527000 Let audio play until finished, then shut down the relay..
            stopAll();
            resetter("complete");
        }
    }
}        // Complete

void stopAll(){
    lightsOff();                    // Kill lights
    relay("off");                   // Turn off power to the amplifier
    resetAudio();                   // Reset the audio card to kill any playing audio..
    pinFixer("all");                // Shut down the output pin on ctrlr towrards audiofx..not sure this works
}

void resetter(String resetCmd){
    if (resetCmd == "partial"){
        fadeAmount = 1;
        brightnessEven = 25; 
        brightnessOdd = 25;
        blink = 1;
        maxValueReached = 0;
        fadeAmountOdd = -1;
        fadeAmountEven = 1;
        cmdMillis = 0;
    }
    if (resetCmd == "complete"){
        fadeAmount = 1;
        brightnessEven = 25; 
        brightnessOdd = 25;
        blink = 1;
        maxValueReached = 0;
        fadeAmountOdd = -1;
        fadeAmountEven = 1;
        cmdMillis = 0;
        looper = 0;                 // The variable that makes this reset the "complete" version. 
    }
}

void button() {
  currentButtonState = digitalRead(buttonPress);

  // if the button state changes to pressed, remember the start time 
  if (currentButtonState == LOW && previousButtonState == HIGH && (millis() - firstTime) > 200) {
    firstTime = millis();
  }

  millis_held = (millis() - firstTime);
  secs_held = millis_held / 1000;

  // This if statement is a basic debouncing tool, the button must be pushed for at least
  // 100 milliseconds in a row for it to be considered as a push.
  if (millis_held > 50) {

        // check if the button was released since we last checked
        if (currentButtonState == HIGH && previousButtonState == LOW) {
            
            // Button pressed for less than 2 seconds, go to the next thing on the list
            if (secs_held <= 1) {
                Particle.publish("Looper before:" ,looper);
                looper = looper + 1;
                buttonLight();
                
                if (looper == 1) falcon("basic");
                if (looper == 2) falcon("intro");
                if (looper == 3) falcon("medley");
                if (looper == 4) falcon("takeOff");
                if (looper == 5) {
                    stopAll();
                    resetter("complete");
                }
            }
            
            // Button held for more than 2 seconds, reset all 
            if (secs_held > 1) {
                ledblink(3,200,buttonLED);
                stopAll();
                resetter("complete");
            }
        }
    }
    digitalWrite(buttonLED, LOW); 
    previousButtonState = currentButtonState;
    prev_secs_held = secs_held;
}

void buttonLight(){
    unsigned long currentMillis = millis();
    if (currentMillis - previousButtonMillis >= buttonInterval) {
        previousButtonMillis = currentMillis;
        digitalWrite(buttonLED, HIGH);
    }else{
        digitalWrite(buttonLED, LOW);
    }
}

void staticDisplay(){
    unsigned long currentMillis = millis();
    pixels.setPixelColor(0, 128, 128, 128); // Headlights

    // Cockpit blinking..brute coding
    if (currentMillis - previousCockpitMillis >= cockpitInterval) {

        previousCockpitMillis = currentMillis;

        if (cockpitLedState == 1) {
            pixels.setPixelColor(1, cockpitBrightness, 0, 0);
            pixels.setPixelColor(2, cockpitBrightness, cockpitBrightness, cockpitBrightness);
            pixels.setPixelColor(3, 0, 0, cockpitBrightness-10);
            cockpitLedState = 2;
        } else if(cockpitLedState == 2) {
            pixels.setPixelColor(2, cockpitBrightness, 0, 0);
            pixels.setPixelColor(3, cockpitBrightness, cockpitBrightness, cockpitBrightness);
            pixels.setPixelColor(1, 0, 0, cockpitBrightness-10);
            cockpitLedState = 3;
        } else if(cockpitLedState == 3) {
            pixels.setPixelColor(3, cockpitBrightness, 0, 0);
            pixels.setPixelColor(1, cockpitBrightness, cockpitBrightness, cockpitBrightness);
            pixels.setPixelColor(2, 0, 0, cockpitBrightness-10);
            cockpitLedState = 1;
        }
        pixels.show();
    }

    if (currentMillis - previousEngineMillis >= engineInterval) {

        previousEngineMillis = currentMillis;
        // reverse the direction of the fading at the ends of the fade:
        if (brightnessEven == minEngineBrightness || brightnessEven == maxEngineBrightness) {
            fadeAmountEven = -fadeAmountEven;
        }
        if (brightnessOdd == maxEngineBrightness || brightnessOdd == minEngineBrightness) {
            fadeAmountOdd = -fadeAmountOdd;
        }
        
        // change the brightness for next time through the loop:
        brightnessEven = brightnessEven + fadeAmountEven;
        brightnessOdd = brightnessOdd + fadeAmountOdd;
        
        // Engine pulsating..brute..
        pixels.setPixelColor(4, brightnessEven, brightnessEven, brightnessEven); // Even numbers
        pixels.setPixelColor(6, brightnessEven, brightnessEven, brightnessEven);
        pixels.setPixelColor(8, brightnessEven, brightnessEven, brightnessEven);   
        pixels.setPixelColor(10, brightnessEven, brightnessEven, brightnessEven);
        pixels.setPixelColor(5, brightnessOdd, brightnessOdd, brightnessOdd); // Odd numbers
        pixels.setPixelColor(7, brightnessOdd, brightnessOdd, brightnessOdd);
        pixels.setPixelColor(9, brightnessOdd, brightnessOdd, brightnessOdd);    
    } 
    pixels.show();
}

void takeOff (){  
    
    // Timings
    unsigned long staticCocpit = 2000;
    unsigned long firstBlink = 4450; 
    unsigned long secondBlink = 7000;
    unsigned long thirdBlink = 9400;
    unsigned long fourthBlink = 10600;
    unsigned long fifthBlink = 12100;
    unsigned long engineOn = 14100;
    unsigned long engineBright = 20000;
    unsigned long normalEngine = 21900;
    unsigned long end = 125000;
    unsigned long currentMillis = millis(); // Current time
    unsigned long millisSinceStart = currentMillis - takeOffStart; // How long has this sequence been running
    
    playSound("takeOff"); // Start the sound
    
    if ((millisSinceStart >= 0) && (millisSinceStart < staticCocpit)) {
        pixels.setPixelColor(1, 30, 0, 0);
        pixels.setPixelColor(2, 30, 30, 20);
        pixels.setPixelColor(3, 0, 0, 20);
    }
    
    // Cockpit lights
    if (millisSinceStart >= staticCocpit) {
        pixels.setPixelColor(0, 128, 128, 128); // Headlights
        if (currentMillis - previousCockpitMillis >= cockpitInterval) {
            // save the last time you blinked the LED
            previousCockpitMillis = currentMillis;
    
            if (cockpitLedState == 1) {
                pixels.setPixelColor(1, cockpitBrightness, 0, 0);
                pixels.setPixelColor(2, cockpitBrightness, cockpitBrightness, cockpitBrightness);
                pixels.setPixelColor(3, 0, 0, cockpitBrightness-10);
                cockpitLedState = 2;
            } else if(cockpitLedState == 2) {
                pixels.setPixelColor(2, cockpitBrightness, 0, 0);
                pixels.setPixelColor(3, cockpitBrightness, cockpitBrightness, cockpitBrightness);
                pixels.setPixelColor(1, 0, 0, cockpitBrightness-10);
                cockpitLedState = 3;
            } else if(cockpitLedState == 3) {
                pixels.setPixelColor(3, cockpitBrightness, 0, 0);
                pixels.setPixelColor(1, cockpitBrightness, cockpitBrightness, cockpitBrightness);
                pixels.setPixelColor(2, 0, 0, cockpitBrightness-10);
                cockpitLedState = 1;
            }
        }
    }
    
    // Engine..
    
    // 5 Engine flashes before proper start
    if ((millisSinceStart >= firstBlink) && (millisSinceStart < secondBlink)) {
        if (blink == 1){
            if (currentMillis - previousEngineMillis >= flashInterval) {
                // save the last time you blinked the LED
                previousEngineMillis = currentMillis;
                // reverse the direction of the fading at the ends of the fade:
                if (brightness == maxBrightness) {
                    fadeAmount = -fadeAmount ;
                }
                
                // change the brightness for next time through the loop:
                brightness = brightness + fadeAmount;
                
                // Engine pulsating..brute..
                for (int i=4; i <= 10; i++){
                    pixels.setPixelColor(i, brightness, brightness, brightness); // Even numbers
                }
                if (brightness == 0) {
                    blink = 2;
                    fadeAmount = 1;
                }
            } 
        }
    }
    if ((millisSinceStart >= secondBlink) && (millisSinceStart < thirdBlink)) {
        if (blink == 2){
            if (currentMillis - previousEngineMillis >= flashInterval) {
                // save the last time you blinked the LED
                previousEngineMillis = currentMillis;
                // reverse the direction of the fading at the ends of the fade:
                if (brightness == maxBrightness) {
                    fadeAmount = -fadeAmount ;
                }
                
                // change the brightness for next time through the loop:
                brightness = brightness + fadeAmount;
                
                // Engine pulsating..brute..
                for (int i=4; i <= 10; i++){
                    pixels.setPixelColor(i, brightness, brightness, brightness); // Even numbers
                }
                if (brightness == 0) {
                    blink = 3;
                    fadeAmount = 1;
                }
            } 
        }
    }   
    if ((millisSinceStart >= thirdBlink) && (millisSinceStart < fourthBlink)) {
        if (blink == 3){        
            if (currentMillis - previousEngineMillis >= flashInterval) {
                // save the last time you blinked the LED
                previousEngineMillis = currentMillis;
                // reverse the direction of the fading at the ends of the fade:
                if (brightness == maxBrightness) {
                    fadeAmount = -fadeAmount ;
                }
                
                // change the brightness for next time through the loop:
                brightness = brightness + fadeAmount;
                
                // Engine pulsating..brute..
                for (int i=4; i <= 10; i++){
                    pixels.setPixelColor(i, brightness, brightness, brightness); // Even numbers
                }  
                if (brightness == 0) {
                    blink = 4;
                    fadeAmount = 1;
                }
            }
        }
    } 
    if ((millisSinceStart >= fourthBlink) && (millisSinceStart < fifthBlink)) {
        if (blink == 4){           
            if (currentMillis - previousEngineMillis >= flashInterval) {
                // save the last time you blinked the LED
                previousEngineMillis = currentMillis;
                // reverse the direction of the fading at the ends of the fade:
                if (brightness == maxBrightness) {
                    fadeAmount = -fadeAmount ;
                }
                
                // change the brightness for next time through the loop:
                brightness = brightness + fadeAmount;
                
                // Engine pulsating..brute..
                for (int i=4; i <= 10; i++){
                    pixels.setPixelColor(i, brightness, brightness, brightness); // Even numbers
                }  
                if (brightness == 0) {
                    blink = 5;
                    fadeAmount = 1;
                }
            } 
        }
    } 
    if ((millisSinceStart >= fifthBlink) && (millisSinceStart < engineOn)) {
        if (blink == 5){
            if (currentMillis - previousEngineMillis >= flashInterval) {
                // save the last time you blinked the LED
                previousEngineMillis = currentMillis;
                // reverse the direction of the fading at the ends of the fade:
                if (brightness == maxBrightness) {
                    fadeAmount = -fadeAmount ;
                }
                
                // change the brightness for next time through the loop:
                brightness = brightness + fadeAmount;
                
                // Engine pulsating..brute..
                for (int i=4; i <= 10; i++){
                    pixels.setPixelColor(i, brightness, brightness, brightness); // Even numbers
                }
                if (brightness == 0) {
                    blink = 6;
                    fadeAmount = 1;
                }
            }
        }
    } 
    
    // Engine starts..kind of
    if ((millisSinceStart >= engineOn) && (millisSinceStart < engineBright)) {
        // Engine lights up
        if (blink == 6){
            if (currentMillis - previousEngineMillis >= flashInterval) {
                // save the last time you blinked the LED
                previousEngineMillis = currentMillis;

                // change the brightness for next time through the loop:
                brightness = brightness + fadeAmount;
                
                // Engine pulsating..brute..
                for (int i=4; i <= 10; i++){
                    pixels.setPixelColor(i, brightness, brightness, brightness); // Even numbers
                }
                if (brightness == maxBrightness) {
                    blink = 7;
                    fadeAmount = 1;
                }
            }
        }
    } 
    
    // Bright flash..
    if ((millisSinceStart >= engineBright) && (millisSinceStart < normalEngine)) {
        // bright flash that fades down to normal brightness
        if (blink == 7){
            if (currentMillis - previousEngineMillis >= flashInterval) {
                // save the last time you blinked the LED
                previousEngineMillis = currentMillis;
                
                if (brightness == maxBrightness+100) {
                    fadeAmount = -fadeAmount;
                    maxValueReached = 1;
                }
                
                // change the brightness for next time through the loop:
                brightness = brightness + fadeAmount;
                
                // Engine pulsating..brute..
                for (int i=4; i <= 10; i++){
                    pixels.setPixelColor(i, brightness, brightness, brightness); // Even numbers
                }
                if (maxValueReached == 1){
                    if (brightness == maxEngineBrightness-5) {
                        blink = 0;
                        fadeAmount = 1;
                        brightnessEven = maxEngineBrightness-5;
                        brightnessOdd = maxEngineBrightness-5;
                    }
                }
            }
        }
    }    
    
    // Normal engine running
    if (millisSinceStart >= normalEngine) {
        if (currentMillis - previousEngineMillis >= engineInterval) {
            // save the last time you blinked the LED
            previousEngineMillis = currentMillis;
            // reverse the direction of the fading at the ends of the fade:
            if (brightnessEven == minEngineBrightness || brightnessEven == maxEngineBrightness) {
                fadeAmountEven = -fadeAmountEven ;
            }
            if (brightnessOdd == maxEngineBrightness || brightnessOdd == minEngineBrightness) {
                fadeAmountOdd = -fadeAmountOdd ;
            }
            
            // change the brightness for next time through the loop:
            brightnessEven = brightnessEven + fadeAmountEven;
            brightnessOdd = brightnessOdd + fadeAmountOdd;
            
            // Engine pulsating..brute..
            pixels.setPixelColor(4, brightnessEven, brightnessEven, brightnessEven); // Even numbers
            pixels.setPixelColor(6, brightnessEven, brightnessEven, brightnessEven);
            pixels.setPixelColor(8, brightnessEven, brightnessEven, brightnessEven);   
            pixels.setPixelColor(10, brightnessEven, brightnessEven, brightnessEven);
            pixels.setPixelColor(5, brightnessOdd, brightnessOdd, brightnessOdd); // Odd numbers
            pixels.setPixelColor(7, brightnessOdd, brightnessOdd, brightnessOdd);
            pixels.setPixelColor(9, brightnessOdd, brightnessOdd, brightnessOdd);    
        } 
    }    
    
    // Done, stop!
    if (millisSinceStart >= end) { 
        stopAll();
        resetter("complete");
    }
    pixels.show();
}

void soundDisplay(String program){
    staticDisplay();
    if (program == "intro") playSound("intro");
    if (program == "medley") playSound("medley");
}      // Complete

void volumeU(int v){                    // Volume up function
    for(int y=0; y<v; y++) {            // Basically this sets the volume up pin low for 20 ms, then sets it high again
        pinResetFast(volumeUp);
        delay(20);
        pinSetFast(volumeUp);
        delay(20);
    }
}

void volumeD(int v){                    // Volume down function
    for(int y=v; y>0; y--) {            // Basically this sets the volume down pin low for 20 ms, then sets it high again
        pinResetFast(volumeDown);        
        delay(20);
        pinSetFast(volumeDown);
        delay(20);
    }
}

void lightsOff(){                               // Function to clear the lights..having issues with this as well, and that indicates issues with the library..
    pixels.clear();                           // Initialize pins for output
    pixels.show();                        // Turn all LEDs off ASAP, this does not allways work for some reason
}

void relay(String relayCmd){                 // The relay that controls the power to the amp takes a string command, on or off
    if (relayCmd == "off"){                   // Reason for this is to remove static hum when displaying only lights
        pinResetFast(relPin);
    }else if(relayCmd == "on"){
        pinSetFast(relPin);
    }
}

void resetAudio(){                  // Trips the rst pin on the audio fx board. Used to stop playing of files mid-flight
    pinResetFast(audioRst);
    delay(wait);
    pinSetFast(audioRst);
}

void pinFixer(String audioCmd){                         // Remember to add the pins here that are used for audio..
    if (audioCmd == "takeOff") pinSetFast(audioPin00);  // Selectively putting pins LOW
    if (audioCmd == "intro") pinSetFast(audioPin01);
    if (audioCmd == "medley") pinSetFast(audioPin02);
    if (audioCmd == "all"){                             // nice to have the option..
        pinSetFast(audioPin00);
        pinSetFast(audioPin01);
        pinSetFast(audioPin02);
    }
}

// Just a simple helper function to blink an led in various patterns
void ledblink(int times, int lengthms, int pinnum){
  for (int x=0; x<times;x++) {
    digitalWrite(pinnum, HIGH);
    delay (lengthms);
    digitalWrite(pinnum, LOW);
    delay(lengthms);
  }
}

void connect() {
  if (Particle.connected() == false) {  // If not connected, then.. 
    Particle.connect();                 // connect!
  }
}
