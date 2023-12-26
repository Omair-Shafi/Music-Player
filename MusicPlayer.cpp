
#include "Timer.h"
#include "LiquidCrystal.h"
#include "pitches.h"

// == Song 1 ==
int song1[] = {
  NOTE_E5,  NOTE_E5,  REFS0,     NOTE_E5,  REFS0,
  NOTE_C5,  NOTE_E5,  NOTE_G5,   REFS0,    NOTE_G4,
  REFS0,    NOTE_C5,  NOTE_G4,   REFS0,    NOTE_E4,
  NOTE_A4,  NOTE_B4,  NOTE_AS4,  NOTE_A4,  NOTE_G4,
  NOTE_E5,  NOTE_G5,  NOTE_A5,   NOTE_F5,  NOTE_G5 };
int song1_time[] = {
  8, 8, 8, 8, 8,
  8, 8, 4, 4, 8,
  4, 4, 8, 4, 4,
  4, 4, 8, 4, 8,
  8, 8, 4, 8, 8 };

// == Song 2 ==
int song2[] = {
  NOTE_C5, NOTE_C5, NOTE_C5, REFS0, NOTE_C5, NOTE_D5, NOTE_E5,
  REFS0,
  NOTE_C5, NOTE_C5, NOTE_C5, REFS0, NOTE_C5, NOTE_D5,
  NOTE_E5, NOTE_C5, NOTE_A4, NOTE_G4,
  NOTE_E5, NOTE_E5, REFS0, NOTE_E5, REFS0, NOTE_C5, NOTE_E5,
  NOTE_G5, REFS0, NOTE_G4, REFS0,
  NOTE_C5, NOTE_G4, REFS0, NOTE_E4 };
int song2_time[] = {
  8, 4, 8, 8, 8, 8, 8,
  1,
  8, 4, 8, 8, 8, 4,
  8, 4, 8, 2,
  8, 8, 8, 8, 8, 8, 4,
  4, 4, 4, 4,
  4, 8, 4, 4 };

// == Song 3 == 
int song3[] = {
  NOTE_C5, NOTE_G4, NOTE_E4,
  NOTE_A4, NOTE_B4, NOTE_A4, NOTE_GS4, NOTE_AS4, NOTE_GS4,
  NOTE_G4, NOTE_D4, NOTE_E4 };
int song3_time[] = {
  4, 4, 4,
  8, 8, 8, 8, 8, 8,
  8, 8, 2 };

// Constants for joystick pins
const int joyY = A0;
const int joyX = A1;
const int sw = 10;

// LCD variables
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Variables to store current position
int x = 1;
int y = 1;
int currSong = 0;
int currScreen = 1;
int prevScreen = 0;

// Variables to store joystick values
int joyValX;
int joyValY;
bool buttonPressed = LOW;

// Variables to store cursor position
int cursorColumn = 6;
int cursorRow = 0;

// Sound variables
int buzzer = 8;
bool isPlaying = true;
int currentNote = 0;
int runSM = 0;

const unsigned long periodLCDOutput = 100;
const unsigned long periodJoystickInput = 100;
const unsigned long periodSoundOutput = 100;
const unsigned long periodController = 500;
const unsigned long tasksPeriodGCD = 100;
unsigned long noteDuration = 100;

enum Joystick { IDLE, LEFT, RIGHT, UP, DOWN };
Joystick xState = IDLE;
Joystick yState = IDLE;

enum Display { MENU, SONG1, SONG2, SONG3 };
Display showState = MENU;

enum SO_States { SO_init, SO_SoundOn, SO_SoundOff };
SO_States SO_State = SO_init;

void TimerISR() {
    TimerFlag = 1;
}

void setup() {
    // Initialize LCD
    lcd.begin(16, 2);  // Set the LCD size to 16 columns and 2 rows
    pinMode(sw, INPUT_PULLUP);
    // Initialize serial communication for debugging
    Serial.begin(9600);
    // Set initial cursor position
    lcd.setCursor(cursorColumn, cursorRow);
    lcd.cursor();
    //initialLCDState();
    runSM = TickFct_LCDOutput(showState);

    TimerSet(tasksPeriodGCD);
    TimerOn();
}

