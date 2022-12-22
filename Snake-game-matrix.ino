#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <String.h>

// SETTINGS INTERVALS //
#define MIN_LCD_CONTRAST_VALUE 200
#define MAX_LCD_CONTRAST_VALUE 60
#define MAX_LCD_BRIGHTNESS_VALUE 250
#define MIN_LCD_BRIGHTNESS_VALUE 60
#define UPDATE_SNAKE_DOTS_INTERVAL 600


#define WELCOME_MESSAGE_STATE 0
#define MENU_STATE 1
#define SETTINGS_MENU_STATE 2
#define HIGHSCORE_STATE 3
#define ABOUT_STATE 4
#define HOW_TO_PLAY_STATE 5
#define GAME_STATE 6

#define DIFFICULTY_STATE 7
#define LCD_CONTRAST_STATE 8
#define LCD_BRIGHTNESS_STATE 9
#define MATRIX_BRIGHTNESS_STATE 10
#define SOUND_STATE 11
#define NAME_STATE 12
#define RESET_HIGHSCORE_STATE 13
#define DIFFICULTY_EASY_STATE 14
#define DIFFICULTY_MEDIUM_STATE 15
#define DIFFICULTY_HARD_STATE 16
#define SOUND_ON_STATE 17
#define SOUND_OFF_STATE 18

#define MATRIX_SIZE 8
#define MATRIX_BRIGHTNESS 2

#define WELCOME_STATE_INTERVAL 3500
#define WALL_BLINK_INTERVAL 1000

#define MENU_ITEMS_NR 7
#define SETTINGS_ITEMS_NR 8
#define HIGHSCORE_ITEMS_NR 7
#define DIFFICULTY_ITEMS_NR 5
#define SOUND_ITEMS_NR 4

#define BUTTON_PRESSED 1
#define BUTTON_NOT_PRESSED 0

#define LEFT 1
#define RIGHT 2
#define DOWN 3
#define UP 4

#define JOYSTICK_MIN_TRESHOLD 400
#define JOYSTICK_MAX_TRESHOLD 600

const byte pinSW = 2;
byte swJoystickState = HIGH;
byte reading = HIGH;
byte lastReading = HIGH;

unsigned long lastDebounceTime = 0;
unsigned int debounceDelay = 50;

const byte pinX = A0;
const byte pinY = A1;

int xValue = 0;
int yValue = 0;

byte xJoystickState = 0;
byte yJoystickState = 0;

bool joystickMoved = false;

const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  // DIN, CLK, LOAD, No. DRIVER

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 13;
const byte d7 = 4;

const byte lcdContrastPin = 11;
const byte lcdBrightnessPin = 5;
const byte buzzerPin = 3;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte contrastLCD;

const byte arrowDown[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};

