#include <Arduino.h>
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "display_handler.h"

#include "hx711_reader.h"
#include "motor_controller.h"
#include "button_handler.h"

#include "fonts/Lato_Regular_7.h"
#include "fonts/Digital.h"
#include "fonts/Pico.h"
#include <fonts/Org_01_5p.h>

const GFXfont *fontDigital = &Segment13pt7b;
const GFXfont *fontPico = &Segment6pt7b;
const GFXfont *fontDesc = &Dialog_plain_9;
const GFXfont *fontMicro = &Org_01_5p;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 700000, 700000);

unsigned long last_update = 0;

enum screen_pages screen_state = STARTUP;
enum main_menu main_menu_state = MAIN_JOG;
enum sweep_menu sweep_menu_state = SWEEP_START_FORCE;

const unsigned int speeds_list[3] = {107, 1067, 10667}; // 0.1, 1, 10.0 mm/s respectively
unsigned int speeds_selector = 0;

const double sweep_start_force_list[6] = {0.5, 1, 2, 5, 7, 9};
unsigned int sweep_start_force_selector = 1;

const double sweep_end_force_list[5] = {2, 5, 7, 8, 10};
unsigned int sweep_end_force_selector = 4;

const int sweep_step_amount_list[5] = {2, 5, 6, 10, 50};
unsigned int sweep_step_amount_selector = 3;

const double sweep_duration_list[9] = {1, 5, 10, 30, 60, 120, 300, 600, 1200};
unsigned int sweep_duration_selector = 2;

double start_force;
double end_force;
int step_amount;
double duration;

double step_force;
double step_time;

double sweep_force; // this value holds the current force in a sweep campaign
bool sweep_running = false;
motor_states last_motor_state = IDLE;
unsigned long sweep_timer = 0;

String mainMenuTags[5] = {
    "Jog",
    "Point",
    "Sweep",
    "Release",
    "Home"};

String sweepMenuTags[5] = {
    "Start Force: ",
    "End Force: ",
    "Step Amount: ",
    "Duration: ",
    "START"};

void display_handler_setup()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
    {
        Serial.println(F("OLED display allocation failed"));
    }
}

screen_pages get_screen_state()
{
    return screen_state;
}

void printStartupScreen()
{
    display.setFont(fontDesc);
    display.setCursor(35, 25);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println("FKFS");
    display.setTextSize(1);
    display.println("      Startup Phase");
}

void printMainMenu()
{
    display.setFont(fontDesc);
    display.setCursor(0, 10);
    display.setTextSize(1);

    switch (get_btn())
    {
    case UP:
        main_menu_state = static_cast<main_menu> constrain(main_menu_state - 1, 0, 4);
        set_btn(NONE);
        break;
    case DOWN:
        main_menu_state = static_cast<main_menu> constrain(main_menu_state + 1, 0, 4);
        set_btn(NONE);
        break;
    case RIGHT:
        switch (main_menu_state)
        {
        case MAIN_JOG:
            set_motor_state(JOG);
            set_btn(NONE);
            screen_state = MANUAL_MENU;
            break;
        case MAIN_POINT:
            screen_state = POINT_MENU;
            set_btn(NONE);
            break;
        case MAIN_SWEEP:
            screen_state = SWEEP_SETUP_MENU;
            set_btn(NONE);
            break;
        case MAIN_RELEASE:
            break;
        case MAIN_HOME:
            set_motor_state(HOMING);
            set_btn(NONE);
            break;
        }
        break;
    }

    for (unsigned int i = 0; i < 5; i++)
    {
        if (main_menu_state == i)
        {
            display.print("> ");
        }
        else
        {
            display.print("   ");
        }
        display.println(mainMenuTags[i]);
    }
}

void printManualMenu()
{
    display.setFont(fontDesc);
    display.setCursor(0, 10);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("      Manual Mode");
    display.println("--------------------------------");

    display.print("> Speed: ");
    display.print(speeds_list[speeds_selector] * DISTANCE_PER_STEP);
    display.println(" mm/s");

    display.print("   Position: ");
    display.print(get_position());
    display.println(" mm");

    display.print("   Force: ");
    display.print(get_filtered_force());
    display.println(" N");

    set_motor_speed(speeds_list[speeds_selector]);

    switch (get_btn())
    {
    case LEFT:
        set_motor_state(IDLE);
        set_btn(NONE);
        speeds_selector = 0;
        screen_state = MAIN_MENU;
        break;
    case RIGHT:
        speeds_selector += 1;
        if (speeds_selector > 2)
        {
            speeds_selector = 0;
        }
        set_btn(NONE);
        break;
    }
}

