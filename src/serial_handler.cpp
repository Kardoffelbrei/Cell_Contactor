#include <Arduino.h>
#include "config.h"
#include "button_handler.h"
#include "hx711_reader.h"
#include "motor_controller.h"
#include "display_handler.h"
#include "serial_handler.h"

volatile long last_logging_message = 0;

void serial_handler_setup()
{
    Serial.begin(115200);
    Serial.setTimeout(50);
}

void serial_handler_loop()
{
    // if controller is running and a valid setpoint is received on the serial line, use it as the new setpoint
    if (Serial.available() > 0)
    {
        // TODO:
        // h home
        // p point
        double request = Serial.parseFloat();
        if (request <= 10 && request >= 0.1 && get_motor_state() == CONTROLLING)
        {
            set_target_force(request);
        }
        else if (request == -1)
        {
            set_motor_state(HOMING);
        }
        else if (request == -2 && get_motor_state() == HOME)
        {
            set_motor_state(PROBING_A);
        }
    }

    // every 50 ms send all the data via the serial line.
    if (millis() - last_logging_message > 50)
    {

        double pos = get_position();
        double cp = get_contact_point();
        double spring = pos - cp;

        Serial.print(get_force(), 4);
        Serial.print(",");
        Serial.print(get_filtered_force(), 4);
        Serial.print(",");
        Serial.print(get_target_force(), 4);
        Serial.print(",");
        Serial.print(get_motor_speed(), 4);
        Serial.print(",");
        Serial.print(pos, 4);
        Serial.print(",");
        Serial.print(cp, 4);
        Serial.print(",");
        Serial.print(spring, 4);
        Serial.print(",");
        /*
        calculate spring constant k
        if (spring != 0)
        {
            Serial.print(get_force() / spring, 4);
            Serial.print(",");
        }
        */
        Serial.print(get_motor_state());
        Serial.print(",");
        Serial.print(get_screen_state());
        Serial.println();

        last_logging_message = millis();
    }
}