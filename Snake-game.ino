#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <String.h>

// SETTINGS INTERVALS //
#define MIN_LCD_CONTRAST_VALUE 200
#define MAX_LCD_CONTRAST_VALUE 60
#define MAX_LCD_BRIGHTNESS_VALUE 250
#define MIN_LCD_BRIGHTNESS_VALUE 60

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

#define MATRIX_SIZE 8
#define MATRIX_BRIGHTNESS 2

#define WELCOME_STATE_INTERVAL 3500

#define MENU_ITEMS_NR 6
#define SETTINGS_ITEMS_NR 7

#define BUTTON_PRESSED 1
#define BUTTON_NOT_PRESSED 0

#define LEFT 1
#define RIGHT 2
#define DOWN 3
#define UP 4

#define JOYSTICK_MIN_TRESHOLD 400
#define JOYSTICK_MAX_TRESHOLD 600

const int minThreshold = 200;
const int maxThreshold = 600;

const byte pinSW = 13;
byte swJoystickState = HIGH;
byte reading = HIGH;
byte lastReading = HIGH;

unsigned long lastDebounceTime = 0;
unsigned int debounceDelay = 50;

const byte pinX = A0;
const byte pinY = A1;

int xValue = 0;
int yValue = 0;

int xJoystickState = 0;
int yJoystickState = 0;

bool joystickMoved = false;

const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  // DIN, CLK, LOAD, No. DRIVER

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;

const byte lcdContrastPin = 11;
const byte lcdBrightnessPin = 3;

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
  "5.How to play"
};

byte menuIndex = 0;
byte menuIndex2 = 0;

const char* settingsItems[] = {
  "Game settings:",
  "1.Difficulty",
  "2.LCD contrast",
  "3.LCD ligth",
  "4.Matrix ligth",
  "5.Sound",
  "6.Back to menu"
};

struct Settings {
  byte difficulty;
  byte lcdContrast;
  byte lcdBrightness;
  byte matrixBrightness;
  bool soundsMuted;
} settings;

byte settingsIndex = 0;
byte settingsIndex2 = 0;

const char* aboutItems[] = {
  "Mihai Birsan",
  "IG: mihai.birsan"
};

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

const byte moveInterval = 100;
unsigned long long lastMoved = 0;

struct ScoareOfAPlayer {
  char name[4];
  short score;
}

ScoareOfAPlayer highscore[5];

byte currentState = WELCOME_MESSAGE_STATE;

void loadFromStorage() {
  EEPROM.get(0, settings);
}

void writeInStorage() {
  EEPROM.put(0, settings);
}

void loadHighscoreFromStorage() {
  EEPROM.get(0, highscore);
}

