#include <Arduino.h>

const int BUTTON_PIN = 7;
const int LED_PIN = 3;

int ledState = LOW;
int lastButtonState;
int currentButtonState;

void setup()
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  currentButtonState = digitalRead(BUTTON_PIN);
}

void loop()
{
  lastButtonState = currentButtonState;
  currentButtonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && currentButtonState == LOW)
  {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}