const byte arrowUp[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

const byte rightArrow[8] = {
  B00000,
  B00100,
  B01000,
  B11111,
  B01000,
  B00100,
  B00000,
  B00000
};

unsigned long welcomeMessageTimer = millis();

const char* menuItems[] = {
  "Snake game:",
  "1.Play",
  "2.Highscore",
  "3.Settings",
  "4.About",
  "5.How to play",
  "6.Reset Highscore"
};

const char* settingsItems[] = {
  "Game settings:",
  "1.Difficulty",
  "2.LCD contrast",
  "3.LCD light",
  "4.Matrix light",
  "5.Sound",
  "6.Name",
  "7.Back to menu"
};

const char* aboutItems[] = {
  "Mihai Birsan",
  "IG: mihai.birsan"
};

const char* difficultyItems[] = {
  "Top highscores:",
  "-Easy",
  "-Medium",
  "-Hard",
  "Back to menu",
};

const char* soundItems[] = {
  "Sound:",
  "-On",
  "-Off",
  "Back to menu",
};

byte menuIndex = 0;
byte menuIndex2 = 0;

byte settingsIndex = 0;
byte settingsIndex2 = 0;

byte highscoreIndex = 0;
byte highscoreIndex2 = 0;

byte difficultyIndex = 0;
byte difficultyIndex2 = 0;

byte soundIndex = 0;
byte soundIndex2 = 0;

struct Settings {
  byte difficulty;
  byte lcdContrast;
  byte lcdBrightness;
  byte matrixBrightness;
  bool sound;
  char name[4];
} settings;

byte points = 0;

bool matrixChanged = true;

byte matrix[MATRIX_SIZE][MATRIX_SIZE] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;

byte genRow;
byte genCol;

unsigned long long lastMoved = 0;

struct ScoareOfAPlayer {
  char records[5][8];
} highscore;

char* highscoreItems[] = {
  "Highscores",
  highscore.records[0],
  highscore.records[1],
  highscore.records[2],
  highscore.records[3],
  highscore.records[4],
  "Back to menu",
};

bool highscoreReset = false;

byte highscoreItemIndex = 0;
byte highscoreSelectedItemIndex = 0;

struct Position {
  byte row, col;
};

Position snakeWalk[MATRIX_SIZE * MATRIX_SIZE];
int snakeSpeedInterval;

unsigned long long wallBlinkTimestamp = millis();
bool activeWallBlink = true;

Position walls[20];

bool snakeMoved = false;

short snakeHeadX;
short snakeHeadY;
byte snakeJoystickX;
byte snakeJoystickY;
byte foodPosX;
byte foodPosY;

byte snakeDotsCount = 0;
byte snakeLength;

byte foodContor = 0;

byte ct = 0;

byte currentState = WELCOME_MESSAGE_STATE;

bool firstPlay = true;

bool isFoodValid = false;

void loadFromStorage() {
  EEPROM.get(0, settings);
}

void writeInStorage() {
  EEPROM.put(0, settings);
}

void loadHighscoreFromStorage() {
  EEPROM.get(sizeof(settings), highscore);
}

void writeHighscoreInStorage() {
  EEPROM.put(sizeof(settings), highscore);
}

void setup() {
  // settings.lcdBrightness = 5;
  // settings.matrixBrightness = 5;
  // writeInStorage();

  // strcpy(highscore.records[0], "14");
  // strcpy(highscore.records[1], "13");
  // strcpy(highscore.records[2], "12"); 
  // strcpy(highscore.records[3], "11"); 
  // strcpy(highscore.records[4], "10"); 
  // writeHighscoreInStorage();


  Serial.begin(9600);

  pinMode(pinSW, INPUT_PULLUP);
  pinMode(lcdContrastPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  loadFromStorage();
  loadHighscoreFromStorage();
  printHighscoreRecords();

  lc.shutdown(0, false);
  lc.setIntensity(0, settings.matrixBrightness);
  lc.clearDisplay(0);

  lcd.begin(16, 2);
  
  analogWrite(lcdBrightnessPin, map(settings.lcdBrightness, 1, 10, MIN_LCD_BRIGHTNESS_VALUE, MAX_LCD_BRIGHTNESS_VALUE));
  // Serial.println(settings.lcdBrightness);

  lcd.createChar(0, arrowDown);
  lcd.createChar(1, arrowUp);
  lcd.createChar(2, rightArrow);
}

void displayWelcomeMessage() {

  lcd.setCursor(0, 0);
  lcd.print("Welcome darling!");

  lcd.setCursor(4, 1);
  lcd.print("~Snake~");

  if (millis() - welcomeMessageTimer > WELCOME_STATE_INTERVAL) {
    lcd.clear();
    currentState = MENU_STATE;
  }
}

void printHighscoreRecords () {
  for (byte i = 0; i < 5; i++) {
    Serial.print("Record ");
    Serial.print(i);
    Serial.print(" :");
    Serial.println(highscore.records[i]);
  }
}

void resetHighscore() {
  for (byte i = 0; i <= 5; i++) {
    if (i == 0) {
      strcpy(highscore.records[i], "");
      continue;
    }
    
    strcpy(highscore.records[i], "?");
  }

  writeHighscoreInStorage();

}

void goToState(byte state) {
  lcd.clear();
  currentState = state;
  swJoystickState = BUTTON_NOT_PRESSED;
  menuSound();
}

void getNextState() {

  getButtonState();

  if (swJoystickState == BUTTON_PRESSED) {

    switch (currentState) {

      case MENU_STATE:
        if (menuIndex2 == 1) {
          goToState(GAME_STATE);
          return;
        }

        if (menuIndex2 == 2) {
          goToState(HIGHSCORE_STATE);
          return;
        }

        if (menuIndex2 == 3) {
          goToState(SETTINGS_MENU_STATE);
          return;
        }

        // Serial.println(menuIndex);

        if (menuIndex2 == 4) {
          goToState(ABOUT_STATE);
          return;
        }

        if (menuIndex2 == 5) {
          goToState(HOW_TO_PLAY_STATE);
          return;
        }

        if (menuIndex2 == 6) {
          goToState(RESET_HIGHSCORE_STATE);
          return;
        }

      case SETTINGS_MENU_STATE:
        if (settingsIndex2 == 1) {
          goToState(DIFFICULTY_STATE);
          return;
        }

        if (settingsIndex2 == 2) {
          goToState(LCD_CONTRAST_STATE);
          return;
        }

        if (settingsIndex2 == 3) {
          goToState(LCD_BRIGHTNESS_STATE);
          return;
        }

        if (settingsIndex2 == 4) {
          goToState(MATRIX_BRIGHTNESS_STATE);
          return;
        }

        if (settingsIndex2 == 5) {
          goToState(SOUND_STATE);
          return;
        }

        if (settingsIndex2 == 6) {
          goToState(NAME_STATE);
          return;
        }

        if (settingsIndex2 == 7) {
          goToState(MENU_STATE);
          return;
        }

      case HIGHSCORE_STATE:
        if (highscoreIndex2 == 6 && highscoreReset == false) {
          goToState(MENU_STATE);
          return;
        }
        if (highscoreReset == true) {
          goToState(MENU_STATE);
          return;
        }
        
      case ABOUT_STATE:
        goToState(MENU_STATE);
        return;

      case HOW_TO_PLAY_STATE:
        goToState(MENU_STATE);
        return;

      case LCD_CONTRAST_STATE:
        goToState(SETTINGS_MENU_STATE);
        return;

      case DIFFICULTY_STATE:
        if (difficultyIndex2 == 1) {
          goToState(DIFFICULTY_EASY_STATE);
          return;
        }
        if (difficultyIndex2 == 2) {
          goToState(DIFFICULTY_MEDIUM_STATE);
          return;
        }
        if (difficultyIndex2 == 3) {
          goToState(DIFFICULTY_HARD_STATE);
          return;
        }
        if (difficultyIndex2 == 4) {
          goToState(SETTINGS_MENU_STATE);
          return;
        }

      case DIFFICULTY_EASY_STATE:
        goToState(DIFFICULTY_STATE);
        return;

      case DIFFICULTY_MEDIUM_STATE:
        goToState(DIFFICULTY_STATE);
        return;

      case DIFFICULTY_HARD_STATE:
        goToState(DIFFICULTY_STATE);
        return;

      case SOUND_STATE:
        if (soundIndex2 == 1) {
          goToState(SOUND_ON_STATE);
          return;
        }
        if (soundIndex2 == 2) {
          goToState(SOUND_OFF_STATE);
          return;
        }
        if (difficultyIndex2 == 3) {
          goToState(SETTINGS_MENU_STATE);
          return;
        }

      case SOUND_ON_STATE:
        goToState(SOUND_STATE);
        return;

      case SOUND_OFF_STATE:
        goToState(SOUND_STATE);
        return;

      case GAME_STATE:
        goToState(MENU_STATE);
        return;

      case RESET_HIGHSCORE_STATE:
        goToState(MENU_STATE);
        return;

    }
  }
}

byte getButtonState() {

  reading = !digitalRead(pinSW);

  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    if (swJoystickState != reading) {
      swJoystickState = reading;

      if (swJoystickState == LOW) {
        return BUTTON_PRESSED;
      }
    }
  }

  lastReading = reading;
  return BUTTON_NOT_PRESSED;
}

byte getDirection() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);


  if (xValue < JOYSTICK_MIN_TRESHOLD && !joystickMoved) {
    joystickMoved = true;
    xJoystickState = RIGHT;
    return RIGHT;
  }

  if (xValue > JOYSTICK_MAX_TRESHOLD && !joystickMoved) {
    joystickMoved = true;
    xJoystickState = LEFT;
    return LEFT;
  }

  if (yValue < JOYSTICK_MIN_TRESHOLD && !joystickMoved) {
    joystickMoved = true;
    yJoystickState = UP;
    return UP;
  }

  if (yValue > JOYSTICK_MAX_TRESHOLD && !joystickMoved) {
    joystickMoved = true;
    yJoystickState = DOWN;
    return DOWN;
  }

  if (xValue >= JOYSTICK_MIN_TRESHOLD && xValue <= JOYSTICK_MAX_TRESHOLD && yValue >= JOYSTICK_MIN_TRESHOLD && yValue <= JOYSTICK_MAX_TRESHOLD) {
    joystickMoved = false;
    return 0;
  }

  return 0;
}

