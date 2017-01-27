SYSTEM_MODE(SEMI_AUTOMATIC);    // Ok, so semi_automatic allows things to start up, THEN connect to the internet. Automatic did it the other way around. 

// Audio Triggers
#define audioPin00  D6   // Left Right Left Right // Will become empty wave file for reset purposes..
#define audioPin01  D5   // Some random Falcon sound // Will become a "ready" sound, i.e. "Chewey, we're home!"
#define audioPin02  D4   // Short takeoff
#define audioPin03  D3   //
#define audioPin04  D2   //
#define audioPin05  D1   //
#define audioPin06  D0   //
#define audioPin07  D3   //
#define audioPin08  A2   //
#define audioPin09  A1   //
#define audioPin10  A0   //

// Relay
#define relPin WKP

// Audio Reset
#define audioRst    D7  // To give me the possibilty to reset the board as it tends to get "weird"

int wait = 1000;

void setup() {
    Particle.function("falcon", falcon);   // The cloud exposed Function
    
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
    
    pinMode(audioRst, OUTPUT);
    digitalWrite(audioRst, HIGH);
    
    pinMode(relPin, OUTPUT);
    digitalWrite(relPin, LOW);
}

int falcon(String cmd) {
    if (cmd == "on"){
        relay("on");
    }
    if (cmd == "off"){
        relay("off");
    }
    if (cmd == "00") { 
        digitalWrite(audioPin00, LOW);
        delay(wait);
        digitalWrite(audioPin00, HIGH);
    } 
    if (cmd == "01") {       
        digitalWrite(audioPin01, LOW);
        delay(wait);
        digitalWrite(audioPin01, HIGH);
    }    
    if (cmd == "02") {        
        digitalWrite(audioPin02, LOW);
        delay(wait);
        digitalWrite(audioPin02, HIGH);
    }    
    if (cmd == "03") {      
        digitalWrite(audioPin03, LOW);
        delay(wait);
        digitalWrite(audioPin03, HIGH);
    }    
    if (cmd == "04") {      
        digitalWrite(audioPin04, LOW);
        delay(wait);
        digitalWrite(audioPin04, HIGH);
    }    
    if (cmd == "05") {     
        digitalWrite(audioPin05, LOW);
        delay(wait);
        digitalWrite(audioPin05, HIGH);
    }    
    if (cmd == "06") {       
        digitalWrite(audioPin06, LOW);
        delay(wait);
        digitalWrite(audioPin06, HIGH);
    }    
    if (cmd == "07") {        
        digitalWrite(audioPin07, LOW);
        delay(wait);
        digitalWrite(audioPin07, HIGH);
    }    
    if (cmd == "08") {         
        digitalWrite(audioPin08, LOW);
        delay(wait);
        digitalWrite(audioPin08, HIGH);
    }    
    if (cmd == "09") {         
        digitalWrite(audioPin09, LOW);
        delay(wait);
        digitalWrite(audioPin09, HIGH);
    }    
    if (cmd == "10") {         
        digitalWrite(audioPin10, LOW);
        delay(wait);
        digitalWrite(audioPin10, HIGH);
    }
    if (cmd == "reset"){
        rstAudio();
    }
   
return 1;   // Returns 1 so we know all went well.. 
}

void loop() {
   connect();   // If Photon was just powered up, then connect() to the internet. 
}

void relay(String cmd){                 // The relay that controls the power to the amp takes a string command, on or off
    if (cmd == "off"){                   // Reason for this is to remove static hum when displaying only lights
        digitalWrite(relPin, LOW);
    }else if(cmd == "on"){
        digitalWrite(relPin, HIGH);
    }
}

void rstAudio(){                    // Reset function for the audio fx board
    digitalWrite(audioRst, LOW);    // Trip the rst pin on the audio fx boardd..
    delay(wait);                    // wait a bit..
    digitalWrite(audioRst, HIGH);   // Back in business
}

void connect() {
  if (Particle.connected() == false) {  // If not connected, then.. 
    Particle.connect();                 // connect!
  }
}
