#include <LiquidCrystal_I2C.h>

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

#define TEMP_SETPOINT_PIN A0
#define LED_SETPOINT_PIN A1
#define LM_20_PIN A2

#define LED_OUT_PIN 11
#define TEMP_OUT_PIN 9

// ADC - analog to digital converter
#define ADC_MIN 0
#define ADC_MAX 1023   // highest possible analog input voltage
#define ADC_V_MIN 0    // (mV) minimum voltage level that the ADC input can measure
#define ADC_V_MAX 5000 // (mV) maximum voltage level that the ADC input can measure

#define LED_SETPOINT_MIN 0
#define LED_SETPOINT_MAX 100

// PWM - pulse width modulation
#define PWM_MIN 0
#define PWM_MAX 255 // maximum duty cycle value for the PWM signal

#define POTENT_TEMP_MIN (-40)
#define POTENT_TEMP_MAX (125)

// Temperature sensor
#define LM20_TMIN (-30)
#define LM20_V_TMIN 206
#define LM20_TMAX (125)
#define LM20_V_TMAX 1745

#define TEMP_SETPOINT_MIN 15
#define TEMP_SETPOINT_MAX 30

#define TEMP_DELTA 1

int led_setpoint_analogue = 0;
int outputValue = 0;

int temp_setpoint = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

long current_led_millis = 0;
long lastLedDisplayTime = 0;
long ledDisplayInterval = 1000;

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

void setup()
{
    lcd.init();
    lcd.backlight();

    pinMode(LED_SETPOINT_PIN, INPUT);
    pinMode(LED_OUT_PIN, OUTPUT);
    pinMode(TEMP_OUT_PIN, OUTPUT);
    redirect_stdout();
    Serial.begin(9600);
}

void loop()
{
    // read setpoint
    led_setpoint_analogue = analogRead(LED_SETPOINT_PIN);

    // convert to level 0...100%
    int led_brightness = map(led_setpoint_analogue, ADC_MIN, ADC_MAX, LED_SETPOINT_MIN, LED_SETPOINT_MAX);

    // led intensity
    int led_out = map(led_brightness, LED_SETPOINT_MIN, LED_SETPOINT_MAX, PWM_MIN, PWM_MAX);

    // apply the brightness to the LED
    analogWrite(LED_OUT_PIN, led_out);

    // print the results to the serial monitor:
    current_led_millis = millis();
    if (current_led_millis - lastLedDisplayTime >= ledDisplayInterval)
    {
        lastLedDisplayTime = current_led_millis;
        printf("LED Brightness: %d\n", led_brightness);
    }

    // ON OFF control
    // read setpoint
    int potentiometerValue = analogRead(TEMP_SETPOINT_PIN);

    // Map the potentiometer value to the setpoint range
    int newSetpoint = map(potentiometerValue, ADC_MIN, ADC_MAX, POTENT_TEMP_MAX, POTENT_TEMP_MIN);

    // Limit the value of newSetpoint within POTENT_TEMP_MIN and POTENT_TEMP_MAX.
    temp_setpoint = constrain(newSetpoint, POTENT_TEMP_MIN, POTENT_TEMP_MAX);

    // Get temperature RAW
    int lm20_analogue = analogRead(LM_20_PIN);

    // Convert raw adc to vold
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
        lcd.print("Setpoint: ");
        lcd.print(temp_setpoint);
        lcd.setCursor(0, 1);
        lcd.print("Temp: ");
        lcd.print(lm20_temperature);
    }
}