void getMenuIndex() {

  byte direction = getDirection();

  if (direction == 0 || (direction != UP && direction != DOWN)) {
    return;
  }

  if (direction == DOWN) {
    menuIndex2++;
  }
  else {
    menuIndex2--;
  }

  if (direction == DOWN && menuIndex2 % 2 == 0) {
    menuIndex = menuIndex2;
  } else if (direction == UP && menuIndex2 % 2 == 1) {
    menuIndex = menuIndex2 - 1;
  }

  menuIndex = constrain(menuIndex, 0, MENU_ITEMS_NR - 1);

  lcd.clear();
}

void showMenu() {
  delay(30);

  lcd.setCursor(0, 0);
  lcd.print(menuItems[menuIndex]);  

  if (menuIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)1);
  }

  if (menuIndex != MENU_ITEMS_NR - 1) {
    lcd.setCursor(0, 1);
    lcd.print(menuItems[menuIndex + 1]);
  }

  if (menuIndex < MENU_ITEMS_NR - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)0);
  }
  
  lcd.setCursor(strlen(menuItems[menuIndex2]), menuIndex2 % 2);
  lcd.write((byte)2);

  getNextState();

  getMenuIndex();
}

void getSettingsIndex() {

  byte direction = getDirection();

  if (direction == 0 || (direction != UP && direction != DOWN)) {
    return;
  }

  if (direction == DOWN) {
    settingsIndex2++;
  }
  else {
    settingsIndex2--;
  }

  if (direction == DOWN && settingsIndex2 % 2 == 0) {
    settingsIndex = settingsIndex2;
  }

  else if (direction == UP && settingsIndex2 % 2 == 1) {
    settingsIndex = settingsIndex2 - 1;
  }

  settingsIndex = constrain(settingsIndex, 0, SETTINGS_ITEMS_NR - 1);

  lcd.clear();
}

