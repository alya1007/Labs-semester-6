#include "scheduler.hpp"
#include <Arduino.h>

int YELLOW_LED = 12;
int RED_LED = 13;

int TOGGLE_BUTTON = 11;
int INC_BUTTON = 10;
int DEC_BUTTON = 9;

unsigned long lastFlicker = millis();

bool isFlicker = false;
bool isButtonPressed = false;

unsigned long flickeringInterval = 2000;

const int MIN_FLICKER_INTERVAL_MS = 500;
const int MAX_FLICKER_INTERVAL_MS = 3000;

int changeFlickeringInterval = 100;

void pressedButtonTask()
{
  bool newIsButtonPressed = digitalRead(TOGGLE_BUTTON);

  if (newIsButtonPressed && !isButtonPressed)
  {
    Serial.println("Button is pressed");
  }
  isButtonPressed = newIsButtonPressed;
}

void checkFlickerYellowTask()
{
  if (digitalRead(RED_LED) == LOW)
  {
    if ((millis() - lastFlicker) > flickeringInterval)
    {
      isFlicker = !isFlicker;
      lastFlicker = millis();
      isFlicker == true ? Serial.println("Yellow led is on") : Serial.println("Yellow led is off");
    }
  }
  else
  {
    isFlicker = false;
  }
  if (digitalRead(DEC_BUTTON))
  {
    flickeringInterval = max(flickeringInterval - changeFlickeringInterval, MIN_FLICKER_INTERVAL_MS);
    Serial.print("Flickering interval: " + String(flickeringInterval) + "ms");
  }
  if (digitalRead(INC_BUTTON))
  {
    flickeringInterval = min(flickeringInterval + changeFlickeringInterval, MAX_FLICKER_INTERVAL_MS);
    Serial.print("Flickering interval: " + String(flickeringInterval) + "ms");
  }
}

void uiTask()
{
  digitalWrite(YELLOW_LED, isFlicker);

  if (isButtonPressed)
  {
    digitalWrite(RED_LED, HIGH);
  }
  else
  {
    digitalWrite(RED_LED, LOW);
  }
}

Scheduler scheduler(3);

void setup()
{
  Serial.begin(9600);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  pinMode(TOGGLE_BUTTON, INPUT);
  pinMode(INC_BUTTON, INPUT);
  pinMode(DEC_BUTTON, INPUT);
  scheduler.addTask(pressedButtonTask);
  scheduler.addTask(checkFlickerYellowTask);
  scheduler.addTask(uiTask);
}

void loop()
{
  scheduler.advanceTick();
}