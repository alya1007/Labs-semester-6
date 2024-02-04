#include <Arduino.h>
#include <stdio.h>

#define LED_PIN 13
#define BAUD_RATE 9600
#define MESSAGE_LENGTH 64

#define LED_ON_MESSAGE "led on"
#define LED_ON_PROMPT "LED IS ON"
#define LED_OFF_MESSAGE "led off"
#define LED_OFF_PROMPT "LED IS OFF"
#define ERROR_PROMPT "Error command."
#define INSTRUCTIONS_PROMPT "Type 'led on' to turn on the LED and 'led off' to turn it off.\n"

int putChar(char c, FILE *fp)
{
  if (c == '\n')
  {
    Serial.write('\n');
    Serial.write('\r');
    return 0;
  }
  return !Serial.write(c);
}

void setup()
{
  Serial.begin(BAUD_RATE);
  pinMode(LED_PIN, OUTPUT);
  fdevopen(putChar, nullptr);
}

void loop()
{
  char message[MESSAGE_LENGTH];
  memset(message, 0, sizeof(message));

  if (Serial.available() > 0)
  {

    Serial.readBytesUntil('\n', message, sizeof(message));

    if (strcmp(message, LED_ON_MESSAGE) == 0)
    {
      digitalWrite(LED_PIN, HIGH);
      printf("%s -> %s\n", message, LED_ON_PROMPT);
    }
    else if (strcmp(message, LED_OFF_MESSAGE) == 0)
    {
      digitalWrite(LED_PIN, LOW);
      printf("%s -> %s\n", message, LED_OFF_PROMPT);
    }
    else
    {
      printf("%s -> %s ", message, ERROR_PROMPT);
      printf("%s", INSTRUCTIONS_PROMPT);
    }
  }
}
