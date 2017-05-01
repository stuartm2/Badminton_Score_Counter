
#include "InputDebounce.h"

#define BUTTON_DEBOUNCE_DELAY   50   // [ms]

#define GRN_1S_1_PIN 22
#define GRN_1S_2_PIN 24
#define GRN_1S_4_PIN 26
#define GRN_1S_8_PIN 28
#define GRN_10S_1_PIN 30
#define GRN_10S_2_PIN 32
#define GRN_INCR_PIN 34
#define GRN_DECR_PIN 36
#define GRN_SET1_PIN 38
#define GRN_SET2_PIN 40
#define GRN_SET3_PIN 42

#define RED_1S_1_PIN 23
#define RED_1S_2_PIN 25
#define RED_1S_4_PIN 27
#define RED_1S_8_PIN 29
#define RED_10S_1_PIN 31
#define RED_10S_2_PIN 33
#define RED_INCR_PIN 35
#define RED_DECR_PIN 37
#define RED_SET1_PIN 39
#define RED_SET2_PIN 41
#define RED_SET3_PIN 43

#define RESET_PIN 52
#define BUZZER_PIN 11

#define NO_TEAM 0
#define GRN_TEAM 1
#define RED_TEAM 2


int grnScore = 0;
int redScore = 0;
int curSet = 0;
int setWins[] = {0, 0, 0};
bool teamWonSet = false;
bool teamWonGame = false;

long lastPressed = 0;
bool grnIncrBtnState = false;
bool grnDecrBtnState = false;
bool redIncrBtnState = false;
bool redDecrBtnState = false;
bool resetBtnState = false;

int flashDuration = 250;
long lastFlashed = 0;
bool flashState = false;


