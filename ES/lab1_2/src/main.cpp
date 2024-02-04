#include <LiquidCrystal.h>
#include <Keypad.h>
#include <stdio.h>

const int LED_RED = 10;
const int LED_GREEN = 11;

const int DELAY = 1000;

const int PASSWORD_LENGTH = 5;

LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

const int ROWS = 4;
const int COLS = 4;

byte rowPins[ROWS] = {3, 2, 19, 18};
byte colPins[COLS] = {17, 16, 15, 14};

char keys[ROWS][COLS] = {
    {'7', '8', '9', '/'},
    {'4', '5', '6', '*'},
    {'1', '2', '3', '-'},
    {'C', '0', '=', '+'}};

Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int keyCount = 0;

const char PASSPORT[PASSWORD_LENGTH] = "1234";
char introducedPassport[PASSWORD_LENGTH];

void ClearKeypad()
{
  lcd.clear();
  lcd.print("Password: ");
  lcd.setCursor(0, 1);
  lcd.cursor();
}

void setup()
{
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  lcd.begin(16, 2);
  ClearKeypad();
}

void loop()
{
  char customKey = customKeypad.getKey();

  if (customKey == 'C')
  {
    ClearKeypad();
    keyCount = 0;
  }

  if (customKey == '=')
  {
    if (strncmp(introducedPassport, PASSPORT, keyCount) == 0)
    {
      ClearKeypad();
      digitalWrite(LED_GREEN, HIGH);
      lcd.print("Door Unlocked!");
      delay(DELAY);
      ClearKeypad();
      digitalWrite(LED_GREEN, LOW);
    }
    else
    {
      ClearKeypad();
      digitalWrite(LED_RED, HIGH);
      lcd.print("Invalid Password!");
      delay(DELAY);
      ClearKeypad();
      digitalWrite(LED_RED, LOW);
    }
    keyCount = 0; // Reset key count after password check
  }

  if (customKey && (keyCount < 4) && (customKey != '=') && (customKey != 'C'))
  {
    lcd.print('*');
    introducedPassport[keyCount] = customKey;
    keyCount++;
  }
}