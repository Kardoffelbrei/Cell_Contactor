// Pin definitions
#define HX_SCK 13
#define HX_DATA 26

#define DRV_STEP 33
#define DRV_DIR 32

#define BTN_STOP 2 // connected to builtin LED

#define BTN_DOWN 16
#define BTN_UP 23
#define BTN_LEFT 4
#define BTN_RIGHT 19

#define ANALOG_FORCE_OUTPUT 25

// Button definition
#define BTN_DEBOUNCE_TIME 50

// Address definition
#define OLED_ADDR 0x3C

// HX definition
#define HX_CALIBRATION -228.02
#define CONTACT_FORCE 0.42
#define LIMIT_FORCE 10

// Motor definitions
#define MAX_SPEED 12000
#define MAX_ACCEL 1500
#define MAX_POS 85
#define HOME_SPEED 6000

#define DISTANCE_PER_STEP 0.0009375

// Probe definitions
#define PROBE_SPEED_FAST 6000
#define PROBE_SPEED_SLOW 100
#define PROBE_BACKOFF 1.5

// PID, Filter and Deadband
#define K_P 69
#define K_I 0
#define K_D 0
#define FILTERCOUNT 50 // how many past measurements for the moving average
#define DEADBAND 0.02 // in Newton