void setup() {
  Serial.begin(115200);
  
  pinMode(GRN_1S_1_PIN, OUTPUT);
  pinMode(GRN_1S_2_PIN, OUTPUT);
  pinMode(GRN_1S_4_PIN, OUTPUT);
  pinMode(GRN_1S_8_PIN, OUTPUT);
  pinMode(GRN_10S_1_PIN, OUTPUT);
  pinMode(GRN_10S_2_PIN, OUTPUT);
  pinMode(GRN_INCR_PIN, INPUT_PULLUP);
  pinMode(GRN_DECR_PIN, INPUT_PULLUP);
  pinMode(RED_1S_1_PIN, OUTPUT);
  pinMode(RED_1S_2_PIN, OUTPUT);
  pinMode(RED_1S_4_PIN, OUTPUT);
  pinMode(RED_1S_8_PIN, OUTPUT);
  pinMode(RED_10S_1_PIN, OUTPUT);
  pinMode(RED_10S_2_PIN, OUTPUT);
  pinMode(RED_INCR_PIN, INPUT_PULLUP);
  pinMode(RED_DECR_PIN, INPUT_PULLUP);
  pinMode(GRN_SET1_PIN, OUTPUT);
  pinMode(GRN_SET2_PIN, OUTPUT);
  pinMode(GRN_SET3_PIN, OUTPUT);
  pinMode(RED_SET1_PIN, OUTPUT);
  pinMode(RED_SET2_PIN, OUTPUT);
  pinMode(RED_SET3_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  Serial.println("Starting");
}

void loop() {
  detectIncrDecrBtnPresses();
  detectResetBtnPresses();
  updateScores();
  showScores();
  showSets();
}

void detectIncrDecrBtnPresses() {
  if (teamWonSet) {
    return;
  }
  
  long timeNow = millis();
  
  if (timeNow > lastPressed + BUTTON_DEBOUNCE_DELAY) {
    // Handle green increment press
    if (digitalRead(GRN_INCR_PIN) == LOW && !grnIncrBtnState) {
      grnIncrBtnState = true;
      grnScore += 1;
    } else if (digitalRead(GRN_INCR_PIN) == HIGH) {
      grnIncrBtnState = false;
    }

    // Handle green decrement press
    if (digitalRead(GRN_DECR_PIN) == LOW && !grnDecrBtnState) {
      grnDecrBtnState = true;
      grnScore -= 1;
      grnScore = max(grnScore, 0);
    } else if (digitalRead(GRN_DECR_PIN) == HIGH) {
      grnDecrBtnState = false;
    }
    
    // Handle red increment press
    if (digitalRead(RED_INCR_PIN) == LOW && !redIncrBtnState) {
      redIncrBtnState = true;
      redScore += 1;
    } else if (digitalRead(RED_INCR_PIN) == HIGH) {
      redIncrBtnState = false;
    }

    // Handle red decrement press
    if (digitalRead(RED_DECR_PIN) == LOW && !redDecrBtnState) {
      redDecrBtnState = true;
      redScore -= 1;
      redScore = max(redScore, 0);
    } else if (digitalRead(RED_DECR_PIN) == HIGH) {
      redDecrBtnState = false;
    }
    
    lastPressed = timeNow;
  }
}

void detectResetBtnPresses() {
  long timeNow = millis();
  
  if (timeNow > lastPressed + BUTTON_DEBOUNCE_DELAY) {
    if (digitalRead(RESET_PIN) == LOW && !resetBtnState) {
      resetBtnState = true;

      grnScore = 0;
      redScore = 0;

      if (teamWonSet) {
        curSet += 1;
        teamWonSet = false;
      }

      if (teamWonGame) {
        curSet = 0;
        setWins[0] = 0;
        setWins[1] = 0;
        setWins[2] = 0;
        teamWonGame = false;
      }
    } else if (digitalRead(RESET_PIN) == HIGH) {
      resetBtnState = false;
    }
  }
}

void updateScores() {
  if (!teamWonSet && greensWin()) {
    // Green wins
    setWins[curSet] = GRN_TEAM;
    teamWonSet = true;
  } else if (!teamWonSet && redsWin()) {
    // Red wins
    setWins[curSet] = RED_TEAM;
    teamWonSet = true;
  }
  
  if (curSet > 2 || (setWins[0] > 0 && setWins[0] == setWins[1])) {
    teamWonGame = true;
  }
}

bool greensWin() {
  if (grnScore == 30 ||
      (grnScore >= 21 && (redScore + 1) < grnScore)) {
    return true;
  } else {
    return false;
  }
}

bool redsWin() {
  if (redScore == 30 ||
      (redScore >= 21 && (grnScore + 1) < redScore)) {
    return true;
  } else {
    return false;
  }
}

void showScores() {
  int grn1s = grnScore % 10;
  int grn10s = grnScore / 10;

  digitalWrite(GRN_1S_1_PIN,  HIGH && (grn1s  & B00000001));
  digitalWrite(GRN_1S_2_PIN,  HIGH && (grn1s  & B00000010));
  digitalWrite(GRN_1S_4_PIN,  HIGH && (grn1s  & B00000100));
  digitalWrite(GRN_1S_8_PIN,  HIGH && (grn1s  & B00001000));
  digitalWrite(GRN_10S_1_PIN, HIGH && (grn10s & B00000001));
  digitalWrite(GRN_10S_2_PIN, HIGH && (grn10s & B00000010));

  int red1s = redScore % 10;
  int red10s = redScore / 10;
  
  digitalWrite(RED_1S_1_PIN,  HIGH && (red1s  & B00000001));
  digitalWrite(RED_1S_2_PIN,  HIGH && (red1s  & B00000010));
  digitalWrite(RED_1S_4_PIN,  HIGH && (red1s  & B00000100));
  digitalWrite(RED_1S_8_PIN,  HIGH && (red1s  & B00001000));
  digitalWrite(RED_10S_1_PIN, HIGH && (red10s & B00000001));
  digitalWrite(RED_10S_2_PIN, HIGH && (red10s & B00000010));
}

void showSets() {
  long timeNow = millis();
  bool grn1State = (setWins[0] == GRN_TEAM);
  bool grn2State = (setWins[1] == GRN_TEAM);
  bool grn3State = (setWins[2] == GRN_TEAM);
  bool red1State = (setWins[0] == RED_TEAM);
  bool red2State = (setWins[1] == RED_TEAM);
  bool red3State = (setWins[2] == RED_TEAM);

  if (timeNow > lastFlashed + flashDuration) {
    flashState = !flashState;
    lastFlashed = timeNow;
  }
  
  if (teamWonGame) {
    // Flash all
    grn1State = grn1State && flashState;
    grn2State = grn2State && flashState;
    grn3State = grn3State && flashState;
    red1State = red1State && flashState;
    red2State = red2State && flashState;
    red3State = red3State && flashState;
  } else if (teamWonSet) {
    // Flash current set
    if (setWins[curSet] == GRN_TEAM) {
      
    }
  }
  
  digitalWrite(GRN_SET1_PIN, grn1State);
  digitalWrite(GRN_SET2_PIN, grn2State);
  digitalWrite(GRN_SET3_PIN, grn3State);
  digitalWrite(RED_SET1_PIN, red1State);
  digitalWrite(RED_SET2_PIN, red2State);
  digitalWrite(RED_SET3_PIN, red3State);
}