void showSettings() {
  delay(30);

  lcd.setCursor(0, 0);
  lcd.print(settingsItems[settingsIndex]);

  if (settingsIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)1);
  }

  if (settingsIndex != SETTINGS_ITEMS_NR - 1) {
    lcd.setCursor(0, 1);
    lcd.print(settingsItems[settingsIndex + 1]);
  }

  if (settingsIndex < SETTINGS_ITEMS_NR - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)0);
  }

  lcd.setCursor(strlen(settingsItems[settingsIndex2]), settingsIndex2 % 2);
  lcd.write((byte)2);

  getNextState();

  getSettingsIndex();
}

void getHighscoreIndex() {

  byte direction = getDirection();

  if (direction == 0 || (direction != UP && direction != DOWN)) {
    return;
  }

  if (direction == DOWN) {
    highscoreIndex2++;
  }
  else {
    highscoreIndex2--;
  }

  if (direction == DOWN && highscoreIndex2 % 2 == 0) {
    highscoreIndex = highscoreIndex2;
  } else if (direction == UP && highscoreIndex2 % 2 == 1) {
    highscoreIndex = highscoreIndex2 - 1;
  }

  highscoreIndex = constrain(highscoreIndex, 0, HIGHSCORE_ITEMS_NR - 1);

  lcd.clear();
}

void showHighscore() {
  
  delay(30);
  if(!strlen(highscore.records[0])) {
    lcd.setCursor(0,0);
    lcd.print("No highscore yet");

    getButtonState();
    getNextState();

    return;
  }

  lcd.setCursor(0, 0);
  lcd.print(highscoreItems[highscoreIndex]);  

  if (highscoreIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)1);
  }

  if (highscoreIndex != HIGHSCORE_ITEMS_NR - 1) {
    lcd.setCursor(0, 1);
    lcd.print(highscoreItems[highscoreIndex + 1]);
  }

  if (highscoreIndex < HIGHSCORE_ITEMS_NR - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)0);
  }
  
  lcd.setCursor(strlen(highscoreItems[highscoreIndex2]), highscoreIndex2 % 2);
  lcd.write((byte)2);

  getNextState();

  getHighscoreIndex();
}

void getDifficultyIndex() {

  byte direction = getDirection();

  if (direction == 0 || (direction != UP && direction != DOWN)) {
    return;
  }

  if (direction == DOWN) {
    difficultyIndex2++;
  }
  else {
    difficultyIndex2--;
  }

  if (direction == DOWN && difficultyIndex2 % 2 == 0) {
    difficultyIndex = difficultyIndex2;
  }

  else if (direction == UP && difficultyIndex2 % 2 == 1) {
    difficultyIndex = difficultyIndex2 - 1;
  }

  difficultyIndex = constrain(difficultyIndex, 0, DIFFICULTY_ITEMS_NR - 1);

  lcd.clear();
}

void showDifficulty() {
  delay(30);

  lcd.setCursor(0, 0);
  lcd.print(difficultyItems[difficultyIndex]);

  if (difficultyIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)1);
  }

  if (difficultyIndex != DIFFICULTY_ITEMS_NR - 1) {
    lcd.setCursor(0, 1);
    lcd.print(difficultyItems[difficultyIndex + 1]);
  }

  if (difficultyIndex < DIFFICULTY_ITEMS_NR - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)0);
  }

  lcd.setCursor(strlen(difficultyItems[difficultyIndex2]), difficultyIndex2 % 2);
  lcd.write((byte)2);

  getNextState();

  getDifficultyIndex();
}

