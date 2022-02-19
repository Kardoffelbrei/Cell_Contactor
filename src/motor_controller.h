#ifndef _MOTOR_CONTROLLER_H_
#define _MOTOR_CONTROLLER_H_


enum motor_states {
  IDLE,
  HOMING,
  HOME,
  PROBING_A, //Probe Cell swiftly
  PROBING_B, //Backoff
  PROBING_C, //Slowly probe cell
  CONTROLLING,
  LIMIT,
  JOG
};

void motor_controller_setup();
void motor_controller_loop();
double get_position();
double get_motor_speed();
double get_target_force();
void set_target_force(double force);
motor_states get_motor_state();
void set_motor_state(motor_states motor_set);
void set_motor_speed(int speed);
double get_contact_point();
void limit_trigger();

#endif