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


int brightness0 = 0;                // how bright the LED is
int brightness1 = 255;
int fadeAmount0 = 5;                // how many points to fade the LED by
int fadeAmount1 = -5;

// States and the like
int ledState = 1;
int state = 1;
int currentButtonState;
int previousButtonState;
String cmd = "0";
String previousCmd;
String lowPin;

// Timing variables
unsigned long pressedMillis;
unsigned long cmdMillis;
unsigned long startTime = 0;        // For timing lights and sound..need to be more specific..
unsigned long sequenceStart;        // For timing lights and sound
unsigned long currentMillis;
unsigned long previousMillis = 0;
const long cockpitInterval = 500;
const long engineInterval = 30;
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
}

int falcon(String cmd) {
    // Control Commands...
    if(cmd == "basic"){
        looper = 1;
    }
    if(cmd == "takeOff"){

    }
    if(cmd == "intro"){

    }
    if(cmd == "medley"){

    }

    // Admin's "special" Commands
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
}

void loop(){                    // The loop that runs all the time.. looper = 0 reserved
    connect();
    Spark.publish("Uptime",looper);

    currentMillis = millis();

    button(); // Check the button on the base; short press = switch program, long press = turn off sound/lights

    if (looper == 1){ //static display
        staticDisplay();
    }

    if (looper == 2){ // take off
        takeOff();
    }

    if (looper == 3){ // intro
        soundDisplay("intro");
    }

    if (looper == 4){ // medley
        soundDisplay("medley");
    }

    if (looper == 11){
        playSound("1");
    }


}

void playSound(String cmd){
    if (cmd == "1") {

        relay("on");

        digitalWrite(audioPin00, LOW);

        if (cmd == previousCmd){
            if (currentMillis - cmdMillis >= 25000) { // Let audio play until finished, then shut down the relay..
                digitalWrite(audioPin00, HIGH);
                relay("off");
                previousCmd = "";
                pinFixer(cmd);          // Shut down the output pin on ctrlr towrards audiofx..
            }
        } else {
            volumeD(25);
            cmdMillis = millis();
            previousCmd = cmd;
        }


    }
    if (cmd == "2") {
        relay("on");
        digitalWrite(audioPin01, LOW);
        delay(wait);
        digitalWrite(audioPin01, HIGH);
        relay("off");
    }
    if (cmd == "3") {
        relay("on");
        digitalWrite(audioPin02, LOW);
        delay(wait);
        digitalWrite(audioPin02, HIGH);
        relay("off");
    }
}



void button(){                                                  // Button press= LOW, LED lit = HIGH
    currentButtonState = digitalRead(buttonPress);              // Check if button is pressed at this moment
    if (currentButtonState == LOW) {                            // If the button IS pressed..then
        if (currentButtonState == previousButtonState){         // Check if it was pressed last time we checked..if it was, then
            if (currentMillis - pressedMillis >= interval) {    // Check if it has been pressed for the duration of "interval", if yes, then
                lightsOff();                                    // Kill lights
                relay("off");                                   // Turn off power to the amplifier
                resetAudio();                                   // Reset the audio card to kill any playing audio..
                pinFixer("all");                                // Shut down the output pin on ctrlr towrards audiofx..not sure this works
                exit;                                          // We do not want anything more happening..break out!
            }
        } else {
            pressedMillis = millis();
            previousButtonState = currentButtonState;
            // turn LED on:
            digitalWrite(buttonLED, HIGH);
        }
    } else if (currentButtonState == HIGH){
        if (currentButtonState != previousButtonState) {
            //switch display program..må nok legge inn en sak som sier at dersom du akkurat har holdt den inne for å "drepe alt" så skal ikke dette skje..kanskje break over er nok?
        }
        previousButtonState = currentButtonState;
        digitalWrite(buttonLED, LOW);
    }
}

