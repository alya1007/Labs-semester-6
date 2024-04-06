#include <LiquidCrystal_I2C.h>

int relayPin = 3;
String command;
#define enA 9
#define in1 7
#define in2 8
#define lcdAddress 32
const int defaultSpeed = 255;
const int minSpeed = -100;
const int maxSpeed = 100;
const int nullSpeed = 0;
const int lcdDimensions[2] = {16, 2};
const float normalizeFactor = 0.01;
int baudRate = 9600;

LiquidCrystal_I2C lcd(lcdAddress, 16, 2);

int serial_putchar(char c, FILE *stream)
{
    Serial.write(c);
    return 0;
}

FILE serial_stdout;

class LCDController
{
public:
    void init()
    {
        lcd.begin(lcdDimensions[0], lcdDimensions[1]);
        lcd.init();
        lcd.backlight();
        displayInitialMessage();
    }

    void displayInitialMessage()
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter the speed");
    }

    void displaySpeed(int speed)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Speed set to: ");
        lcd.setCursor(0, 1);
        lcd.print(speed);
        lcd.print("%");
        delay(300);
        displayInitialMessage();
    }

    void displayErrorMessage()
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Out of range!");
        lcd.setCursor(0, 1);
        lcd.print("Range:-100...100");
        delay(300);
        displayInitialMessage();
    }
};

class L293DController
{
public:
    void init()
    {
        pinMode(enA, OUTPUT);
        pinMode(in1, OUTPUT);
        pinMode(in2, OUTPUT);
    }

    void setCounterClockwiseSpeed(int speed)
    {
        analogWrite(enA, (speed * normalizeFactor) * defaultSpeed);
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
    }

    void setClockwiseSpeed(int speed)
    {
        analogWrite(enA, (abs(speed) * normalizeFactor) * defaultSpeed);
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
    }
};

class MotorController
{
public:
    L293DController driverController;

    void init()
    {
        driverController.init();
    }

    void changeSpeed(int speed)
    {
        if (speed >= 0)
        {
            driverController.setCounterClockwiseSpeed(speed);
        }
        else if (speed < 0)
        {
            driverController.setClockwiseSpeed(speed);
        }
    }
};

MotorController motorController;
LCDController lcdController;

void setup()
{
    pinMode(relayPin, OUTPUT);
    motorController.init();
    Serial.begin(baudRate);
    lcdController.init();

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
        int speed = command.toInt();
        if (speed >= nullSpeed && speed <= maxSpeed)
        {
            motorController.changeSpeed(speed);
            lcdController.displaySpeed(speed);
        }
        else if (speed >= minSpeed && speed <= nullSpeed)
        {
            motorController.changeSpeed(speed);
            lcdController.displaySpeed(speed);
        }
        else
        {
            lcdController.displayErrorMessage();
        }
    }
}