void showEasyDifficulty() {
  delay(30);
  settings.difficulty = 1;
  writeInStorage();
  lcd.setCursor(1,0);
  lcd.print("Easy difficulty");
  lcd.setCursor(1,1);
  lcd.print("Press to confirm!");

  getNextState();
}

void showMediumDifficulty() {
  delay(30);
  settings.difficulty = 2;
  writeInStorage();
  lcd.setCursor(1,0);
  lcd.print("Medium difficulty");
  lcd.setCursor(1,1);
  lcd.print("Press to confirm!");

  getNextState();
}

void showHardDifficulty() {
  delay(30);
  settings.difficulty = 3;
  writeInStorage();
  lcd.setCursor(1,0);
  lcd.print("Hard difficulty");
  lcd.setCursor(1,1);
  lcd.print("Press to confirm!");

  getNextState();
}

void getSoundIndex() {

  byte direction = getDirection();

  if (direction == 0 || (direction != UP && direction != DOWN)) {
    return;
  }

  if (direction == DOWN) {
    soundIndex2++;
  }
  else {
    soundIndex2--;
  }

  if (direction == DOWN && soundIndex2 % 2 == 0) {
    soundIndex = soundIndex2;
  }

  else if (direction == UP && soundIndex2 % 2 == 1) {
    soundIndex = soundIndex2 - 1;
  }

  soundIndex = constrain(soundIndex, 0, SOUND_ITEMS_NR - 1);

  lcd.clear();
}

void showSound() {
  delay(30);

  lcd.setCursor(0, 0);
  lcd.print(soundItems[soundIndex]);

  if (soundIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)1);
  }

  if (soundIndex2 != SOUND_ITEMS_NR - 1) {
    lcd.setCursor(0, 1);
    lcd.print(soundItems[soundIndex + 1]);
  }

  if (soundIndex < SOUND_ITEMS_NR - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)0);
  }

  lcd.setCursor(strlen(soundItems[soundIndex2]), soundIndex2 % 2);
  lcd.write((byte)2);

  getNextState();

  getSoundIndex();
}

void showSoundOn() {
  delay(30);
  settings.sound = true;
  writeInStorage();
  lcd.setCursor(1,0);
  lcd.print("Sound ON!");
  lcd.setCursor(1,1);
  lcd.print("Press to confirm!");

  getNextState();
}

void showSoundOff() {
  delay(30);
  settings.sound = false;
  writeInStorage();
  lcd.setCursor(1,0);
  lcd.print("Sound OFF!");
  lcd.setCursor(1,1);
  lcd.print("Press to confirm!");

  getNextState();
}

void menuSound() {
  if (settings.sound == true) {
    tone(buzzerPin, 300, 300);
  }
}

void snakeEatSound() {
  if (settings.sound == true) {
    tone(buzzerPin, 200, 300);
  }
}

void snakeLoseSound() {
  if (settings.sound == true) {
    tone(buzzerPin, 100, 1500);
  }
}


void showAbout() {
  delay(30);

  lcd.setCursor(2, 0);
  lcd.print(aboutItems[0]);

  lcd.setCursor(0, 1);
  lcd.print(aboutItems[1]);

  getNextState();
}

void showHowtoplay() {
  delay(30);

  lcd.setCursor(2, 0);
  lcd.print("Have fun!!!");

  lcd.setCursor(2, 1);
  lcd.print("Eat the food!!!");

  getNextState();
}

void showContrast() {
  delay(30);

  lcd.setCursor(4, 0);
  lcd.print("Use the");
  lcd.setCursor(2, 1);
  lcd.print("potentiometer");

  getNextState();
}

void showResetHighscore() {
  delay(30);

  lcd.setCursor(1,0);
  lcd.print("Reset highscore");
  lcd.setCursor(0,1);
  lcd.print("Press to confirm!");
  highscoreReset = true;

  getNextState();
}

void matrixLightUp() {
  for(byte row =0; row < MATRIX_SIZE; row++){
     for(byte col =0; col < MATRIX_SIZE; col++){
      lc.setLed(0, row, col, true);
      }
   }
}

void matrixLightOff() {
  for(byte row =0; row < MATRIX_SIZE; row++){
     for(byte col =0; col < MATRIX_SIZE; col++){
      lc.setLed(0, row, col, false);
      }
   }
}

void turnOffPoint(Position position) {
  lc.setLed(0, position.col, position.row, false);
}

