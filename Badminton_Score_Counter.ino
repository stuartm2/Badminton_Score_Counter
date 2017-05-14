
#define BUTTON_DEBOUNCE_DELAY   50   // [ms]

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

#define RESET_PIN 10
// define BUZZER_PIN 11
#define GRN_DECR_PIN 1
#define GRN_INCR_PIN 0
#define RED_DECR_PIN 8
#define RED_INCR_PIN 9

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
bool grnIncrBtnState = false;
bool grnDecrBtnState = false;
bool redIncrBtnState = false;
bool redDecrBtnState = false;
bool resetBtnState = false;

int flashDuration = 250;
long lastFlashed = 0;
bool flashState = false;

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

  pinMode(GRN_INCR_PIN, INPUT_PULLUP);
  pinMode(GRN_DECR_PIN, INPUT_PULLUP);
  pinMode(RED_INCR_PIN, INPUT_PULLUP);
  pinMode(RED_DECR_PIN, INPUT_PULLUP);

  /*
  pinMode(GRN_SET1_PIN, OUTPUT);
  pinMode(GRN_SET2_PIN, OUTPUT);
  pinMode(GRN_SET3_PIN, OUTPUT);
  pinMode(RED_SET1_PIN, OUTPUT);
  pinMode(RED_SET2_PIN, OUTPUT);
  pinMode(RED_SET3_PIN, OUTPUT);
  */
  pinMode(RESET_PIN, INPUT_PULLUP);
  //pinMode(BUZZER_PIN, OUTPUT);
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
  
  if (timeNow > idLastPressed + BUTTON_DEBOUNCE_DELAY) {
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

void showSets() {
  /*
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
  */
}

