#include <LiquidCrystal_I2C.h>
#include <util/atomic.h>

// https://forum.arduino.cc/t/printf-on-arduino/888528/3
FILE f_out;

int sput(char c, __attribute__((unused)) FILE *f)
{
    if (c == '\n')
    {
        return !Serial.write("\r\n");
    }
    return !Serial.write(c);
}

void redirect_stdout()
{
    //  https://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html#gaf41f158c022cbb6203ccd87d27301226
    fdev_setup_stream(&f_out, sput, nullptr, _FDEV_SETUP_WRITE);
    stdout = &f_out;
}

#define TEMP_SETPOINT_PIN A1
#define LM_20_PIN A2

#define TEMP_OUT_PIN 11

// Encoder pins
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3

// Motor control pins
#define MOTOR_PWM_PIN 5
#define MOTOR_DIRECTION_PIN_1 7
#define MOTOR_DIRECTION_PIN_2 6

// Potentiometer input pin for setting target speed
#define SPEED_POTENTIOMETER_PIN A0

// Encoder counts per revolution (PPR)
const int ENCODER_PPR = 416;

// PID variables
long previousTimeMicros = 0;
int previousPosition = 0;
double lastError = 0, rateError = 0;
volatile int position = 0; // This variable is changed within the interrupt service routine

// PID constants
const float kp = 1.0;  // Proportional gain
const float ki = 0.01; // Integral gain
const float kd = 0.1;  // Derivative gain

// ADC - analog to digital converter
#define ADC_MIN 0
#define ADC_MAX 1023
#define ADC_V_MIN 0    // mV
#define ADC_V_MAX 5000 // mV

#define POTENT_TEMP_MIN (-40)
#define POTENT_TEMP_MAX (125)

#define LM20_TMIN (-30)
#define LM20_V_TMIN 206
#define LM20_TMAX (125)
#define LM20_V_TMAX 1745

#define TEMP_SETPOINT_MIN 15
#define TEMP_SETPOINT_MAX 30

#define TEMP_DELTA 1

int temp_setpoint = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

long currentLcdMillis = 0;
long lastLcdDisplayTime = 0;
long lcdDisplayInterval = 500;

bool get_temperature_control(int temperature)
{
    if (temperature < temp_setpoint - TEMP_DELTA)
    {
        return HIGH;
    }
    else if (temperature > temp_setpoint + TEMP_DELTA)
    {
        return LOW;
    }
    return LOW;
}

// Set motor speed and direction
void setMotor(int motorDirection, int pwmValue)
{
    analogWrite(MOTOR_PWM_PIN, pwmValue);

    if (motorDirection == 1)
    {
        digitalWrite(MOTOR_DIRECTION_PIN_1, HIGH);
        digitalWrite(MOTOR_DIRECTION_PIN_2, LOW);
    }
    else
    {
        digitalWrite(MOTOR_DIRECTION_PIN_1, LOW);
        digitalWrite(MOTOR_DIRECTION_PIN_2, HIGH);
    }
}

// Encoder reading
void readEncoder()
{
    static int lastA = LOW;
    int a = digitalRead(ENCODER_PIN_A);
    int b = digitalRead(ENCODER_PIN_B);

    if (a != lastA)
    {
        if (a == HIGH)
        {
            if (b == LOW)
            {
                position--;
            }
            else
            {
                position++;
            }
        }
        else
        {
            if (b == LOW)
            {
                position++;
            }
            else
            {
                position--;
            }
        }
    }
    lastA = a;
}

void setup()
{
    lcd.init();
    lcd.backlight();

    pinMode(ENCODER_PIN_A, INPUT);
    pinMode(ENCODER_PIN_B, INPUT);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), readEncoder, CHANGE);

    pinMode(TEMP_OUT_PIN, OUTPUT);
    pinMode(MOTOR_PWM_PIN, OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN_1, OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN_2, OUTPUT);
    redirect_stdout();
    Serial.begin(9600);
}

void loop()
{
    int currentPosition = 0;

    // Ensure position is read atomically to prevent errors during interrupt handling
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        currentPosition = position;
    }

    // Calculate velocity
    long currentTimeMicros = micros();
    float deltaTimeSec = (currentTimeMicros - previousTimeMicros) / 1000000.0;
    previousTimeMicros = currentTimeMicros;
    int positionChange = currentPosition - previousPosition;
    previousPosition = currentPosition;
    float velocityCountsPerSec = positionChange / deltaTimeSec;

    // Convert counts per second to RPM
    // We multiply by 60 to convert seconds to minutes and divide by ENCODER_PPR to get revolutions
    float currentRPM = velocityCountsPerSec * 60.0 / ENCODER_PPR;

    // Read the potentiometer and set target RPM
    float targetRPM = map(analogRead(SPEED_POTENTIOMETER_PIN), 0, 1023, 0, 250); // Map to desired RPM range

    // PID calculation
    float error = targetRPM - currentRPM;
    static float integralError = 0;
    integralError += error * deltaTimeSec;
    rateError = (error - lastError) / deltaTimeSec;
    float pidOutput = kp * error + ki * integralError + kd * rateError;
    lastError = error;

    // Constrain PID output to allowable PWM range and set motor direction
    float controlSignal = constrain(pidOutput, -255, 255);
    int motorDirection = controlSignal >= 0 ? 1 : -1;

    // Set motor speed and direction
    setMotor(motorDirection, abs(controlSignal));

    // ON OFF control
    // read setpoint
    int potentiometerValue = analogRead(TEMP_SETPOINT_PIN);

    // Map the potentiometer value to the setpoint range
    int newSetpoint = map(potentiometerValue, ADC_MIN, ADC_MAX, POTENT_TEMP_MAX, POTENT_TEMP_MIN);

    temp_setpoint = constrain(newSetpoint, POTENT_TEMP_MIN, POTENT_TEMP_MAX);

    // Get temperature RAW
    int lm20_analogue = analogRead(LM_20_PIN);

    // Convert raw adc to voltage
    int lm20_voltage = map(lm20_analogue, ADC_MIN, ADC_MAX, ADC_V_MIN, ADC_V_MAX);

    // Convert voltage to temperature
    int lm20_temperature = map(lm20_voltage, LM20_V_TMIN, LM20_V_TMAX, LM20_TMIN, LM20_TMAX);

    // ON OFF HIST
    bool temperature_control = get_temperature_control(lm20_temperature);
    digitalWrite(TEMP_OUT_PIN, temperature_control);

    currentLcdMillis = millis();
    if (currentLcdMillis - lastLcdDisplayTime >= lcdDisplayInterval)
    {
        lastLcdDisplayTime = currentLcdMillis;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(lm20_temperature);
        lcd.print(" SP: ");
        lcd.print(temp_setpoint);

        lcd.setCursor(0, 1);
        lcd.print("Motor SP: ");
        lcd.print((int)targetRPM);
    }
}
