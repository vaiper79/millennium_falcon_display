// This #include statement was automatically added by the Particle IDE.
#include <neopixel.h>

/*
Remaining items:
Takeoff sequence, needs some planning in terms of what the engine should look like for the various sounds..
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

// Variables
int looper = 0;                     // So we don't enter any if statements in the void loop :)
int audio = 0;                      // Used to set state so audio files are only triggered once
unsigned long takeOffStart;

const int cockpitBrightness = 30;                     // 255 is waaay to bright. Go below 50 to be able to see details in the cockpit
const int minEngineBrightness = 20;
const int maxEngineBrightness = 50;
int brightnessEven = maxEngineBrightness/2;     // how bright the LED is
int brightnessOdd = maxEngineBrightness/2;      // how bright the LED is

int fadeAmountEven = 1;                         // how many points to fade the LED by
int fadeAmountOdd = -1;
int fadeAmount = 1;
int brightness = 0;
const int maxBrightness = 100;

// States and the like
int boot = 0;
int blink = 1;
int bob = 0;

int cockpitLedState = 1;
int currentButtonState;
int previousButtonState = HIGH;
int longPress = 0; 

String cmd;
String previousCmd;
String previousAudioCmd;

// Timing variables
unsigned long pressedMillis;
unsigned long startTime = 0;        // For timing lights and sound..need to be more specific..
unsigned long sequenceStart;        // For timing lights and sound
unsigned long previousMillis = 0;       
unsigned long previousEngineMillis = 0;       
unsigned long previousCockpitMillis = 0;
unsigned long cmdMillis = 0;

const long cockpitInterval = 200; //500
const long flashInterval = 5;
const long engineInterval = 20;
const long interval = 2000;

// Temp variables..testing etc
int wait = 1000;

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
    if (cmd == "basic"){
        looper = 1;
    }
    if (cmd == "intro"){
        cmdMillis = millis();
        relay("on");
        pinResetFast(audioPin01);
        looper = 2;        
    }
    if (cmd == "medley"){
        cmdMillis = millis();
        relay("on");
        pinResetFast(audioPin02);
        looper = 3;        
    }
    if (cmd == "takeOff"){
        takeOffStart = millis(); // To get a time for when this started
        looper = 4;
    }
    
    // Admin's "special" Commands..shh..secret!
    if(cmd == "reset"){ // Need a quick way to kill the thing when working at night if it suddenly plays at high volume.. 
        stopAll();
    }
    if(cmd == "resetAudio"){ // Need a quick way to kill the thing when working at night if it suddenly plays at high volume.. 
        resetAudio();        
    }
    if(cmd == "resetRelay"){ // Need a quick way to kill the thing when working at night if it suddenly plays at high volume.. 
        relay("off");       
    }
    if(cmd == "resetLights"){ // Need a quick way to kill the thing when working at night if it suddenly plays at high volume.. 
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
    return 1;   // Returns 1 so we know all went well..cloud stuff 
}



void loop(){                    // The loop that runs all the time.. looper = 0 reserved
    connect();
    
    if (boot == 0){                                             // Do these things only once, at boot
        Particle.publish("Admin:" ,"Booted or reset, ready for duty");
        volumeD(25); // TEMPORARY, working at night config
        boot = 1;
    }
    
    button(); // Check the button on the base; short press = switch program, long press = turn off sound/lights
    
    if (looper == 1){ //static display
        staticDisplay();
    }
    if (looper == 2){ // intro
        soundDisplay("intro");
    }
    if (looper == 3){ // medley
        soundDisplay("medley");
    }
    if (looper == 4){ // take off
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
        }
    }    
    if (audioCmd == "medley") {
        if (currentMillis - cmdMillis >= 527000) { // 527000 Let audio play until finished, then shut down the relay..
            stopAll();
        }
    }
}        // Complete

void stopAll(){
    lightsOff();                                    // Kill lights
    relay("off");                                   // Turn off power to the amplifier
    resetAudio();                                   // Reset the audio card to kill any playing audio..
    pinFixer("all");                                // Shut down the output pin on ctrlr towrards audiofx..not sure this works

    // Variables to reset 
    looper = 0;
    cmd = "0";
    boot = 0;
    fadeAmount = 1;
    brightnessEven = 25; 
    brightnessOdd = 25;
    blink = 1;
    bob = 0;
    fadeAmountOdd = -1;
    fadeAmountEven = 1;
    cmdMillis = 0;
    
    volumeD(25);                                    // TEMPORARY, working at night config
}                         // Complete

void button(){                                                  // Button press= LOW, LED lit = HIGH
    unsigned long currentMillis = millis();
    
    currentButtonState = digitalRead(buttonPress);              // Check if button is pressed at this moment
    
    if (currentButtonState == LOW) {                            // If the button IS pressed..then
        if (currentButtonState == previousButtonState){         // Check if it was pressed last time we checked..if it was, then
            if (currentMillis - pressedMillis >= interval) {    // Check if it has been pressed for the duration of "interval", if yes, then
                longPress = 1;                                  // Button has been pressed >= interval, this ensures we drop out gracefully of the button function
                falcon("reset");                                // Why not use the cloud exposed function for something good, this resets everything back to nothing.
            }
        } else {                                                // If interval hasn't been reached then
            pressedMillis = millis();                           // When did we press the button..
            previousButtonState = currentButtonState;           // Record the button state..
            digitalWrite(buttonLED, HIGH);                      // Light the LED on the button :)
        }
    } else if (currentButtonState == HIGH){                     // The most common state, button NOT pressed..
        if (previousButtonState == LOW) {                       // However, what if the button was pressed on the last run through? This checks to see if it has been released
            if (longPress == 1){                                // and what if we pressed it a really really really long time?
                longPress = 0;                                  // Well..we're not pressing it for a really really really long time any more.. 
            }else{
                falcon("reset");                                // because if that is the case then we need to stop what we're doing and see what
                looper = looper + 1;                            // increase the looper variable to move to the next program in line
                
                if (looper == 1) falcon("basic");               // Commands to be executed..
                if (looper == 2) falcon("intro");
                if (looper == 3) falcon("medley");
                if (looper == 4) falcon("takeOff");
                
                if (looper == 5) looper = 0;                    // and make sure we loop looper around when we have nothing more to show off :)
            }
            previousButtonState = currentButtonState;           // Record the button state for the next run around.. 
        }
        digitalWrite(buttonLED, LOW);                           // Turn of the button LED
    }
}                          // Complete

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
}                   // Complete

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
    
    playSound("takeOff"); // Start the sound
    
    unsigned long currentMillis = millis(); // Current time
    
    unsigned long millisSinceStart = currentMillis - takeOffStart; // How long has this sequence been running
    
    if ((millisSinceStart >= 0) && (millisSinceStart < staticCocpit)) {
        pixels.setPixelColor(1, 30, 0, 0);
        pixels.setPixelColor(2, 30, 30, 20);
        pixels.setPixelColor(3, 0, 0, 20);
    }
    
    // Cockpit lights
    if (millisSinceStart >= staticCocpit) {
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
                    bob = 1;
                }
                
                // change the brightness for next time through the loop:
                brightness = brightness + fadeAmount;
                
                // Engine pulsating..brute..
                for (int i=4; i <= 10; i++){
                    pixels.setPixelColor(i, brightness, brightness, brightness); // Even numbers
                }
                if (bob == 1){
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
    }
    pixels.show();
}                        // WIP

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
}                    // Complete

void volumeD(int v){                    // Volume down function
    for(int y=v; y>0; y--) {            // Basically this sets the volume down pin low for 20 ms, then sets it high again
        pinResetFast(volumeDown);        
        delay(20);
        pinSetFast(volumeDown);
        delay(20);
    }
}                    // Complete

void lightsOff(){                               // Function to clear the lights..having issues with this as well, and that indicates issues with the library..
    pixels.clear();                           // Initialize pins for output
    pixels.show();                        // Turn all LEDs off ASAP, this does not allways work for some reason
}                       // Complete

void relay(String relayCmd){                 // The relay that controls the power to the amp takes a string command, on or off
    if (relayCmd == "off"){                   // Reason for this is to remove static hum when displaying only lights
        pinResetFast(relPin);
    }else if(relayCmd == "on"){
        pinSetFast(relPin);
    }
}            // Complete

void resetAudio(){                  // Trips the rst pin on the audio fx board. Used to stop playing of files mid-flight
    pinResetFast(audioRst);
    delay(wait);
    pinSetFast(audioRst);
}                      // Complete

void pinFixer(String audioCmd){                         // Remember to add the pins here that are used for audio..
    if (audioCmd == "takeOff") pinSetFast(audioPin00);  // Selectively putting pins LOW
    if (audioCmd == "intro") pinSetFast(audioPin01);
    if (audioCmd == "medley") pinSetFast(audioPin02);
    if (audioCmd == "all"){                             // nice to have the option..
        pinSetFast(audioPin00);
        pinSetFast(audioPin01);
        pinSetFast(audioPin02);
    }
}           // Complete

void connect() {
  if (Particle.connected() == false) {  // If not connected, then.. 
    Particle.connect();                 // connect!
  }
}                        // Complete
