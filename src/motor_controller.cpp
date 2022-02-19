#include <Arduino.h>
#include <AccelStepper.h>
#include "config.h"
#include "motor_controller.h"
#include "button_handler.h"
#include "hx711_reader.h"
#include <PID_v1.h>

AccelStepper stepper(AccelStepper::DRIVER, DRV_STEP, DRV_DIR);

enum motor_states state = IDLE;

int manual_speed = 0;
double contact_point = 0;

double Setpoint = 6.9;

double Input, Output;
PID controller(&Input, &Output, &Setpoint, K_P, K_I, K_D, DIRECT);

void motor_controller_setup()
{
    stepper.setMaxSpeed(MAX_SPEED);
    stepper.setAcceleration(MAX_ACCEL);
    stepper.setPinsInverted(true);
}

motor_states get_motor_state()
{
    return state;
}

void set_motor_state(motor_states motor_set)
{
    state = motor_set;
}

void set_motor_speed(int speed)
{
    manual_speed = speed;
}

double get_position()
{
    return stepper.currentPosition() * DISTANCE_PER_STEP;
}

double get_motor_speed()
{
    return stepper.speed();
}

double get_target_force()
{
    return Setpoint;
}

void set_target_force(double force)
{
    Setpoint = force;
}

double get_contact_point()
{
    return contact_point;
}

void limit_trigger()
{
    if (state == HOMING)
    {
        stepper.setCurrentPosition(0);
        contact_point = 0;
        state = HOME;
        hx_tara();
    }
}

void motor_controller_loop()
{

    if (get_force() > LIMIT_FORCE || get_position() > MAX_POS)
    {
        // Serial.println("Limit exceeded");
    }

    switch (state)
    {
    case IDLE:
        stepper.moveTo(stepper.currentPosition());
        stepper.runSpeed();
        break;

    case HOMING:
        // check if already home
        if (digitalRead(BTN_STOP))
        {
            stepper.setCurrentPosition(0);
            contact_point = 0;
            state = HOME;
            hx_tara();
        }
        stepper.setSpeed(-HOME_SPEED);
        stepper.runSpeed();
        break;

    case HOME:
        stepper.setSpeed(0);
        stepper.runSpeed();
        break;

    case PROBING_A:
        stepper.setSpeed(PROBE_SPEED_FAST);
        stepper.runSpeed();
        if (contact_detected())
        {
            state = PROBING_B;
            stepper.move(-PROBE_BACKOFF * (1 / DISTANCE_PER_STEP));
        }
        break;

    case PROBING_B:
        if (!stepper.run())
        {
            state = PROBING_C;
        }
        break;

    case PROBING_C:
        stepper.setSpeed(PROBE_SPEED_SLOW);
        stepper.runSpeed();
        if (contact_detected())
        {
            contact_point = stepper.currentPosition() * DISTANCE_PER_STEP;
            stepper.setSpeed(0);
            stepper.runSpeed();
            controller.SetOutputLimits(-1000, 1000);
            controller.SetMode(AUTOMATIC);
            state = CONTROLLING;
        }
        break;

    case CONTROLLING:
        Input = get_filtered_force();
        // Input = get_force();
        if (Input < Setpoint - 0.80 || Input > Setpoint + 0.80)
        {
            if (Input < Setpoint)
            {
                stepper.setSpeed(420);
            }
            else
            {
                stepper.setSpeed(-420);
            }
        }
        else if (Input < Setpoint - DEADBAND || Input > Setpoint + DEADBAND)
        {
            controller.Compute();
            stepper.setSpeed(Output);
        }
        else
        {
            stepper.setSpeed(0);
        }

        stepper.runSpeed();
        break;

    case LIMIT:
        stepper.setSpeed(0);
        stepper.runSpeed();
        break;

    case JOG:
        if (!digitalRead(BTN_UP) && stepper.currentPosition() > 0)
        {
            stepper.setSpeed(-manual_speed);
        }
        else if (!digitalRead(BTN_DOWN) && get_position() < MAX_POS)
        {
            stepper.setSpeed(manual_speed);
        }
        else
        {
            stepper.setSpeed(0);
        }
        stepper.run();
        break;
    }
}