void printPointMenu()
{
    display.setFont(fontDesc);
    display.setCursor(0, 10);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    if (get_motor_state() == HOME)
    {
        display.println("  Point Mode - HOME");
    }
    else if (get_motor_state() == PROBING_A || get_motor_state() == PROBING_B || get_motor_state() == PROBING_C)
    {
        display.println("  Point Mode - PROB");
    }
    else if (get_motor_state() == CONTROLLING)
    {
        display.println("  Point Mode - CONT");
    }
    else
    {
        display.println("  Point Mode - ?");
        set_motor_state(HOMING);
    }

    display.println("--------------------------------");

    display.print("Target Force: ");
    display.print(get_target_force());
    display.println(" N");

    display.print("Actual Force: ");
    display.print(get_filtered_force());
    display.println(" N");

    double pos = get_position();
    double cp = get_contact_point();

    /*
        display.print("Position: ");
        display.print(pos);
        display.println(" mm");

        display.print("CP: ");
        display.print(cp);
        display.println(" mm");
    */

    if (cp != 0)
    {
        display.print("Spring: ");
        display.print(pos - cp);
        display.println(" mm");
    }

    switch (get_btn())
    {
    case LEFT:
        set_motor_state(IDLE);
        set_btn(NONE);
        screen_state = MAIN_MENU;
        break;
    case RIGHT:
        if (get_motor_state() == HOME)
        {
            set_motor_state(PROBING_A);
        }
        set_btn(NONE);
        break;
    case UP:
        set_target_force(constrain(get_target_force() + 0.1, 0.1, LIMIT_FORCE));
        set_btn(NONE);
        break;
    case DOWN:
        set_target_force(constrain(get_target_force() - 0.1, 0.1, LIMIT_FORCE));
        set_btn(NONE);
        break;
    }
}

void printSweepSetupMenu()
{
    display.setFont(fontDesc);
    display.setCursor(0, 10);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    for (unsigned int i = 0; i < 5; i++)
    {
        if (sweep_menu_state == i)
        {
            display.print("> ");
        }
        else
        {
            display.print("   ");
        }
        display.print(sweepMenuTags[i]);

        switch (i)
        {
        case SWEEP_START_FORCE:
            display.print(sweep_start_force_list[sweep_start_force_selector]);
            display.println(" N");
            break;
        case SWEEP_END_FORCE:
            display.print(sweep_end_force_list[sweep_end_force_selector]);
            display.println(" N");
            break;
        case SWEEP_STEP_AMOUNT:
            display.println(sweep_step_amount_list[sweep_step_amount_selector]);
            break;
        case SWEEP_TIME:
            if (sweep_duration_list[sweep_duration_selector] < 60)
            {
                display.print(sweep_duration_list[sweep_duration_selector]);
                display.println(" m");
            }
            else
            {
                display.print(sweep_duration_list[sweep_duration_selector] / 60);
                display.println(" h");
            }
            break;
        case SWEEP_START:
            break;
        }
    }

    switch (get_btn())
    {
    case LEFT:
        set_motor_state(IDLE);
        set_btn(NONE);
        screen_state = MAIN_MENU;
        break;
    case RIGHT:
        set_btn(NONE);
        switch (sweep_menu_state)
        {
        case SWEEP_START_FORCE:
            sweep_start_force_selector += 1;
            if (sweep_start_force_selector >= sizeof(sweep_start_force_list) / sizeof(sweep_start_force_list[0]))
            {
                sweep_start_force_selector = 0;
            }
            break;
        case SWEEP_END_FORCE:
            sweep_end_force_selector += 1;
            if (sweep_end_force_selector >= sizeof(sweep_end_force_list) / sizeof(sweep_end_force_list[0]))
            {
                sweep_end_force_selector = 0;
            }
            break;
        case SWEEP_STEP_AMOUNT:
            sweep_step_amount_selector += 1;
            if (sweep_step_amount_selector >= sizeof(sweep_step_amount_list) / sizeof(sweep_step_amount_list[0]))
            {
                sweep_step_amount_selector = 0;
            }
            break;
        case SWEEP_TIME:
            sweep_duration_selector += 1;
            if (sweep_duration_selector >= sizeof(sweep_duration_list) / sizeof(sweep_duration_list[0]))
            {
                sweep_duration_selector = 0;
            }
            break;
        case SWEEP_START:
            start_force = sweep_start_force_list[sweep_start_force_selector];
            end_force = sweep_end_force_list[sweep_end_force_selector];
            step_amount = sweep_step_amount_list[sweep_step_amount_selector];
            duration = sweep_duration_list[sweep_duration_selector];

            // only start sweep if end force is bigger than start force and a single step is longer than a minute
            // TODO: tell user which setting is wrong
            if (true || (start_force < end_force) && (duration / step_amount >= 1))
            {
                // calculate needed values before starting
                step_time = duration / step_amount;
                step_force = (end_force - start_force) / (step_amount - 1);
                set_target_force(start_force);
                screen_state = SWEEP_MENU;
            }
            break;
        }
        break;
    case UP:
        sweep_menu_state = static_cast<sweep_menu> constrain(sweep_menu_state - 1, 0, 4);
        set_btn(NONE);
        break;
    case DOWN:
        sweep_menu_state = static_cast<sweep_menu> constrain(sweep_menu_state + 1, 0, 4);
        set_btn(NONE);
        break;
    }
}