void setup() {
  // settings.lcdBrightness = 5;
  // settings.matrixBrightness = 5;
  // writeInStorage();

  Serial.begin(9600);

  pinMode(pinSW, INPUT_PULLUP);
  pinMode(lcdContrastPin, OUTPUT);

  loadFromStorage();

  lc.shutdown(0, false);
  lc.setIntensity(0, settings.matrixBrightness);
  lc.clearDisplay(0);

  lcd.begin(16, 2);
  
  analogWrite(lcdBrightnessPin, map(settings.lcdBrightness, 1, 10, MIN_LCD_BRIGHTNESS_VALUE, MAX_LCD_BRIGHTNESS_VALUE));
  Serial.println("sa vedem ce are:");
  Serial.println(settings.lcdBrightness);

  // contrastLCD = EEPROM.read(0);
  // analogWrite(lcdContrastPin, contrastLCD);

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

void goToState(byte state) {
  lcd.clear();
  currentState = state;
  swJoystickState = BUTTON_NOT_PRESSED;
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

        Serial.println(menuIndex);

        if (menuIndex2 == 4) {
          goToState(ABOUT_STATE);
          return;
        }

        if (menuIndex2 == 5) {
          goToState(HOW_TO_PLAY_STATE);
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
          goToState(MENU_STATE);
          return;
        }

      case ABOUT_STATE:
        goToState(MENU_STATE);
        return;

      case HOW_TO_PLAY_STATE:
        goToState(MENU_STATE);
        return;

      case HIGHSCORE_STATE:
        goToState(MENU_STATE);
        return;

      case LCD_CONTRAST_STATE:
        goToState(SETTINGS_MENU_STATE);
        return;

      case DIFFICULTY_STATE:
        goToState(SETTINGS_MENU_STATE);
        return;

      case SOUND_STATE:
        goToState(SETTINGS_MENU_STATE);
        return;

      case GAME_STATE:
        goToState(MENU_STATE);
        return;

      // case LCD_BRIGHTNESS_STATE:
      //   writeInStorage();
      //   goToState(SETTINGS_MENU_STATE);
      //   return;

      // case MATRIX_BRIGHTNESS_STATE:
      //   writeInStorage();
      //   goToState(SETTINGS_MENU_STATE);
      //   return;
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

void showHighscore() {
  delay(30);

  lcd.setCursor(2, 0);
  lcd.print("In future..");
  lcd.setCursor(1, 1);
  lcd.print("Press to exit!");

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

void showDifficulty() {
  delay(30);

  lcd.setCursor(2, 0);
  lcd.print("In future..");
  lcd.setCursor(1, 1);
  lcd.print("Press to exit!");


  getNextState();
}

void showSound() {
  delay(30);

  lcd.setCursor(2, 0);
  lcd.print("In future..");
  lcd.setCursor(1, 1);
  lcd.print("Press to exit!");

  getNextState();
}

void matrixLightUp() {
  for(int row =0; row < MATRIX_SIZE; row++){
     for(int col =0; col < MATRIX_SIZE; col++){
      lc.setLed(0, row, col, true);
      }
   }
}

void playSnake() {
  if (millis() - lastMoved > moveInterval) {
    updatePositions();  // calculare stare
  }

  if (matrixChanged == true) {
    updateMatrix();
    matrixChanged = false;
  }

  if (xPos == genRow && yPos == genCol) {
    generateFood();
  }
}

void generateFood() {
  genRow = random(MATRIX_SIZE);
  genCol = random(MATRIX_SIZE);
  matrix[genRow][genCol] = 1;
  updateMatrix();
}

void updateMatrix() {
  for (int i = 0; i < MATRIX_SIZE; i++) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
      lc.setLed(0, i, j, matrix[i][j]);
    }
  }
}

void updatePositions() {
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);

  xLastPos = xPos;
  yLastPos = yPos;
  if (xValue < minThreshold) {
    xPos--;
    if (xPos < 0) {
      xPos = 7;
    }
  }

  if (xValue > maxThreshold) {
    xPos++;
    if (xPos > 7) {
      xPos = 0;
    }
  }

  if (yValue > maxThreshold) {
    if (yPos < MATRIX_SIZE - 1) {
      yPos++;
    } else {
      yPos = 0;
    }
  }

  if (yValue < minThreshold) {
    if (yPos > 0) {
      yPos--;
    } else {
      yPos = MATRIX_SIZE - 1;
    }
  }

  if (xLastPos != xPos || yLastPos != yPos) {
    matrix[xLastPos][yLastPos] = 0;
    matrix[xPos][yPos] = 1;
    matrixChanged = true;
    lastMoved = millis();
  }
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
  // Serial.println(updates);
  Serial.println("ce are");


  for (byte i = 1; i <= barIndex; i++)
    content += bar;
  for (byte i = barIndex + 1; i <= nrBars; i++)
    content += ' ';
  content += '+';

  lcd.clear();
  Serial.println(settings.lcdBrightness);

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
  Serial.println(updates);
  goToState(SETTINGS_MENU_STATE);
}

void showSnakeGame() {
  delay(30);

  lcd.setCursor(2, 0);
  lcd.print("Have fun!!!");

  getNextState();
}


void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("si acum:");
  Serial.println(settings.lcdBrightness);
  switch (currentState) {

    case WELCOME_MESSAGE_STATE:
      displayWelcomeMessage();
      break;

    case MENU_STATE:
      showMenu();
      break;

    case SETTINGS_MENU_STATE:
      showSettings();
      break;

    case ABOUT_STATE:
      showAbout();
      break;

    case HOW_TO_PLAY_STATE:
      showHowtoplay();
      break;

    case HIGHSCORE_STATE:
      showHighscore();
      break;

    case GAME_STATE:
      playSnake();
      showSnakeGame();
      break;

    case DIFFICULTY_STATE:
      showDifficulty();
      break;

    case LCD_CONTRAST_STATE:
      showContrast();
      break;

    case LCD_BRIGHTNESS_STATE:
      Serial.println("de ce nu merge");
      showGradation(10, settings.lcdBrightness, lcdBrightnessPin, MIN_LCD_BRIGHTNESS_VALUE, MAX_LCD_BRIGHTNESS_VALUE);
      Serial.println("habr nu am");
      break;

    case MATRIX_BRIGHTNESS_STATE:
      matrixLightUp();
      showGradation(10, settings.matrixBrightness, -1, 0 , 15);
      break;

    case SOUND_STATE:
      showSound();
      break;
  }
}