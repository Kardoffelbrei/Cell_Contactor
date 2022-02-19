#include <Arduino.h>
#include "button_handler.h"
#include "config.h"
#include "motor_controller.h"

int buttons[] = {BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT};
int buttonState[4];
int lastButtonState[4];
long lastDebounceTime = 0;

enum btn_states btn = NONE;

void btn_stop_ISR()
{
  limit_trigger();
}

void button_handler_setup()
{
  pinMode(BTN_STOP, INPUT);
  attachInterrupt(digitalPinToInterrupt(BTN_STOP), btn_stop_ISR, RISING);
 
  for (int i = 0; i < 4; i++)
  {
    pinMode(buttons[i], INPUT_PULLUP);
    lastButtonState[i] = HIGH;
  }
}

btn_states get_btn()
{
  return btn;
}

void set_btn(btn_states btn_set)
{
  btn = btn_set;
}

void button_handler_loop()
{
  for (int currentButton = 0; currentButton < 4; currentButton++)
  {
    int reading = !digitalRead(buttons[currentButton]);

    if (reading != lastButtonState[currentButton])
    {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > BTN_DEBOUNCE_TIME)
    {
      if (reading != buttonState[currentButton])
      {
        buttonState[currentButton] = reading;
        if (buttonState[currentButton] == HIGH)
        {
          btn = static_cast<btn_states>(currentButton);
        }
      }
    }
    lastButtonState[currentButton] = reading;
  }
}