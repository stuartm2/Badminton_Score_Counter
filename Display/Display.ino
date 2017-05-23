
#include <Wire.h>

#define SEG_A_PIN 16
#define SEG_B_PIN 14
#define SEG_C_PIN 15
#define SEG_D_PIN A0
#define SEG_E_PIN A1
#define SEG_F_PIN A2
#define SEG_G_PIN A3

#define DIG0_EN_PIN 4
#define DIG1_EN_PIN 5
#define DIG2_EN_PIN 6
#define DIG3_EN_PIN 7


static int segmentVals[][7] = {
// A, B, C, D, E, F, G
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}, // 9
  {0, 0, 0, 0, 0, 0, 0}, // Blank
};


int grnScore = 0;
int redScore = 0;
int i = 0;
int lastDisplayChanged = 0;

void setup() {
  pinMode(SEG_A_PIN, OUTPUT);
  pinMode(SEG_B_PIN, OUTPUT);
  pinMode(SEG_C_PIN, OUTPUT);
  pinMode(SEG_D_PIN, OUTPUT);
  pinMode(SEG_E_PIN, OUTPUT);
  pinMode(SEG_F_PIN, OUTPUT);
  pinMode(SEG_G_PIN, OUTPUT);

  pinMode(DIG0_EN_PIN, OUTPUT);
  pinMode(DIG1_EN_PIN, OUTPUT);
  pinMode(DIG2_EN_PIN, OUTPUT);
  pinMode(DIG3_EN_PIN, OUTPUT);

  Wire.begin(8);
  Wire.onReceive(onI2CReceived);
}

void loop() {
  long timeNow = millis();
  int score = 0;
  
  if (i >= 4) {
    i = 0;
  }

  if (timeNow >= lastDisplayChanged + 5) {
    if (i == 0) {
      score = grnScore / 10;
    } else if (i == 1) {
      score = grnScore % 10;
    } else if (i == 2) {
      score = redScore / 10;
    } else {
      score = redScore % 10;
    }
    
    displayNumber(score, i++);
    lastDisplayChanged = timeNow;
  }
}

void displayNumber(int num, int pos) {
  digitalWrite(DIG0_EN_PIN, LOW);
  digitalWrite(DIG1_EN_PIN, LOW);
  digitalWrite(DIG2_EN_PIN, LOW);
  digitalWrite(DIG3_EN_PIN, LOW);

  digitalWrite(SEG_A_PIN, segmentVals[num][0]);
  digitalWrite(SEG_B_PIN, segmentVals[num][1]);
  digitalWrite(SEG_C_PIN, segmentVals[num][2]);
  digitalWrite(SEG_D_PIN, segmentVals[num][3]);
  digitalWrite(SEG_E_PIN, segmentVals[num][4]);
  digitalWrite(SEG_F_PIN, segmentVals[num][5]);
  digitalWrite(SEG_G_PIN, segmentVals[num][6]);

  digitalWrite(DIG0_EN_PIN, pos == 0 && num > 0);
  digitalWrite(DIG1_EN_PIN, pos == 1);
  digitalWrite(DIG2_EN_PIN, pos == 2 && num > 0);
  digitalWrite(DIG3_EN_PIN, pos == 3);
}

void onI2CReceived(int howMany) {
  int score = Wire.read();    // receive byte as an integer

  if (score >= 31) {
    redScore = score - 31;
  } else {
    grnScore = score;
  }
}