void loop() {
    // Read the joystick values
    int xValue = analogRead(joyX);
    int yValue = analogRead(joyY);

    runSM = TickFct_xJoystickInput(xValue);
    runSM = TickFct_yJoystickInput(yValue);

    lcd.setCursor(cursorColumn, cursorRow);
    lcd.cursor();

    joystickPressState();

    runSM = TickFct_LCDOutput(showState);

    if (currScreen == 2) {
        runSM = TickFct_SoundOutput(SO_State);
    }
}

int TickFct_xJoystickInput(int xValue) {
    switch (xState) {
    case IDLE:
        if (xValue < 400) {
            xState = LEFT;
        }
        else if (xValue > 600) {
            xState = RIGHT;
        }
        break;
    case LEFT:
        if (xValue > 600) {
            xState = RIGHT;
        }
        else if (xValue >= 400 && xValue <= 600) {
            xState = IDLE;
        }
        break;
    case RIGHT:
        if (xValue < 400) {
            xState = LEFT;
        }
        else if (xValue >= 400 && xValue <= 600) {
            xState = IDLE;
        }
        break;
    }

    switch (xState) {
    case IDLE:

        break;
    case LEFT:
        if (x > 1) {
            x--;
        }
        if (cursorColumn > 6) {
            cursorColumn--;
        }
        break;
    case RIGHT:
        if (x < 2) {
            x++;
        }
        if (cursorColumn < 7) {
            cursorColumn++;
        }
        break;
    }
    return 0;
}

int TickFct_yJoystickInput(int yValue) {
    switch (yState) {
    case IDLE:
        if (yValue < 100) {
            yState = DOWN;
        }
        else if (yValue > 900) {
            yState = UP;
        }
        break;
    case UP:
        if (yValue > 900) {
            yState = UP;
        }
        else if (yValue >= 100 && yValue <= 900) {
            yState = IDLE;
        }
        break;
    case DOWN:
        if (yValue < 100) {
            yState = DOWN;
        }
        else if (yValue >= 100 && yValue <= 900) {
            yState = IDLE;
        }
        break;
    }

    switch (yState) {
    case IDLE:
        break;
    case UP:
        if (y < 2) {
            y++;
        }
        if (cursorRow < 1) {
            cursorRow++;
        }
        break;
    case DOWN:
        if (y > 1) {
            y--;
        }
        if (cursorRow > 0) {
            cursorRow--;
        }
        break;
    }
    return 0;
}

void joystickPressState() {
    buttonPressed = digitalRead(sw) == LOW;
    if (currScreen == 1) {
        if (x == 1 && y == 1 && buttonPressed) {  //1
            lcd.setCursor(6, 0);
            lcd.print("* ");
            lcd.setCursor(6, 1);
            lcd.print(" ");
            currSong = 1;
        }
        else if (x == 1 && y == 2 && buttonPressed) {  //3
            lcd.setCursor(6, 0);
            lcd.print("  ");
            lcd.setCursor(6, 1);
            lcd.print("*");
            currSong = 3;
        }
        else if (x == 2 && y == 1 && buttonPressed) {  //2
            lcd.setCursor(6, 0);
            lcd.print(" *");
            lcd.setCursor(6, 1);
            lcd.print(" ");
            currSong = 2;
        }
        else if (x == 2 && y == 2 && buttonPressed && currSong != 0) {   //start
            currScreen = 2;
        }
        runSM = TickFct_LCDOutput(showState);
    }
    else if (currScreen == 2) {
        if (x == 1 && y == 2 && buttonPressed) {  //pause
            lcd.setCursor(6, 1);
            lcd.print("* ");
            isPlaying = false;

            lcd.setCursor(5, 1);
        }
        else if (x == 2 && y == 2 && buttonPressed) {  //play
            lcd.setCursor(6, 1);
            lcd.print(" *");
            isPlaying = true;

            lcd.setCursor(5, 1);
        }
        else {

        }
    }
}

