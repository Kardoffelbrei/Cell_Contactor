#ifndef _BUTTON_H_
#define _BUTTON_H_

enum btn_states
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};

void btn_stop_ISR();
void btn_down_ISR();
void btn_up_ISR();
void btn_left_ISR();
void btn_right_ISR();
void button_handler_setup();
void button_handler_loop();
btn_states get_btn();
void set_btn(btn_states btn_set);

#endif