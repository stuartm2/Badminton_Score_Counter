
#include <ChainableLED.h>
#include <TM1637Display.h>
#include "pitches.h"

#define NUM_LEDS 2

#define LED_CLK_PIN 8
#define LED_DATA_PIN 9
ChainableLED leds(LED_CLK_PIN, LED_DATA_PIN, NUM_LEDS);

#define CLK 3//pins definitions for TM1637 and can be changed to other ports    
#define DIO 4
TM1637Display display(CLK, DIO);


#define BUTTON_DEBOUNCE_DELAY 50  // ms

#define RESET_PIN 7
#define BUZZER_PIN 6
#define GRN_INC_PIN A1
#define GRN_DEC_PIN A0
#define RED_INC_PIN A2
#define RED_DEC_PIN A3

#define NO_TEAM 0
#define GRN_TEAM 1
#define RED_TEAM 2


int grnScore = 0;
int redScore = 0;
int curSet = 0;
int setWins[] = {0, 0, 0};
bool teamWonSet = false;
bool teamWonGame = false;

long idLastPressed = 0;
long rstLastPressed = 0;
long scoresLastSent = 0;
bool grnIncBtnState = false;
bool grnDecBtnState = false;
bool redIncBtnState = false;
bool redDecBtnState = false;
bool resetBtnState = false;

int flashDuration = 250;
long lastFlashed = 0;
bool flashState = false;
bool sendGrnScore = false;

bool playingTune = false;
int tuneAt = 0;
int tuneLen = 0;
long lastNoteStarted = 0;

int tuneSteps[][2] = {
  {0, 12},
  {12, 72 + 12}
};

int winTunes[] = {
  // Game win
  NOTE_A4, 200,
  NOTE_B4, 200,
  NOTE_C4, 200,
  NOTE_D4, 200,
  NOTE_E4, 200,
  NOTE_F5, 600,

  // Match win
  NOTE_G4, 400,
  0, 150,
  NOTE_G4, 400,
  0, 150,
  NOTE_G4, 400,
  0, 150,
  NOTE_DS4, 300,
  0, 50,
  NOTE_AS4, 100,
  0, 50,
  NOTE_G4, 400,
  0, 100,
  NOTE_DS4, 300,
  0, 50,
  NOTE_AS4, 100,
  0, 50,
  NOTE_G4, 600,
  0, 300,
  
  NOTE_D5, 400,
  0, 150,
  NOTE_D5, 400,
  0, 150,
  NOTE_D5, 400,
  0, 150,
  NOTE_DS5, 300,
  0, 50,
  NOTE_AS4, 100,
  0, 50,
  NOTE_FS4, 400,
  0, 100,
  NOTE_DS4, 300,
  0, 50,
  NOTE_AS4, 100,
  0, 50,
  NOTE_G4, 600,
  0, 50
};

void startTune(int tune) {
  playingTune = false;
  
  if (tune == 1) {
    tone(BUZZER_PIN, NOTE_F6, 300);
  } else if (tune == 2) {
    tone(BUZZER_PIN, NOTE_A4, 300);
  } else if (tune == 3) {
    playingTune = true;
    tuneAt = tuneSteps[0][0];
    tuneLen = tuneSteps[0][1];
    lastNoteStarted = millis();
    tone(BUZZER_PIN, winTunes[tuneAt]);
  } else if (tune == 4) {
    playingTune = true;
    tuneAt = tuneSteps[1][0];
    tuneLen = tuneSteps[1][1];
    lastNoteStarted = millis();
    tone(BUZZER_PIN, winTunes[tuneAt]);
  } else {
    stopTune();
  }
}

void stopTune() {
  playingTune = false;
  tuneAt = 0;
  tuneLen = 0;
  noTone(BUZZER_PIN);
}