int TickFct_LCDOutput(int showState) {
    switch (showState) { // State Transitions
    case MENU:
        if (currSong == 1 && currScreen == 2) {
            showState = SONG1;
        }
        else if (currSong == 2 && currScreen == 2) {
            showState = SONG2;
        }
        else if (currSong == 3 && currScreen == 2) {
            showState = SONG3;
        }
        break;
    case SONG1:
        if (currSong == 0 && currScreen == 1) {
            showState = MENU;
        }
        break;
    case SONG2:
        if (currSong == 0 && currScreen == 1) {
            showState = MENU;
        }
        break;
    case SONG3:
        if (currSong == 0 && currScreen == 1) {
            showState = MENU;
        }
        break;
    }

    switch (showState) { // State Actions
    case MENU:
        initialLCDState();
        break;
    case SONG1:
        songLCDState();
        break;
    case SONG2:
        songLCDState();
        break;
    case SONG3:
        songLCDState();
        break;
    }
    return 0;
}

int TickFct_SoundOutput(int state) {
    switch (state) { // State Transitions
    case SO_init:
        state = SO_SoundOn;
        if (currSong == 1 && currentNote >= sizeof(song1) / sizeof(song1[0])) {
            state = SO_SoundOff;
        }
        if (currSong == 2 && currentNote >= sizeof(song2) / sizeof(song2[0])) {
            state = SO_SoundOff;
        }
        if (currSong == 3 && currentNote >= sizeof(song3) / sizeof(song3[0])) {
            state = SO_SoundOff;
        }
        break;
    case SO_SoundOn:
        state = SO_SoundOff;
        break;
    case SO_SoundOff:
        break;
    }
    switch (state) { // State Actions
    case SO_SoundOn:
        if (isPlaying) {
            if (currSong == 1 && currentNote < sizeof(song1) / sizeof(song1[0])) {
                noteDuration = 1000 / song1_time[currentNote];
                tone(buzzer, song1[currentNote], noteDuration);
                //delay(noteDuration);
                TimerSet(noteDuration);
                while (!TimerFlag) {}
                TimerFlag = 0;
                noTone(buzzer);
                currentNote++;
            }
            else if (currSong == 2 && currentNote < sizeof(song2) / sizeof(song2[0])) {
                noteDuration = 1000 / song2_time[currentNote];
                tone(buzzer, song2[currentNote], noteDuration);
                //delay(noteDuration);
                TimerSet(noteDuration);
                //for(int i = 0; i<2 ; i++) {
                while (!TimerFlag) {}
                TimerFlag = 0;
                //}
                noTone(buzzer);
                currentNote++;
            }
            else if (currSong == 3 && currentNote < sizeof(song3) / sizeof(song3[0])) {
                noteDuration = 1000 / song3_time[currentNote];
                tone(buzzer, song3[currentNote], noteDuration);
                //delay(noteDuration);
                TimerSet(noteDuration);
                while (!TimerFlag) {}
                TimerFlag = 0;
                noTone(buzzer);
                currentNote++;
            }
        }
        break;
    case SO_SoundOff: //end here
        currSong = 0;
        currScreen = 1;
        currentNote = 0;
        /*
        lcd.clear();
        lcd.print(5);
        delay(2000);
        */
        break;

    }
    return 0;
}

void initialLCDState() {
    if (currScreen == prevScreen) {
        //
    }
    else {
        lcd.clear();          // starting position
        lcd.setCursor(0, 0);  // range is (0-15, 0-1)
        lcd.print("Song 1");
        lcd.setCursor(8, 0);
        lcd.print("Song 2");
        lcd.setCursor(0, 1);
        lcd.print("Song 3");
        lcd.setCursor(8, 1);
        lcd.print("Start");
    }
    prevScreen = currScreen;
}

void songLCDState() {
    if (currScreen == prevScreen) {
        //
    }
    else {
        lcd.clear();          // starting position
        lcd.setCursor(0, 0);  // range is (0-15, 0-1)
        lcd.print("Playing  Song ");
        lcd.print(currSong);
        lcd.setCursor(0, 1);
        lcd.print("Pause    Play");
        lcd.setCursor(5, 1);
    }
    prevScreen = currScreen;
}