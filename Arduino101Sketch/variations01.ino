// Arduino 101 Bluetooth MIDI Controller
// Copyright 2017 Don Coleman
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// Uses Arduino 101 with Adafruit Capactive Touch Shield https://www.adafruit.com/products/2024
// Tested with Garage Band on an iPad.
//    Click wrench on top right. Settings -> Advanced -> Bluetooth MIDI Devices -> MIDI_101

// based on code from Limor Fried and Oren Levy
// MIDIBLE.ino written by Oren Levy (MIT License)
// https://github.com/01org/corelibs-arduino101/tree/4ae5af7f8a63dfaa9d44b35c0c820d93a4a6980e/libraries/CurieBLE/examples/peripheral/MIDIBLE/MIDIBLE.ino
// MRP121test.ino Written by Limor Fried/Ladyada for Adafruit Industries. (BSD License)
// https://github.com/adafruit/Adafruit_MPR121/blob/980511c0bb52f78a136e679357f6e5f0d7b65518/examples/MPR121test/MPR121test.ino 

#include <CurieBLE.h>

class MidiButtons{
  
  #define button1 7
  #define button2 8
  #define button3 9
  #define button4 10
  #define button5 11
  #define button6 12

  boolean wasButtonPressed;
  boolean wasButtonReleased;
  
  // Keeps track of the last pins touched
  // so we know when buttons are 'released'
  uint8_t previousPressed = 255;
  uint8_t currentPressed = 255;

 // button debouncing
 long pressDelay = 50;
 long previousTime, currentTime;
 boolean b1, b2, b3, b4, b5, b6;
 
  
  public:

  MidiButtons(int dummy){}
  void init()
  {

    pinMode(button1, INPUT_PULLUP);
    pinMode(button2, INPUT_PULLUP);
    pinMode(button3, INPUT_PULLUP);
    pinMode(button4, INPUT_PULLUP);
    pinMode(button5, INPUT_PULLUP);
    pinMode(button6, INPUT_PULLUP);
    b1 = TRUE;
    b2 = TRUE;
    b3 = TRUE;
    b4 = TRUE;
    b5 = TRUE;
    b6 = TRUE;
    wasButtonPressed = FALSE;
    wasButtonReleased = FALSE;
    previousPressed = 255; 
    previousTime = 0;
    
  }
  
  void debounce()
  {
   b1 = digitalRead(button1);
   b2 = digitalRead(button2);
   b3 = digitalRead(button3);
   b4 = digitalRead(button4);
   b5 = digitalRead(button5);
   b6 = digitalRead(button6);

   if (!b1 || !b2 || !b3 || !b4 || !b5 || !b6) 
   { 
    currentTime = millis(); 
    if ((currentTime - previousTime) > pressDelay) 
    {
      previousTime = currentTime;
      if ( !b1 && !digitalRead(button1)) { currentPressed = 0;  }
      else if (!b2 && !digitalRead(button2)) { currentPressed = 1; }
      else if (!b3 && !digitalRead(button3)) { currentPressed = 2; }
      else if (!b4 && !digitalRead(button4)) { currentPressed = 3; }
      else if (!b5 && !digitalRead(button5)) { currentPressed = 4; }
      else if (!b6 && !digitalRead(button6)) { currentPressed = 5; }
      wasButtonPressed = TRUE; 
      wasButtonReleased = FALSE;
      previousPressed = currentPressed;
    }
   } 
   else if( b1 && b2 && b3 && b4 && b5 && b6 )
   { 
      currentTime = millis();
      if ((currentTime-previousTime) > pressDelay)
      {
        previousTime = currentTime;
        if ( wasButtonPressed ) 
        {
          wasButtonPressed = FALSE;
          wasButtonReleased = TRUE;
          // Serial.println("button Released");
        }
      }
   }

  
 }
  void Update() 
  {
    debounce(); 
  }
  uint8_t getCurrentPressed() { return currentPressed; }
  boolean getWasButtonPressed() { return wasButtonPressed; }
  boolean getWasButtonReleased() { return wasButtonReleased; }
};


//Buffer to hold 5 bytes of MIDI data. Note the timestamp is forced
uint8_t midiData[] = {0x80, 0x80, 0x00, 0x00, 0x00};

int midiChannel = 0;

// https://www.midi.org/specifications/item/gm-level-1-sound-set
int instruments[] = {
  36, // C3 Kick
  38, // D3 Snare
  42, // F#3 Closed hi-hat
  46, // A#3 Open hi-hat
  49  // C#4 crash symbol
};

// https://www.midi.org/specifications/item/bluetooth-le-midi
BLEPeripheral blePeripheral;
BLEService midiService("03B80E5A-EDE8-4B33-A751-6CE34EC4C700"); 
BLECharacteristic midiCharacteristic("7772E5DB-3868-4112-A1A9-F2669D106BF3", BLEWrite | BLEWriteWithoutResponse | BLENotify | BLERead, sizeof(midiData));

void setupBluetooth()
{
  blePeripheral.setLocalName("MIDI_101");
  blePeripheral.setDeviceName("MIDI_101");

  blePeripheral.setAdvertisedServiceUuid(midiService.uuid());

  blePeripheral.addAttribute(midiService);
  blePeripheral.addAttribute(midiCharacteristic);

  blePeripheral.setEventHandler(BLEConnected, connectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, disconnectHandler);

  blePeripheral.begin();
  Serial.println(("Waiting for Bluetooth connections."));
}

void playNote(uint8_t index) {
  int note = instruments[index];
  
  midiData[2] = 0x90 + midiChannel;
  midiData[3] = note;
  midiData[4] = 127; // velocity
  
  midiCharacteristic.setValue(midiData, sizeof(midiData));
}

void releaseNote(uint8_t index) {
  int note = instruments[index];
  
  midiData[2] = 0x80 + midiChannel;
  midiData[3] = note;
  midiData[4] = 0; // velocity

  midiCharacteristic.setValue(midiData, sizeof(midiData));
}

void connectHandler(BLECentral& central) {
  Serial.print("Central device: ");
  Serial.print(central.address());
  Serial.println(" connected.");
}

void disconnectHandler(BLECentral& central) {
  Serial.print("Central device: ");
  Serial.print(central.address());
  Serial.println(" disconnected.");
}

MidiButtons midiButtons(0);

void setup() {  
  Serial.begin(9600);
  Serial.println("Arduino 101 Bluetooth MIDI Controller\n"); 
  midiButtons.init();
  setupBluetooth();
}

// maintaining state in the loop
uint8_t currentNote, lastNote;
boolean wasPressed, wasReleased;

void loop(){
  
  midiButtons.Update();
  currentNote = midiButtons.getCurrentPressed();
  wasPressed = midiButtons.getWasButtonPressed();
  wasReleased = midiButtons.getWasButtonReleased();

  if ( wasPressed && (currentNote != lastNote)) 
  {
    playNote(currentNote);
    lastNote = currentNote;
  } 
  if (wasReleased && (lastNote == currentNote))
  {
    releaseNote(currentNote);
    lastNote = 255;  
  }
}
