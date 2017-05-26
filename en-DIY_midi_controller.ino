/* 
Made by Gustavo Silveira, 2016.
This Sketch reads the Arduino's digital and analog ports and send midi notes and midi control change

http://www.musiconerd.com
http://www.bitcontrollers.com

If you are using for anything that's not personal use don't forget to give credit.

gustavosilveira@musiconerd.com
*/

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

/////////////////////////////////////////////

const int NButtons = 4; // *Enter here the number of digital inputs used 
const int button[NButtons] = {2, 3, 4, 5}; // *In this array enter the pins of the digital ports used
int buttonCState[NButtons] = {0}; // Current state of the digital port
int buttonPState[NButtons] = {0}; // Previous state of the digital port

/////////////////////////////////////////////

const int NPots = 1; // *Enter here the number of analog inputs used
const int pot[NPots] = {A0}; // *In this array enter the pins of the analog ports used in the desired order
int potCState[NPots] = {0}; // Current state of the analog port
int potPState[NPots] = {0}; // Previous state of the analogue port
int potVar = 0; // Variation between the value of the previous state and the current value of the analogue port

/////////////////////////////////////////////

byte midiCh = 1; // *Midi channel to be used 1-16
byte note = 36; // *Lowest note that will be used
byte cc = 1; // *Lowest CC that will be used

/////////////////////////////////////////////

int TIMEOUT = 300; //Amount of time that the potentiometer will be read after it exceeds the varThreshold
int varThreshold = 7; // *Threshold for the potentiometer signal variation
boolean potMoving = true; // If the potentiometer is moving
unsigned long pTime[NPots] = {0}; // Previously stored time
unsigned long timer[NPots] = {0}; // Stores the time that has elapsed since the timer was reseted

/////////////////////////////////////////////

void setup () {

  //Start midi connection
  MIDI.begin();
  //Serial Connection
  Serial.begin(115200); // Use 115200 for Hairless midi or 31250 for a physical midi port
  //Initialize Digital Pins as inputs
  for (int i=0; i<NButtons; i++){
    pinMode(button[i], INPUT_PULLUP);
  }
  
  
  //Initialize Analog Pins as inputs
  for (int i=0; i<NPots; i++){
    pinMode(pot[i], INPUT);
  }
  

}

void loop () {

  //Reads all digital inputs
  for (int i=0; i<NButtons; i++) {
    buttonCState[i] = digitalRead(button[i]);
  }
    
  //Reads the state of the buttons and send NoteOns to each one that is active
  for (int i=0; i<NButtons; i++) {

    if (buttonCState[i] != buttonPState[i]) {

      if(buttonCState[i] == LOW) {     
        MIDI.sendNoteOn(note+i, 127, midiCh); // sends NoteOn(note, velocity, midi channel)
        buttonPState[i] = buttonCState[i];
      }
      else {
        MIDI.sendNoteOn(note+i, 0, midiCh); 
        buttonPState[i] = buttonCState[i];
      }
    }
    
  }

  ////////////////////////////////////////////////////////////////////////////////////////

  for (int i=0; i<NPots; i++) { // Reads all analog inputs used
    potCState[i] = analogRead(pot[i]);
  }

    /* So that only the analog ports are read when they are used, without losing resolution,
     You must set a threshold (varThreshold), a minimum value that the pots have to have to be moved
     To start reading. After this, a sort of "door" is created, a door that opens and allows
    Analogue ports to be read without interruption for a certain time (TIMEOUT). When the timer is less than TIMEOUT
    Means that the potentiometer has been shaken for a very short time, meaning that it is probably still moving,
    Then, the door should be kept open; If the timer is longer than TIMEOUT means that it has not been moved for a while,
    Then, the door should be closed. For this logic to happen, the timer must be reset (lines 99 and 100) each time the analogue port
    is bigger than the established varThreshold.
    */
  for (int i=0; i<NPots; i++) {

    potVar = abs(potCState[i] - potPState[i]); // Computes the analogue port variation

    if (potVar >= varThreshold) {  //Sets the threshold for the variance in the pot state, if it varies more than x it sends the cc message
      pTime[i] = millis(); // Stores the previous time
    }
    timer[i] = millis() - pTime[i]; // Resets the timer
    if (timer[i] < TIMEOUT) { // If the timer is less than the maximum allowed time it means that the potentiometer is still moving
      potMoving = true;
    }
    else {
      potMoving = false;
    }

    if (potMoving == true) { // If the potentiometer is still moving, it sends the control change
      MIDI.sendControlChange(cc+i, map(potCState[i], 0, 1023, 0, 127), midiCh); // Sends Control Change (CC number, CC value, midi channel)
      potPState[i] = potCState[i]; // Stores the current reading of the potentiometer to compare with the next
    }
  }
  
}


