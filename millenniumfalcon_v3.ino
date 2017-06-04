/*
Remaining items:
Takeoff sequence
Sound display
*/

// Get this show started, we will use Neopixels in this version
#include "application.h"        // Not 100% sure what this is..
#include <neopixel.h>
//#include "neopixel/neopixel.h"  // Neopixel Library

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
#define relPin          DAC // Controlling the relay
#define buttonPress     RX // Input pin (pull_down)
#define buttonLED       TX // Output pin
// Volume - issues..looking at other option
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

int cockpitBrightness = 30;                 // 255 is waaay to bright. Go below 50 to be able to see details in the cockpit
int minEngineBrightness = 20;
int maxEngineBrightness = 50;
int brightnessEven = maxEngineBrightness/2;   // how bright the LED is
int brightnessOdd = maxEngineBrightness/2;    // how bright the LED is
int fadeAmountEven = 1;                     // how many points to fade the LED by
int fadeAmountOdd = -1;


// States and the like
int boot = 0;

int cockpitLedState = 1;
int currentButtonState;
int previousButtonState = HIGH;

String cmd;
String previousCmd;
String previousAudioCmd;

// Timing variables
unsigned long pressedMillis;
unsigned long cmdMillis;
unsigned long startTime = 0;        // For timing lights and sound..need to be more specific..
unsigned long sequenceStart;        // For timing lights and sound
unsigned long previousMillis = 0;       
unsigned long previousEngineMillis = 0;       
unsigned long previousCockpitMillis = 0;

const long cockpitInterval = 200; //500
const long engineInterval = 20;
const long interval = 2000;

// Tempt variables..testing etc
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

