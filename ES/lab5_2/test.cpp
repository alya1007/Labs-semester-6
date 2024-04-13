#include <LiquidCrystal_I2C.h>

#define LCD_ADDRESS 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

#define pinEncoder 3
#define pinMotor1 6
#define pinMotor2 7
#define pot A0
#define PWM 5

float setPoint;
float processVariable;
float cv, cv1, error, error1, error2;

float Kp = 1;
float Ki = 1;
float Kd = 0.01;
float samplingTime = 0.1;

volatile unsigned long counter = 0;
unsigned long previousMillis = 0;
unsigned long interval = 100;

bool change = false;

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

void setup()
{

    pinMode(PWM, OUTPUT);
    pinMode(pinMotor1, OUTPUT);
    pinMode(pinMotor2, OUTPUT);
    pinMode(pot, INPUT);

    Serial.begin(57600);
    attachInterrupt(1, interruption, RISING); // Rising edge of pin 3;
    pinMode(pinEncoder, INPUT_PULLUP);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("SetPoint:");
    lcd.setCursor(0, 1);
    lcd.print("C.Value:");
}

void loop()
{

    unsigned long currentMillis = millis();

    if ((currentMillis - previousMillis) >= interval)
    {
        previousMillis = currentMillis; // Simulates a delay
        detachInterrupt(1);
        /* Considering 11 pulses per revolution and a reduction ratio
        of 1/34, we have the following speed relation:
        V(rpm) = Frequency (pulses/second) * (1 (revolution)/ Pulses * Reduction ratio)* (60 s / 1 min)
        V = F * (1/374) * (60) rpm
        When using specific motors, look for the number of pulses and the reduction ratio
        of the motor used.
        */
        processVariable = 10 * counter * (60.0 / 534.14);

        // cv = 0;
        attachInterrupt(1, interruption, RISING);
        counter = 0;
    }

    setPoint = map(analogRead(pot), 0, 1023, -350, 350);
    // setPoint = analogRead(pot) * (350/1023.0); // Ranges from 0 to 350 rpm;

    // cv = 0;

    if (setPoint >= 0)
    {
        digitalWrite(pinMotor1, LOW);
        digitalWrite(pinMotor2, HIGH);
        error = setPoint - processVariable;
        lcd.setCursor(10, 0);
        lcd.print("    ");
        lcd.setCursor(10, 0);
        lcd.print(setPoint);

        lcd.setCursor(9, 1);
        lcd.print("    ");
        lcd.setCursor(9, 1);
        lcd.print(processVariable);
        Serial.print("Set point: ");
        Serial.print(setPoint);
        Serial.print(" Process Variable: ");
        Serial.println(processVariable);
    }
    else
    {
        digitalWrite(pinMotor1, HIGH);
        digitalWrite(pinMotor2, LOW);
        error = (-setPoint) - processVariable;
        lcd.setCursor(10, 0);
        lcd.print("    ");
        lcd.setCursor(10, 0);
        lcd.print(setPoint);
        lcd.setCursor(9, 1);
        lcd.print("    ");
        lcd.setCursor(9, 1);
        lcd.print(processVariable);
        Serial.print("Set point: ");
        Serial.print(setPoint);
        Serial.print(" Process Variable: ");
        Serial.println((-1) * processVariable);
    }

    // error = setPoint - processVariable;

    // -----ODE----
    cv = cv1 + (Kp + Kd / samplingTime) * error + (-Kp + Ki * samplingTime - 2 * Kd / samplingTime) * error1 + (Kd / samplingTime) * error2;
    cv1 = cv;
    error2 = error1;
    error1 = error;

    // ----SATURATION---
    if (cv >= 350.0)
    {
        cv = 350.0;
    }
    if (cv >= 0.0 && cv < 5.0)
    {
        cv = 5.0;
    }
    if (cv <= -350.0)
    {
        cv = -350.0;
    }
    if (cv < 0.0 && cv > -5.0)
    {
        cv = -5.0;
    }

    analogWrite(PWM, cv * (255.0 / 350.0)); // Ranges from 0 to 255;

    delay(100);
}

void interruption()
{
    counter++;
}