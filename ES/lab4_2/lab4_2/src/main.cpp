#include <Arduino.h>

int relayPin = 3;
String command;

int serial_putchar(char c, FILE *stream)
{
  Serial.write(c);
  return 0;
}

FILE serial_stdout;

void setup()
{
  pinMode(relayPin, OUTPUT);
  Serial.begin(9600);

  fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &serial_stdout;
}

void loop()
{
  char message[100];
  memset(message, 0, 100);
  if (Serial.available() > 0)
  {
    command = Serial.readStringUntil('\n');
  }

  if (command.startsWith("led on"))
  {
    digitalWrite(relayPin, HIGH);
    printf("LED turned on\n");
  }
  else if (command.startsWith("led off"))
  {
    digitalWrite(relayPin, LOW);
    printf("LED turned off\n");
  }
  else
  {
    printf("Invalid command\n");
  }
}
