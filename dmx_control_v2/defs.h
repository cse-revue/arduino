// Time Macro
#define SEC2MILLIS(n)      (n * 1000)

// DMX Library
#define RXEN_PIN             3

// DMX
#define DEFAULT_DMX_ADDR   1
#define DMX_ADDRESSES      512
#define DMX_TIMEOUT        SEC2MILLIS(1)
#define STORE_DMX          0

// Menu
#define DISP_CYCLE_FREQ  SEC2MILLIS(2)
#define N_MENU_OPTIONS   4
#define NO_MENU          255
#define MENU_INFO        0
#define MENU_ADDR        1
#define MENU_OUTS        2
#define MENU_RESET       3
#define MENU_IN_RESET    100

#define DEFAULT_LED_OUTS   2
#define MAX_LED_OUTS       6
#define DMX_SLAVE_CHANNELS   6

#define DEFAULT_PIN_START  9
const int pin_channel_offeset[] = {0,1,2,-6,-4,-3};

// LCD I2C addr
#define LCD_I2C_ADDR  0x20