void turnOnPoint(Position position) {
  lc.setLed(0, position.col, position.row, true);
}

bool snakeOut1() {
  if (snakeHeadX == -1 || snakeHeadX == MATRIX_SIZE || snakeHeadY == -1 || snakeHeadY == MATRIX_SIZE)
    return true;
  return false;
}

bool snakeOut2() {
  for (byte i = 2; i < snakeLength; i++)
    if(snakeWalk[i].row == snakeHeadX && snakeWalk[i].col == snakeHeadY) {
      return true;
    }
      
  return false;
}

bool snakeOut3() {
  for(byte i = 0; i < 20; i++) 
    if (walls[i].row == snakeHeadX && walls[i].col == snakeHeadY) 
      return true;
  return false;
}

void getSnakeDirection() {
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);

  if (xValue >= JOYSTICK_MIN_TRESHOLD && xValue <= JOYSTICK_MAX_TRESHOLD && yValue >= JOYSTICK_MIN_TRESHOLD && yValue <= JOYSTICK_MAX_TRESHOLD)
    snakeMoved = false;

  if (xValue > JOYSTICK_MAX_TRESHOLD && snakeMoved == false && snakeJoystickX == 0) {
    snakeJoystickX = RIGHT;
    snakeJoystickY = 0;
    snakeMoved = true;
  }
  else if (xValue < JOYSTICK_MIN_TRESHOLD && snakeMoved == false && snakeJoystickX == 0) {
    snakeJoystickX = LEFT;
    snakeJoystickY = 0;
    snakeMoved = true;
  }

  if (yValue > JOYSTICK_MAX_TRESHOLD && snakeMoved == false && snakeJoystickY == 0) {
    snakeJoystickY = UP;
    snakeJoystickX = 0;
    snakeMoved = true;
  }
  else if (yValue < JOYSTICK_MIN_TRESHOLD && snakeMoved == false && snakeJoystickY == 0) {
    snakeJoystickY = DOWN;
    snakeJoystickX = 0;
    snakeMoved = true;
  }
}

void nextPositionSnake() {
  if (snakeJoystickX == RIGHT)
    snakeHeadX += 1;
  else if (snakeJoystickX == LEFT)
    snakeHeadX -= 1;

  if (snakeJoystickY == UP)
    snakeHeadY += 1;
  else if (snakeJoystickY == DOWN)
    snakeHeadY -= 1;

  matrix[snakeWalk[snakeLength-1].row][snakeWalk[snakeLength-1].col] = 0;
  for (byte i = snakeLength; i >= 1; i--) {
    snakeWalk[i] = snakeWalk[i-1];
  }

  snakeWalk[0] = {snakeHeadX, snakeHeadY};

  matrix[snakeHeadX][snakeHeadY] = 1;
  matrixChanged = true;
  lastMoved = millis();

}

void generateFood() {
  
  foodPosX = random(MATRIX_SIZE);
  foodPosY = random(MATRIX_SIZE);
}

void generateRightFood() {
  isFoodValid = false;
  while (!isFoodValid) {
      generateFood();
      isFoodValid = true;
      for (byte i = 0; i < snakeLength; i++)
        if (snakeWalk[i].row == foodPosX && snakeWalk[i].col == foodPosY) {
          isFoodValid = false;
          break;
        }
      if (settings.difficulty > 1) {
        for (byte i = 0; i < 20; i++) {
          if (walls[i].row == foodPosX && walls[i].col == foodPosY) {
            isFoodValid = false;
            break;
          }
        }
      }
    }

}

void activateFood(byte foodPosX, byte foodPosY) {
  lc.setLed(0, foodPosX, foodPosY, true);
}

void deactivateFood(byte foodPosX, byte foodPosY) {
  lc.setLed(0, foodPosX, foodPosY, false);
}

void generateWall1() {
  for (byte i = 0; i < 3; i++) {
    matrix[0][i] = 1;
    ct++;
    walls[ct] = {0, i};
    blinkDot(walls[ct]);

    matrix[7][i] = 1;
    ct++;
    walls[ct] = {7, i};
    blinkDot(walls[ct]);
  }

  for (byte i = 1; i < 7; i++) {
    matrix[i][4] = 1;
    ct++;
    walls[ct] = {i, 4};
    blinkDot(walls[ct]);
  }
} 

