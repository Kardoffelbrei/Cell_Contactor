#ifndef _DISPLAY_H_
#define _DISPLAY_H_

enum screen_pages
{
    STARTUP,
    MAIN_MENU,
    MANUAL_MENU,
    POINT_MENU,
    SWEEP_SETUP_MENU,
    SWEEP_MENU
};

enum main_menu
{
    MAIN_JOG,
    MAIN_POINT,
    MAIN_SWEEP,
    MAIN_RELEASE,
    MAIN_HOME
};

enum sweep_menu
{
    SWEEP_START_FORCE,
    SWEEP_END_FORCE,
    SWEEP_STEP_AMOUNT,
    SWEEP_TIME,
    SWEEP_START
};

void display_handler_setup();
void display_handler_loop();
screen_pages get_screen_state();

#endif