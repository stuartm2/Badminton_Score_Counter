
#include <LowPower.h>
#include <RFM69.h>
#include <SPI.h>

// Addresses for this node. CHANGE THESE FOR EACH NODE!

#define NETWORKID     0   // Must be the same for all nodes
#define MYNODEID      1   // My node ID
#define TONODEID      3   // Destination node ID

// RFM69 frequency, uncomment the frequency of your module:

#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY     RF69_915MHZ

// Create a library object for our RFM69HCW module:

RFM69 radio;


#define BTN_PIN 3
#define LED_PIN 9
#define BTN_DELAY 250  // ms

void wakeUp() {} // A handler for the pin interrupt.

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(LED_PIN, LOW);

  // Initialize the RFM69HCW:
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
}

void loop() {
  // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(1, wakeUp, LOW);
  
  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
  
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(1);

  // Do something here
  sendData();
  digitalWrite(LED_PIN, HIGH);
  delay(BTN_DELAY);
  digitalWrite(LED_PIN, LOW);

  // Sleep for 8 seconds
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); 
}

void sendData() {
  // Send the packet!

  static char sendBuffer[62];
  static int sendLength = 0;

  // There are two ways to send packets. If you want
  // acknowledgements, use sendWithRetry():

  sendBuffer[0] = 84;
  sendBuffer[1] = 101;
  sendBuffer[2] = 115;
  sendBuffer[3] = 116;
  sendLength = 4;

  radio.send(TONODEID, sendBuffer, sendLength);
}