void generateWall2() {
  matrix[3][0] = 1;
  ct++;
  walls[ct] = {3, 0};
  blinkDot(walls[ct]);

  matrix[4][0] = 1;
  ct++;
  walls[ct] = {4, 0};
  blinkDot(walls[ct]);

  matrix[1][6] = 1;
  ct++;
  walls[ct] = {1, 6};
  blinkDot(walls[ct]);

  matrix[6][6] = 1;
  ct++;
  walls[ct] = {6, 6};
  blinkDot(walls[ct]);

  matrix[3][6] = 1;
  ct++;
  walls[ct] = {3, 6};
  blinkDot(walls[ct]);

  matrix[4][6] = 1;
  ct++;
  walls[ct] = {4, 6};
  blinkDot(walls[ct]);

  matrix[3][7] = 1;
  ct++;
  walls[ct] = {3, 7};
  blinkDot(walls[ct]);

  matrix[4][7] = 1;
  ct++;
  walls[ct] = {4, 7};
  blinkDot(walls[ct]);
}

// void generateWall(byte index) {
//   byte ct = 0;
//   for (byte i = index; i < MATRIX_SIZE; i++) {
//     if (i != 3 && i != 4) {
//       matrix[index][i] = 1;
//       ct++;
//       walls[ct] = {index, i};
//       blinkDot(walls[ct]);
//     }
//     if ((i >= (index + 1)) || (i <= (MATRIX_SIZE - 1))) {
//       matrix[i][MATRIX_SIZE - index - 1] = 1;
//       ct++;
//       walls[ct] = {i, MATRIX_SIZE - index - 1};
//       blinkDot(walls[ct]);
//       matrix[i][index] = 1;
//       ct++;
//       walls[ct] = {i, index};
//       blinkDot(walls[ct]);
//     }
//     matrix[MATRIX_SIZE - index - 1][i] = 1;
//     ct++;
//     walls[ct] = {MATRIX_SIZE - index - 1, i};
//     blinkDot(walls[ct]);
//   }
//   // matrix[index][4] = 0;
//   // matrix[index][3] = 0;
//   // matrix[index][index...matrix_size - index]
//   // matrix[matrix_size - index][index...matrix_size - index]
//   // matrix[index...matrix_size - index][index]
//   // matrix[index...matrix_size - index][matrix_size - index]
// }


void updateMatrix(byte foodPosX, byte foodPosY) {
  for (byte i = 0; i < MATRIX_SIZE; i++) {
    for (byte j = 0; j < MATRIX_SIZE; j++) {
      if (i != foodPosX || j != foodPosY)
        lc.setLed(0, i, j, matrix[i][j]);
    }
  }
}

void playSnake(byte param) {
  if (firstPlay == true) {
    snakeHeadX = 1;
    snakeHeadY = 2;
    snakeLength = 3;
    snakeWalk[0] = {1,2};
    snakeWalk[1] = {1,1};
    snakeWalk[2] = {1,0};
    
    for (int i =  0; i < snakeLength; i++) {
      matrix[snakeWalk[i].row][snakeWalk[i].col] = 1;
    }
   
    generateRightFood();
    updateMatrix(foodPosX, foodPosY);
    matrixChanged = false;

    if(param == 2)
      generateWall1();  
    if(param == 3) {
      generateWall1();
      generateWall2();
    }

    lastMoved = 0;
    snakeJoystickX = 0;
    snakeJoystickY = RIGHT;
    firstPlay = false;
  }  

  if (snakeOut1()) 
    return;

  if (snakeOut3())
    return;

  showSnakeGame();

  if (millis() - lastMoved > UPDATE_SNAKE_DOTS_INTERVAL - 100 * param)
    nextPositionSnake();

  // activateFood(foodPosX, foodPosY);
  blinkDotPos(foodPosX, foodPosY);

  getSnakeDirection();

  

  if(snakeHeadX == foodPosX && snakeHeadY == foodPosY) {
    snakeEatSound();
    matrix[foodPosX][foodPosY] = 1;
    matrixChanged = true;
    generateRightFood();
    snakeLength += 1; 
  }

  if(matrixChanged == true) {
    updateMatrix(foodPosX, foodPosY);
    matrixChanged = false;
  }

  if (snakeOut2()) 
    return;
}


void showSnakeGameEnded() {
  
  byte pct = snakeLength - 3;
  char pointsDisplay[7] = "Puncte: ";
  lcd.setCursor(0,0);
  lcd.print("The snake died");
  lcd.setCursor(0,1);
  lcd.print(pointsDisplay);
  lcd.setCursor(8,1);
  lcd.print(pct);

  getNextState();
}

void showSnakeGame() {
  
  byte pct = snakeLength - 3;
  char pointsDisplay[7] = "Puncte: ";
  lcd.setCursor(0,0);
  lcd.print("You are playing");
  lcd.setCursor(0,1);
  lcd.print(pointsDisplay);
  lcd.setCursor(8,1);
  lcd.print(pct);

  getNextState();
}

