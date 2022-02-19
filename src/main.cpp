#include <Arduino.h>
#include "config.h"
#include "button_handler.h"
#include "hx711_reader.h"
#include "motor_controller.h"
#include "display_handler.h"
#include "serial_handler.h"

TaskHandle_t Task1;

void Task1code(void *pvParameters)
{
  button_handler_setup();
  display_handler_setup();
  serial_handler_setup();
  for (;;)
  {
    display_handler_loop();
    button_handler_loop();
    serial_handler_loop();
  }
}

void setup()
{
  hx_reader_setup();
  motor_controller_setup();

  // create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task1code, /* Task function. */
      "Task1",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task1,    /* Task handle to keep track of created task */
      0);        /* pin task to core 0 */
  delay(500);
}

void loop()
{
  hx_reader_loop();
  motor_controller_loop();
}