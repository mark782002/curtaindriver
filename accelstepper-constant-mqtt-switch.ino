#include <Ethernet.h>
#include <ArduinoHA.h>
#include <AccelStepper.h>

// This is the current sketch for curtain automation
// extension with switch 20 - 2 -23

// Nema 17  & L298N driver

#define BROKER_ADDR     IPAddress(192,168,1,151)

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};


EthernetClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);

const int buttonPinOpen = 8;
const int buttonPinClose = 2;
const int ledPin = 13;

const int manualSwitchOpen = 9;
const int manualSwitchClose = 3;

int buttonCloseState = 0;  // variable for reading the pushbutton status
int buttonOpenState = 0;
int manualSwitchOpenState = 0;
int manualSwitchCloseState = 0;
bool isClosed = false;
bool isOpen = false;

HASwitch ledA("leda");
HASwitch ledB("ledb");

AccelStepper stepper(AccelStepper::FULL4WIRE, 4, 5, 6, 7);

void onSwitchCommand(bool state, HASwitch* sender)
{

  //digitalWrite(LED_PIN, (state ? HIGH : LOW));
  sender->setState(state); // report state back to the Home Assistant

  /*  if (Serial.available()) {
      input = Serial.read();
      Serial.print("You typed: " );
      Serial.println(input);
    }*/
}


void setup()
{

  Serial.begin(9600);
  Ethernet.begin(mac);

  device.setName("Arduino");
  device.setSoftwareVersion("1.0.0");

  // adding button
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPinOpen, INPUT);
  pinMode(buttonPinClose, INPUT);
  pinMode(manualSwitchOpen, INPUT);
  pinMode(manualSwitchClose, INPUT);


  // set icon (optional)
  ledA.setIcon("mdi:curtains");
  ledA.setName("Gordijnen openen");
  ledB.setIcon("mdi:curtains-closed");
  ledB.setName("Gordijnen sluiten");

  // handle switch state
  ledA.onCommand(onSwitchCommand);
  ledB.onCommand(onSwitchCommand);

  mqtt.begin(BROKER_ADDR, 1883);

  stepper.setMaxSpeed(1000);

}

void loop()
{
  // adding button
  buttonCloseState = digitalRead(buttonPinClose);
  buttonOpenState = digitalRead(buttonPinOpen);
  manualSwitchOpenState = digitalRead(manualSwitchOpen);
  manualSwitchCloseState = digitalRead(manualSwitchClose);

      //Serial.print("Switch state: " );
      //Serial.println(manualSwitchOpenState);
  

  
  //Serial.println(buttonOpenState);
  Ethernet.maintain();
  mqtt.loop();


  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonCloseState == HIGH) {
    
    isClosed = true;
  ledB.setState(false);
  }

  if (buttonOpenState == HIGH) {
    isOpen = true;
      ledA.setState(false);
  }

  if ((ledA.getCurrentState() == 1 || manualSwitchOpenState == HIGH) && isOpen == false)
  {
    // open
    isClosed = false;
    stepper.setSpeed(200);
    stepper.run();  

  } else if ((ledB.getCurrentState() == 1 || manualSwitchCloseState == HIGH) && isClosed == false) {
    
    // close
    isOpen = false;
    stepper.setSpeed(-200);
    stepper.run();
  } else if (ledA.getCurrentState() == 0 && ledB.getCurrentState() == 0) {
    stepper.disableOutputs();
  }



}