int falcon(String cmd) {
    // Control Commands...
    if(cmd == "basic"){
        looper = 1;
    }
    if(cmd == "intro"){
        looper = 2;
    }
    if(cmd == "medley"){
        looper = 3;
    }
    if(cmd == "takeOff"){
        looper = 4;
    }
    
    // Admin's "special" Commands..shh..secret!
    if(cmd == "reset"){ // Need a quick way to kill the thing when working at night if it suddenly plays at high volume.. 
        lightsOff(); 
        relay("off");
        resetAudio();
        pinFixer("all");
        looper = 0;
        cmd = "0";
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
    if (cmd == "1") { // WIP.. 
        looper = 11;
    }  
    if (cmd == "2") { // Not operational...
        looper = 12;
    }    
    if (cmd == "3") { // Not operational...
        looper = 13;
    }
    return 1;   // Returns 1 so we know all went well..cloud stuff 
}                // WIP

void loop(){                    // The loop that runs all the time.. looper = 0 reserved
    connect();
    
    if (boot == 0){
        Particle.publish("Admin:" ,"Booted, ready for duty");
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
    if (looper == 4){ //11
        playSound("1");
    }
    
    if (looper == 5){ //12
        playSound("2");
    }
    if (looper == 6){ //13
        playSound("3");
    }
    /*
    if (looper == 4){ // take off
        takeOff();
    }
    */
    Particle.publish("Looper:", String(looper));
}                            // WIP

void playSound(String audioCmd){
    unsigned long currentMillis = millis();
    if (audioCmd == "takeOff") {
        relay("on");
        pinResetFast(audioPin00);
        if (audioCmd == previousAudioCmd){
            if (currentMillis - cmdMillis >= 125000) { // 125000 Let audio play until finished, then shut down the relay..
                pinSetFast(audioPin00);
                relay("off");
                pinFixer(audioCmd);          // Shut down the output pin on ctrlr towrards audiofx.. 
            }
        } else {
            cmdMillis = millis();
            previousCmd = audioCmd;
        }
    } 
    if (audioCmd == "intro") { 
        relay("on");
        pinResetFast(audioPin01);
        if (cmd == previousAudioCmd){
            if (currentMillis - cmdMillis >= 386000) { // 125000 Let audio play until finished, then shut down the relay..
                pinSetFast(audioPin01);
                relay("off");
                pinFixer(audioCmd);          // Shut down the output pin on ctrlr towrards audiofx.. 
            }
        } else {
            cmdMillis = millis();
            previousCmd = audioCmd;
        }
    }    
    if (audioCmd == "medley") {
        relay("on");
        pinResetFast(audioPin02);
        if (audioCmd == previousAudioCmd){
            if (currentMillis - cmdMillis >= 527000) { // 125000 Let audio play until finished, then shut down the relay..
                pinSetFast(audioPin02);
                relay("off");
                pinFixer(audioCmd);          // Shut down the output pin on ctrlr towrards audiofx.. 
            }
        } else {
            cmdMillis = millis();
            previousAudioCmd = audioCmd;
        }
    }
}             // Test function

void stopAll(){
    lightsOff();                                    // Kill lights
    relay("off");                                   // Turn off power to the amplifier
    resetAudio();                                   // Reset the audio card to kill any playing audio..
    pinFixer("all");                                // Shut down the output pin on ctrlr towrards audiofx..not sure this works
    looper = 0;
}                         // Complete

void button(){                                                  // Button press= LOW, LED lit = HIGH
    unsigned long currentMillis = millis();
    currentButtonState = digitalRead(buttonPress);              // Check if button is pressed at this moment
    if (currentButtonState == LOW) {                            // If the button IS pressed..then
        if (currentButtonState == previousButtonState){         // Check if it was pressed last time we checked..if it was, then
            if (currentMillis - pressedMillis >= interval) {    // Check if it has been pressed for the duration of "interval", if yes, then
                stopAll();                                      // A bunch of things must happen to turn off, so I made a function for that purpose
                exit;                                           // We do not want anything more happening..break out! Apparently "exit;" does nothing.. 
            }
        } else {                                                // If interval hasn't been reached then
            pressedMillis = millis();                           // When did we press the button..
            previousButtonState = currentButtonState;           // Record the button state..
            digitalWrite(buttonLED, HIGH);                      // Light the LED on the button :)
        }
    } else if (currentButtonState == HIGH){                     // The most common state, button NOT pressed..
        if (currentButtonState != previousButtonState) {        // However, what if the button was pressed on the last run through? This checks to see if it has been released
            stopAll();                                          // because if that is the case then we need to stop what we're doing and
            looper = looper + 1;                                // Increase the looper variable to move to the next program in line
            if (looper == 7) looper =0;                         // and make sure we loop looper around when we have nothing more to show off :)
        }
        previousButtonState = currentButtonState;               // Record the button state for the next run around..
        digitalWrite(buttonLED, LOW);                           // Turn of the button LED
    }
}                          // Complete

void staticDisplay(){
    unsigned long currentMillis = millis();
    pixels.setPixelColor(0, 128, 128, 128); // Headlights

    // Cockpit blinking..brute coding
    if (currentMillis - previousCockpitMillis >= cockpitInterval) {
        // save the last time you blinked the LED
        previousCockpitMillis = currentMillis;
    
        // if the LED is off turn it on and vice-versa:
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
    pixels.show();
}                   // Complete

void takeOff (){                            // Very early attempt at a take off sequence..basically just found out how to do the timing
    unsigned long currentMillis = millis();
    if (audio == 0){                        // I just want the audio pin to trigger ONCE..so I have to create some sort of controller. audio was set to 0 during power up
        digitalWrite(audioPin00, LOW);      // Trigger audio
        audio = 1;                          // Set audio to 1 as the sound has been triggered..no need to do it again
        if (currentMillis - sequenceStart >1000){
            digitalWrite(audioPin00, HIGH);
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
}                        // WIP

void soundDisplay(String program){
    staticDisplay();
    if (program == "intro") playSound("intro");
    if (program == "medley") playSound("medley");
}      // WIP

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

void colorEngine(int r, int g, int b) {         // Function to color the engine portion of the strip
    for(int i=0; i<4; i++) {                    // Set aside the first 5 pixels for the engine during testing, think this will be 17 in the end.. 
        pixels.setPixelColor(i, r, g, b);
    }
    pixels.show();                            // Send info to LEDs
} // Probably not in use.. 