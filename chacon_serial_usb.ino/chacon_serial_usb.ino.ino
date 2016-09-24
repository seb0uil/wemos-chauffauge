


#define VOLET_ENTREE V0
#define LUMIERE_TOILETTE V1

#define RC_TRANSMIT 7
#define RC_RECEIVE 0 // Interrupt 0 -> port #2

#include <NewRemoteTransmitter.h>
#include <NewRemoteReceiver.h>
#include <SoftwareSerial.h>
#include <BlynkSimpleSerial.h>

SoftwareSerial SwSerial(2, 3); // RX, TX
#define BLYNK_PRINT SwSerial


// Create a transmitter on address 123, using digital pin 11 to transmit, 
// with a period duration of 260ms (default), repeating the transmitted
// code 2^3=8 times.
NewRemoteTransmitter transmitter(20400274, RC_TRANSMIT, 260, 3);
NewRemoteTransmitter lumtoilette(15273926, RC_TRANSMIT, 260, 3);

char auth[] = "xxx";

boolean volet_sens = true;  //sens de fonctionnement du volet
boolean toilette = false;   //etat de la lumiere

int ledPin = 13;                 // LED connected to digital pin 13

void setup() {
  SwSerial.begin(57600);
  Blynk.begin(auth, 57600);

  NewRemoteReceiver::init(RC_RECEIVE, 2, showCode);

  pinMode(ledPin, OUTPUT);      // sets the digital pin as output
}

void loop() {  
    Blynk.run();
}

// Fonctionnement du volet
BLYNK_WRITE(V2)
{
  long pinValue = param.asLong(); 
  NewRemoteTransmitter mytransmitter(pinValue, RC_TRANSMIT, 260, 3);
  mytransmitter.sendUnit(0, volet_sens);
}

BLYNK_WRITE(V3)
{
  int pinValue = param.asInt();
  volet_sens = (pinValue != 0);
}

// Fonctionnement du volet
BLYNK_WRITE(VOLET_ENTREE)
{
  transmitter.sendUnit(0, volet_sens);
  volet_sens = !volet_sens;
}


// Lumiere toilette
BLYNK_WRITE(LUMIERE_TOILETTE)
{
  lumtoilette.sendUnit(9, toilette);
  toilette = !toilette;
}

// Callback function is called only when a valid code is received.
void showCode(NewRemoteCode receivedCode) {
//        Serial.print(receivedCode.address);
//        Serial.println(receivedCode.switchType);
/*
  switch (receivedCode.switchType) {
    case NewRemoteCode::off:
      break;
    case NewRemoteCode::on:
      break;
    case NewRemoteCode::dim:
      break;
  }
  */
}
