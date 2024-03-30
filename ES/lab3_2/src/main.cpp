#include "io.hpp"

#define ntc_pin A0               // Pin,to which the voltage divider is connected
#define nominal_resistance 10000 // Nominal resistance at 25⁰C
#define nominal_temperature 25   // temperature for nominal resistance (almost always 25⁰ C)
#define rate 50                  // Number of samples
#define beta 3950                // The beta coefficient
#define Rref 10000               // Value of  resistor used for the voltage divider
#define numReadings 10
#define BAUD_RATE 9600

int analogReadings[rate];
int readings[numReadings];
int readIndex = 0;
long total = 0;

float adc(int sample)
{
    float sampleFloat = 0;
    sampleFloat = sample;
    sampleFloat = 1023 / sampleFloat - 1;
    sampleFloat = Rref / sampleFloat;

    float temperature;
    temperature = sampleFloat / nominal_resistance;      // (R/Ro)
    temperature = log(temperature);                      // ln(R/Ro)
    temperature /= beta;                                 // 1/B * ln(R/Ro)
    temperature += 1.0 / (nominal_temperature + 273.15); // + (1/To)
    temperature = 1.0 / temperature;                     // Invert
    temperature -= 273.15;                               // convert absolute temp to C

    return temperature;
}

float salt_pepper()
{
    float average = 0;
    int sum = 0;
    for (int i = rate - 10; i < rate; i++)
    {
        sum += analogReadings[i];
    }
    average = sum / 10;
    float converted_average = adc(average);
    return converted_average;
}

long smooth()
{
    total = total - readings[readIndex];       // Subtract the oldest reading
    readings[readIndex] = analogRead(ntc_pin); // Read the newest reading
    total = total + readings[readIndex];       // Add the newest reading to the total
    readIndex = (readIndex + 1) % numReadings; // Increment index and handle wrap-around
    float result = total / numReadings;
    float convertedResult = adc(result);
    return convertedResult;
}

void setup(void)
{
    redirect_stdout();
    Serial.begin(BAUD_RATE);
    printf("Serial communication started\n");
}

void loop(void)
{
    for (int i = 0; i < rate; i++)
    {
        analogReadings[i] = analogRead(ntc_pin);
        delay(5);
    }

    int sample = analogRead(ntc_pin);

    float result = adc(sample);
    printf("Temperature (ADC)");
    printf("%d", (int)result);
    printf(" *C\n");

    float salt_pepper_result = salt_pepper();
    printf("Temperature (Salt and Pepper)");
    printf("%d", (int)salt_pepper_result);
    printf(" *C\n");

    float moving_average_result = smooth();
    printf("Temperature (Moving Average): ");
    printf("%d", (int)moving_average_result);
    printf(" *C\n");

    printf("\n\n");

    delay(2000);
}