void doTune() {
  if (!playingTune) {
    return;
  }
  
  long timeNow = millis();

  if (timeNow > lastNoteStarted + winTunes[tuneAt + 1]) {
    tuneAt += 2;

    if (tuneAt >= tuneLen) {
      stopTune();
      return;
    }

    if (winTunes[tuneAt] == 0) {
      noTone(BUZZER_PIN);
    } else {
      tone(BUZZER_PIN, winTunes[tuneAt]);
    }
    
    lastNoteStarted = timeNow;
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(GRN_INC_PIN, INPUT);
  pinMode(GRN_DEC_PIN, INPUT);
  pinMode(RED_INC_PIN, INPUT);
  pinMode(RED_DEC_PIN, INPUT);

  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  leds.init();
  
  display.setBrightness(0x0f);
}

void loop() {
  //radioReceive();
  detectIncrDecrBtnPresses();
  detectResetBtnPresses();
  updateScores();
  showSets();
  doTune();
  sendScores();
}

/*void radioReceive() {
  // RECEIVING

  // In this section, we'll check with the RFM69HCW to see
  // if it has received any packets:

  if (radio.receiveDone()) {
    // Print out the information:

    Serial.print("received from node ");
    Serial.print(radio.SENDERID, DEC);
    Serial.print(", message [");

    // The actual message is contained in the DATA array,
    // and is DATALEN bytes in size:

    for (byte i = 0; i < radio.DATALEN; i++) {
      Serial.print((char)radio.DATA[i]);
    }

    // RSSI is the "Receive Signal Strength Indicator",
    // smaller numbers mean higher power.

    Serial.print("], RSSI ");
    Serial.println(radio.RSSI);

    if (teamWonSet) {
      return;
    } else if (radio.SENDERID == 1) {
      startTune(1);
      grnScore += 1;
    } else if (radio.SENDERID == 2) {
      startTune(1);
      redScore += 1;
    }
  }
}*/

void sendScores() {
  long timeNow = millis();

  if (timeNow > scoresLastSent + 50) {
    uint8_t data[] = {
      display.encodeDigit(grnScore / 10),
      display.encodeDigit(grnScore % 10),
      display.encodeDigit(redScore / 10),
      display.encodeDigit(redScore % 10)
    };
  
    if (data[0] == display.encodeDigit(0)) {
      data[0] = 0x00;
    }
    
    if (data[2] == display.encodeDigit(0)) {
      data[2] = 0x00;
    }
    
    data[1] = 0b10000000 | data[1];
    display.setSegments(data);
    
    sendGrnScore = !sendGrnScore;
    scoresLastSent = timeNow;
  }
}

void detectIncrDecrBtnPresses() {
  if (teamWonSet) {
    return;
  }
  
  long timeNow = millis();
  
  if (timeNow > idLastPressed + BUTTON_DEBOUNCE_DELAY) {
    // Handle green button presses
    bool grnIncBtnPressed = digitalRead(GRN_INC_PIN) == LOW;
    bool grnDecBtnPressed = digitalRead(GRN_DEC_PIN) == LOW;
    
    if (grnIncBtnPressed && !grnIncBtnState) {
      grnIncBtnState = true;
      grnScore += 1;
      startTune(1);
    } else if (grnDecBtnPressed && !grnDecBtnState && grnScore > 0) {
      grnDecBtnState = true;
      grnScore -= 1;
      startTune(2);
    }
    
    if (!grnIncBtnPressed) {
      grnIncBtnState = false;
    }

    if (!grnDecBtnPressed) {
      grnDecBtnState = false;
    }

    // Handle red button presses
    bool redIncBtnPressed = digitalRead(RED_INC_PIN) == LOW;
    bool redDecBtnPressed = digitalRead(RED_DEC_PIN) == LOW;
    
    if (redIncBtnPressed && !redIncBtnState) {
      redIncBtnState = true;
      redScore += 1;
      startTune(1);
    } else if (redDecBtnPressed && !redDecBtnState && redScore > 0) {
      redDecBtnState = true;
      redScore -= 1;
      startTune(2);
    }
    
    if (!redIncBtnPressed) {
      redIncBtnState = false;
    }

    if (!redDecBtnPressed) {
      redDecBtnState = false;
    }

    idLastPressed = timeNow;
  }
}

void detectResetBtnPresses() {
  long timeNow = millis();
  
  if (timeNow > rstLastPressed + BUTTON_DEBOUNCE_DELAY) {
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
      
      stopTune();
    } else if (digitalRead(RESET_PIN) == HIGH) {
      resetBtnState = false;
    }

    rstLastPressed = timeNow;
  }
}

void updateScores() {
  if (!teamWonSet && greensWin()) {
    // Green wins
    setWins[curSet] = GRN_TEAM;
    teamWonSet = true;
    startTune(3);
  } else if (!teamWonSet && redsWin()) {
    // Red wins
    setWins[curSet] = RED_TEAM;
    teamWonSet = true;
    startTune(3);
  }

  if (!teamWonGame && (setWins[2] > 0 || (setWins[0] > 0 && setWins[0] == setWins[1]))) {
    teamWonGame = true;
    startTune(4);
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

bool greensWinGame() {
  int wins = 0;
  
  for (int i = 0; i < 3; i++) {
    if (setWins[i] == GRN_TEAM) {
      wins++;
    }
  }

  return wins > 1;
}

bool redsWin() {
  if (redScore == 30 ||
      (redScore >= 21 && (grnScore + 1) < redScore)) {
    return true;
  } else {
    return false;
  }
}

bool redsWinGame() {
  int wins = 0;
  
  for (int i = 0; i < 3; i++) {
    if (setWins[i] == RED_TEAM) {
      wins++;
    }
  }

  return wins > 1;
}

void showSets() {
  long timeNow = millis();
  bool grn1State = (setWins[0] == GRN_TEAM);
  bool grn2State = (setWins[1] == GRN_TEAM);
  bool red1State = (setWins[0] == RED_TEAM);
  bool red2State = (setWins[1] == RED_TEAM);

  if (timeNow > lastFlashed + flashDuration) {
    flashState = !flashState;
    lastFlashed = timeNow;
  }
  
  if (teamWonGame) {
    bool grnWin = greensWinGame();
    bool redWin = redsWinGame();
    
    grn1State = grnWin && flashState;
    grn2State = grnWin && flashState;
    red1State = redWin && flashState;
    red2State = redWin && flashState;
  } else if (teamWonSet) {
    // Flash all
    grn1State = grn1State && flashState;
    grn2State = grn2State && flashState;
    red1State = red1State && flashState;
    red2State = red2State && flashState;
  }

  if (grn1State) {
    leds.setColorRGB(0, 0, 255, 0);
  } else if (red1State) {
    leds.setColorRGB(0, 255, 0, 0);
  } else {
    leds.setColorRGB(0, 0, 0, 0);
  }

  if (grn2State) {
    leds.setColorRGB(1, 0, 255, 0);
  } else if (red2State) {
    leds.setColorRGB(1, 255, 0, 0);
  } else {
    leds.setColorRGB(1, 0, 0, 0);
  }
}

