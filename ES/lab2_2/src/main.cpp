#include <Arduino.h>
#include <Adafruit_LiquidCrystal.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

const int LED_BUTTON = 11;
const int DECREASE_BUTTON = 10;
const int INCREASE_BUTTON = 9;
const int RED_LED = 13;
const int YELLOW_LED = 12;
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const int LCD_COLUMNS = 16;
const int LCD_ROWS = 2;
Adafruit_LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

bool button_pressed = false;
bool flicker_led_on = false;
bool decrease_button_pressed = false;
bool increase_button_pressed = false;
bool should_ticks_show = false;
uint32_t last_flicker = 0;
uint32_t interval = 1000;
uint32_t last_button_press = 0;

auto any_button_pressed = xSemaphoreCreateBinary();
auto update_ui = xSemaphoreCreateBinary();

void init_lcd()
{
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  lcd.clear();
}

// Task to monitor the LED button
void button_task(void *pvParameters)
{
  for (;;)
  {
    // Read the state of the LED button
    button_pressed = digitalRead(LED_BUTTON);

    // If the button is pressed, signal the button press
    if (button_pressed)
    {
      last_button_press = millis();
      xSemaphoreGive(any_button_pressed); // Signal button press
      xSemaphoreGive(update_ui);          // Signal UI update
    }

    // Delay before checking the button state again
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Task to control LED flickering and handle button presses
void flicker_task(void *pvParameters)
{
  for (;;)
  {
    // Control LED flickering when the LED button is not pressed
    if (!button_pressed)
    {
      if (millis() - last_flicker > interval)
      {
        flicker_led_on = !flicker_led_on;
        last_flicker = millis();
      }
    }
    else
    {
      flicker_led_on = false;
    }

    // Check for decrease button press
    decrease_button_pressed = digitalRead(DECREASE_BUTTON);
    if (digitalRead(DECREASE_BUTTON))
    {
      interval = max(interval - 100, 200); // Adjust interval with a lower bound
      should_ticks_show = true;
      last_button_press = millis();
      xSemaphoreGive(any_button_pressed); // Signal button press
      xSemaphoreGive(update_ui);          // Signal UI update
      vTaskDelay(pdMS_TO_TICKS(200));     // Debouncing delay
    }

    // Check for increase button press
    increase_button_pressed = digitalRead(INCREASE_BUTTON);
    if (digitalRead(INCREASE_BUTTON))
    {
      interval += 100; // Adjust interval
      should_ticks_show = true;
      last_button_press = millis();
      xSemaphoreGive(any_button_pressed); // Signal button press
      xSemaphoreGive(update_ui);          // Signal UI update
      vTaskDelay(pdMS_TO_TICKS(200));     // Debouncing delay
    }

    // Delay before the next iteration
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Task to handle LCD display and user interface
void idle_task(void *pvParameters)
{
  vTaskDelay(pdMS_TO_TICKS(500)); // Delay for LCD initialization
  for (;;)
  {
    // Handle button press indication on LCD
    if (xSemaphoreTake(any_button_pressed, pdMS_TO_TICKS(200)))
    {
      if (button_pressed)
      {
        lcd.setCursor(0, 0);
        lcd.print("LED button pressed");
      }
    }

    // Update UI elements on LCD
    if (xSemaphoreTake(update_ui, pdMS_TO_TICKS(200)))
    {
      if (should_ticks_show)
      {
        lcd.setCursor(0, 0);
        lcd.print("Interval: ");
        lcd.print(interval);
        lcd.print("ms");
        should_ticks_show = false;
      }
    }

    // Delay before clearing the LCD and repeating the loop
    vTaskDelay(pdMS_TO_TICKS(200));
    lcd.clear();
  }
}

// Task to control LED states
void leds_control_task(void *pvParameters)
{
  for (;;)
  {
    // Control the state of the LEDs
    digitalWrite(YELLOW_LED, button_pressed);
    digitalWrite(RED_LED, flicker_led_on);
    vTaskDelay(pdMS_TO_TICKS(10)); // Delay before the next iteration
  }
}

void setup()
{
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  init_lcd();

  xTaskCreate(button_task, "Button Task", 64, NULL, 1, NULL);
  xTaskCreate(flicker_task, "Flicker Task", 128, NULL, 1, NULL);
  xTaskCreate(idle_task, "UI Task", 256, NULL, 1, NULL);
  xTaskCreate(leds_control_task, "UI LEDs Task", 64, NULL, 1, NULL);

  vTaskStartScheduler();
}

void loop()
{
}
