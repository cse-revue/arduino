// CHANGE THIS FOR ARDUINO TYPE
//#define ARDUINO_UNO    1
#define ARDUINO_MEGA   1

// Time Macro
#define SEC2MILLIS(n)      (n * 1000)

// DMX Library
#define RXEN_PIN             3

// DMX
#define DEFAULT_DMX_ADDR   132
#define DMX_ADDRESSES      512
#define DMX_TIMEOUT        SEC2MILLIS(1)
#define STORE_DMX          0

// RGB/HSV
#define DEFAULT_RGB_MODE  true
#define INDEX_R           0
#define INDEX_G           1
#define INDEX_B           2
#define MAX_LED_OUTS      4

// Menu
#define DISP_CYCLE_FREQ  SEC2MILLIS(2)
#define N_MENU_OPTIONS   5
#define NO_MENU          255
#define MENU_INFO        0
#define MENU_ADDR        1
#define MENU_MRGB        2
#define MENU_OUTS        3
#define MENU_RESET       4
#define MENU_IN_RESET    100

// Button management
#define BUTTON_NONE               0
#define BUTTON_RIGHT              1
#define BUTTON_UP                 2
#define BUTTON_DOWN               3
#define BUTTON_LEFT               4
#define BUTTON_SELECT             5

// Output pin offsets
#ifdef ARDUINO_UNO
  #define DEFAULT_PIN_START  9
  #define DEFAULT_LED_OUTS   2
  #define DMX_SLAVE_CHANNELS   6
  
  #define PIN_R1_OFFSET      0
  #define PIN_G1_OFFSET      1
  #define PIN_B1_OFFSET      2
  #define PIN_R2_OFFSET      -6
  #define PIN_G2_OFFSET      -4
  #define PIN_B2_OFFSET      -3
  #define PIN_R3_OFFSET      -6
  #define PIN_G3_OFFSET      -4
  #define PIN_B3_OFFSET      -3
  #define PIN_R4_OFFSET      -6  // Only need one extra pin really
  #define PIN_G4_OFFSET      -6
  #define PIN_B4_OFFSET      -6
#endif
#ifdef ARDUINO_MEGA
  #define DEFAULT_PIN_START  2
  #define DEFAULT_LED_OUTS   4
  #define DMX_SLAVE_CHANNELS   12
  
  #define PIN_R1_OFFSET      0
  #define PIN_G1_OFFSET      1
  #define PIN_B1_OFFSET      2
  #define PIN_R2_OFFSET      3
  #define PIN_G2_OFFSET      4
  #define PIN_B2_OFFSET      5
  #define PIN_R3_OFFSET      6
  #define PIN_G3_OFFSET      7
  #define PIN_B3_OFFSET      8
  #define PIN_R4_OFFSET      9  // Only need one extra pin really
  #define PIN_G4_OFFSET      9
  #define PIN_B4_OFFSET      9
#endif

// LCD Digital Pins
#define LCD_PIN_RS      6
#define LCD_PIN_ENABLE  7
#define LCD_PIN_BLIGHT  13
#define LCD_PIN_D4      2
#define LCD_PIN_D5      3
#define LCD_PIN_D6      4
#define LCD_PIN_D7      5

// LCD I2C addr
#define LCD_I2C_ADDR  0x20