void staticDisplay(){
    colorEngine(0,0,255);           // The initial blue
    pixels.setPixelColor(0, 255, 255, 255);

    // Cockpit blinking..brute coding
    if (currentMillis - previousMillis >= cockpitInterval) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;

        // if the LED is off turn it on and vice-versa:
        if (state == 1) {
            pixels.setPixelColor(1, 255, 0, 0);
            pixels.setPixelColor(2, 255, 255, 255);
            pixels.setPixelColor(3, 0, 0, 255);
            state = 2;
        } else if(state == 2) {
            pixels.setPixelColor(2, 255, 0, 0);
            pixels.setPixelColor(3, 255, 255, 255);
            pixels.setPixelColor(1, 0, 0, 255);
            state = 3;
        } else if(state == 3) {
            pixels.setPixelColor(3, 255, 0, 0);
            pixels.setPixelColor(1, 255, 255, 255);
            pixels.setPixelColor(2, 0, 0, 255);
            state = 1;
        }
    }

    // Engine pulsating..brute..
    pixels.setPixelColor(4, brightness0, brightness0, brightness0); // Even numbers
    pixels.setPixelColor(6, brightness0, brightness0, brightness0);
    pixels.setPixelColor(8, brightness0, brightness0, brightness0);
    pixels.setPixelColor(10, brightness0, brightness0, brightness0);
    pixels.setPixelColor(5, brightness1, brightness1, brightness1); // Odd numbers
    pixels.setPixelColor(7, brightness1, brightness1, brightness1);
    pixels.setPixelColor(9, brightness1, brightness1, brightness1);

    // change the brightness for next time through the loop:
    brightness0 = brightness0 + fadeAmount0;
    brightness1 = brightness1 + fadeAmount1;

    if (currentMillis - previousMillis >= engineInterval) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;
        // reverse the direction of the fading at the ends of the fade:
        if (brightness0 == 0 || brightness0 == 255) {
            fadeAmount0 = -fadeAmount0 ;
        }
        if (brightness1 == 255 || brightness1 == 0) {
            fadeAmount1 = -fadeAmount1 ;
        }
        pixels.show();
    }
}

void takeOff (){                            // Very early attempt at a take off sequence..basically just found out how to do the timing
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
}

void soundDisplay(String program){
    staticDisplay();

    if (program == "intro"){
        if (audio == 0){                        // I just want the audio pin to trigger ONCE..so I have to create some sort of controller. audio was set to 0 during power up
            digitalWrite(audioPin01, LOW);      // Trigger audio
            audio = 1;                          // Set audio to 1 as the sound has been triggered..no need to do it again

            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                digitalWrite(audioPin01, HIGH);
            }
        }
    } else if (program == "medley") {
        if (audio == 0){                        // I just want the audio pin to trigger ONCE..so I have to create some sort of controller. audio was set to 0 during power up
            digitalWrite(audioPin02, LOW);      // Trigger audio
            audio = 1;                          // Set audio to 1 as the sound has been triggered..no need to do it again

            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                digitalWrite(audioPin02, HIGH);
            }
        }
    }
}

void colorEngine(int r, int g, int b) {         // Function to color the engine portion of the strip
    for(int i=0; i<4; i++) {                    // Set aside the first 5 pixels for the engine during testing, think this will be 17 in the end..
        pixels.setPixelColor(i, r, g, b);
    }
    pixels.show();                            // Send info to LEDs
}

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

void pinFixer(String lowPin){ // Remember to add the pins here that are used for audio..
    if (lowPin == "1") pinSetFast(audioPin00);      // Selectively putting pins LOW
    if (lowPin == "2") pinSetFast(audioPin01);
    if (lowPin == "3") pinSetFast(audioPin02);
    if (lowPin == "all"){                           // nice to have the option..
        pinSetFast(audioPin00);
        pinSetFast(audioPin01);
        pinSetFast(audioPin02);
    }
}

void connect() {
  if (Particle.connected() == false) {  // If not connected, then..
    Particle.connect();                 // connect!
  }
}