void printSweepMenu()
{
    display.setFont(fontDesc);
    display.setCursor(0, 10);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    if (get_motor_state() == HOME)
    {
        display.println("  Sweep Mode - HOME");
    }
    else if (get_motor_state() == PROBING_A || get_motor_state() == PROBING_B || get_motor_state() == PROBING_C)
    {
        display.println("  Sweep Mode - PROB");
    }
    else if (get_motor_state() == CONTROLLING)
    {
        display.println("  Sweep Mode - CONT");
    }
    else
    {
        display.println("  Sweep Mode - ?");
        set_motor_state(HOMING);
    }

    display.println("--------------------------------");

    display.print("Target Force: ");
    display.print(get_target_force());
    display.println(" N");

    display.print("Actual Force: ");
    display.print(get_filtered_force());
    display.println(" N");

    // start sweep as soon as controller is running
    if ((get_motor_state() == CONTROLLING) && (last_motor_state == PROBING_C))
    {
        sweep_running = true;
        sweep_timer = millis();
        sweep_force = start_force;
        set_target_force(sweep_force);
    }
    last_motor_state = get_motor_state();

    unsigned long now = millis();
    if (sweep_running == true)
    {
        double next_step = -(now - sweep_timer - (step_time * 60 * 1000)) / 1000;
        display.print("Next Step in: ");
        if (next_step < 60)
        {
            display.print(next_step);
            display.println(" s");
        }
        else if (next_step < 3600)
        {
            display.print(next_step / 60);
            display.println(" m");
        }
        else
        {
            display.print(next_step / 3600);
            display.println(" h");
        }

        if (now - sweep_timer > (step_time * 60 * 1000))
        {
            sweep_timer = millis();
            step_amount -= 1;

            if (step_amount == 0)
            {
                sweep_running = false;
                screen_state = SWEEP_SETUP_MENU;
                set_motor_state(HOMING);
            }
            else
            {
                sweep_force += step_force;
                set_target_force(sweep_force);
            }
        }
    }

    switch (get_btn())
    {
    case LEFT:
        set_motor_state(IDLE);
        set_btn(NONE);
        sweep_running = false;
        screen_state = SWEEP_SETUP_MENU;
        break;
    case RIGHT:
        if (get_motor_state() == HOME)
        {
            set_motor_state(PROBING_A);
        }
        set_btn(NONE);
        break;
    case UP:
        set_btn(NONE);
        break;
    case DOWN:
        set_btn(NONE);
        break;
    }
}

void updateScreen()
{
    display.clearDisplay();
    display.setTextColor(WHITE);

    switch (screen_state)
    {

    case STARTUP:
        printStartupScreen();
        if (millis() > 5000)
        {
            screen_state = MAIN_MENU;
            set_motor_state(HOMING);
        }
        break;

    case MAIN_MENU:
        printMainMenu();
        break;

    case MANUAL_MENU:
        printManualMenu();
        break;

    case POINT_MENU:
        printPointMenu();
        break;

    case SWEEP_SETUP_MENU:
        printSweepSetupMenu();
        break;

    case SWEEP_MENU:
        printSweepMenu();
        break;

    default:
        break;
    }
    display.display();
}

void display_handler_loop()
{
    if (millis() - last_update > 100)
    {
        last_update = millis();
        updateScreen();
    }
}