void blinkDot(Position pos) {
  unsigned long time = millis() - wallBlinkTimestamp;
  if (time > WALL_BLINK_INTERVAL) {
    activeWallBlink = !activeWallBlink;
    wallBlinkTimestamp = millis();
  }
  if (activeWallBlink == true)
    turnOnPoint(pos);
  else
    turnOffPoint(pos);
}

void blinkDotPos(byte row, byte col) {
  unsigned long time = millis() - wallBlinkTimestamp;
  if (time > WALL_BLINK_INTERVAL) {
    activeWallBlink = !activeWallBlink;
    wallBlinkTimestamp = millis();
  }
  if (activeWallBlink == true)
    activateFood(row, col);
  else
    deactivateFood(row, col);
}


void showOnCenter(String message, byte row) {
  lcd.setCursor(16 / 2 - message.length() / 2, row);
  lcd.print(message);
}

void showGradation(byte nrBars, byte& updates, short modifierPin = 0, byte minInterval = 0, byte maxInterval = 0) {
  delay(30);
  char bar = 0xff;
  String content = "-";
  byte barIndex = updates;

  for (byte i = 1; i <= barIndex; i++)
    content += bar;
  for (byte i = barIndex + 1; i <= nrBars; i++)
    content += ' ';
  content += '+';

  lcd.clear();
  // Serial.println(settings.lcdBrightness);

  showOnCenter(content, 0);
  showOnCenter("Press to save!", 1);

  while (swJoystickState != 1) {
    int x = getDirection();
    if (xJoystickState != 0) {
      if (x == RIGHT && barIndex < nrBars) {
        barIndex++;
        content[barIndex] = 0xff;
      } else if (x == LEFT && barIndex > 1) {
        content[barIndex] = ' ';
        barIndex--;
      }
      Serial.print("ceva23");

      showOnCenter(content, 0);
      
      byte value = map(barIndex, 1, nrBars, minInterval, maxInterval);

      if (modifierPin > 0) {
        analogWrite(modifierPin, value);
      }
      if (modifierPin == -1) {
        Serial.println("value");
        lc.setIntensity(0, value);
      }

      delay(5);
    }
    getButtonState();
  }
  updates = barIndex;
  writeInStorage();
  // Serial.println(updates);
  goToState(SETTINGS_MENU_STATE);
}

void loop() {
  
  switch (currentState) {

    case WELCOME_MESSAGE_STATE:
      displayWelcomeMessage();
      break;

    case MENU_STATE:
      firstPlay = true;
      showMenu();
      break;

    case SETTINGS_MENU_STATE:
      firstPlay = true;
      showSettings();
      break;

    case ABOUT_STATE:
      firstPlay = true;
      showAbout();
      break;

    case HOW_TO_PLAY_STATE:
      firstPlay = true;
      showHowtoplay();
      break;

    case RESET_HIGHSCORE_STATE:
      firstPlay = true;
      resetHighscore();
      showResetHighscore();
      break;

    case HIGHSCORE_STATE:
      firstPlay = true;
      showHighscore();
      break;

    case GAME_STATE:
      // Serial.println(settings.difficulty);
      playSnake(settings.difficulty);
      if (snakeOut1()) {
        snakeLoseSound();
        lcd.clear();
        showSnakeGameEnded();
      }
      break;

    case DIFFICULTY_STATE:
      firstPlay = true;
      // Serial.println(settings.difficulty);
      showDifficulty();
      break;

    case DIFFICULTY_EASY_STATE:
      showEasyDifficulty();
      break;

    case DIFFICULTY_MEDIUM_STATE:
      showMediumDifficulty();
      break;

    case DIFFICULTY_HARD_STATE:
      showHardDifficulty();
      break;

    case LCD_CONTRAST_STATE:
      firstPlay = true;
      showContrast();
      break;

    case LCD_BRIGHTNESS_STATE:
      firstPlay = true;
      showGradation(10, settings.lcdBrightness, lcdBrightnessPin, MIN_LCD_BRIGHTNESS_VALUE, MAX_LCD_BRIGHTNESS_VALUE);
      break;

    case MATRIX_BRIGHTNESS_STATE:
      firstPlay = true; 
      matrixLightUp();
      showGradation(10, settings.matrixBrightness, -1, 0 , 15);
      break;

    case SOUND_STATE:
      firstPlay = true;
      showSound();
      break;

    case SOUND_ON_STATE:
      showSoundOn();
      break;

    case SOUND_OFF_STATE:
      showSoundOff();
      break;

    case NAME_STATE:
      firstPlay = true;
      break;
  }

}
