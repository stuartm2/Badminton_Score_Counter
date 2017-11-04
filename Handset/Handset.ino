
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
#define BUTTON_DEBOUNCE_DELAY 100  // ms

long lastPressed = 0;
bool btnState = false;

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  Serial.println("@");
  
  // Initialize the RFM69HCW:
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  //radio.setHighPower(true); // Always use this for RFM69HCW
  
  Serial.println("*");
}

void loop() {
  long timeNow = millis();
  
  if (timeNow > lastPressed + BUTTON_DEBOUNCE_DELAY) {
    if (digitalRead(BTN_PIN) == LOW && !btnState) {
      digitalWrite(4, HIGH);
      btnState = true;
      sendData();
    } else if (digitalRead(BTN_PIN) == HIGH) {
      digitalWrite(4, LOW);
      btnState = false;
    }

    lastPressed = timeNow;
  }
}

void sendData() {
  // Send the packet!

  static char sendBuffer[62];
  static int sendLength = 0;

  Serial.print("sending to node ");
  Serial.print(TONODEID, DEC);
  Serial.println(" >> Test");

  // There are two ways to send packets. If you want
  // acknowledgements, use sendWithRetry():

  sendBuffer[0] = 84;
  sendBuffer[1] = 101;
  sendBuffer[2] = 115;
  sendBuffer[3] = 116;
  sendLength = 4;

  radio.send(TONODEID, sendBuffer, sendLength);
}

