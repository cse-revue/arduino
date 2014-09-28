// Persistant storage
#include <EEPROM.h>

// DMX Control Includes
#include <Conceptinetics.h>

// LiquidCrystal
//#include <LiquidCrystal.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// For Software reset & reboot
#include <avr/wdt.h>

// RGB defs
#include "defs.h"

//
// The slave device will use a block of 10 channels counting from
// its start address.
//
// If the start address is for example 56, then the channels kept
// by the dmx_slave object is channel 56-66
//

// Configure a DMX slave controller
// THIS MUST BE HERE FOR GLOBAL SETTING
DMX_Slave dmx_slave ( DMX_SLAVE_CHANNELS, RXEN_PIN );

// DMX Variables
int dmx_addr = DEFAULT_DMX_ADDR;

// DMX Timeout
long lastArduinoLoopTime;
long lastFrameReceivedTime;
byte dmx_timedout;

/* LCD Initialisation
 The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/
// initialize the library with the numbers of the interface pins
//LiquidCrystal lcd(LCD_PIN_RS, LCD_PIN_ENABLE, LCD_PIN_D4, LCD_PIN_D5, LCD_PIN_D6, LCD_PIN_D7);

/* LCD I2C Initialisation
 The circuit:
 * LCD SDA pin to analog pin 4
 * LCD SCL pin to analog pin 5
*/
LiquidCrystal_I2C lcd(LCD_I2C_ADDR,16,2);

// LED Variables
const int pin_start = DEFAULT_PIN_START;
byte update_lcd;

// Menu management
byte output_enabled;       // Output RGB
byte menu;                 // Menu type
byte menu_opts_changed;     // Address has changed

// RGB Storage values
byte num_outs = DEFAULT_LED_OUTS;
int rgb_1[3];
int rgb_2[3];
int rgb_3[3];
int rgb_4[3];

// RGB or HSV
byte rgb_mode = DEFAULT_RGB_MODE;

// the setup routine runs once when you press reset:
void setup() { 
  // Load Options
  // FIRST TIME LOAD
  //save_options();
  load_options();
  
  // Initialise LED digitial output pins
  init_outputs(pin_start, PIN_R1_OFFSET, PIN_G1_OFFSET, PIN_B1_OFFSET);
  init_outputs(pin_start, PIN_R2_OFFSET, PIN_G2_OFFSET, PIN_B2_OFFSET);
  #ifdef ARDUINO_MEGA
    init_outputs(pin_start, PIN_R3_OFFSET, PIN_G3_OFFSET, PIN_B3_OFFSET);
    init_outputs(pin_start, PIN_R4_OFFSET, PIN_G4_OFFSET, PIN_B4_OFFSET);
  #endif
  zero_rgb();
  output_rgb();
  
  // Configure DMX
  dmx_slave.enable();
  dmx_slave.setStartAddress(dmx_addr);
  
  // Setup LCD (Digital)
  // set up the LCD's number of columns and rows: 
  //lcd.begin(16, 2);
  
  // Setup LCD I2C
  lcd.init();
  lcd.backlight();
  
  // Default LCD Display
  lcd.clear();
  update_lcd = true;
  
  // TODO: Check if need the recieve to avoid too many outputs?
  lastArduinoLoopTime = 0;
  lastFrameReceivedTime = -DMX_TIMEOUT + 1;
  dmx_timedout = true;
  dmx_slave.onReceiveComplete ( OnFrameReceiveComplete );
  
  // Setup button reads
  setup_buttons();
  
  // Setup RGB reads
  //setup_rgb_random(rgb);
  
  // Setup menu controls
  output_enabled = true;
  menu = NO_MENU;
  menu_opts_changed = false;
  
  //THIS IS THE MOST IMPORTANT LINE OF CODE IN THE GOD DAMN PROGRAM
  UCSR0A &= ~(1 << U2X0);
}

// the loop routine runs over and over again forever:
void loop() 
{
  // Update process time
  lastArduinoLoopTime = millis();
  
  // Process buttons
  byte button = BUTTON_NONE;
  button = ReadButtons();
  process_button(button);
  
  // Load new RGB values
  if (output_enabled) {
    byte new_val = false;
    
    // Check for timeout
    //DMX_TIMEOUT
    if (lastArduinoLoopTime - lastFrameReceivedTime >= DMX_TIMEOUT) {
      // Timed-out
      new_val = true;
      dmx_timedout = true;
      zero_rgb();
    } else {
      // Have got DMX
      dmx_timedout = false;
      new_val = rgb_dmx();
      new_val = true;  // Always assume new val for now
    }
   
    //byte new_val = rgb_random(rgb, INDEX_R, INDEX_G, INDEX_B);
    
    update_lcd = new_val;
  } else {
    zero_rgb();
  }
  
  // Output new values
  output_rgb();
  
  // Output LCD values
  output_lcd();
  
  //if (output_enabled) {
  //  delay(500);
  //}
}

// Output RGB LED values given the R pin
void output_rgb() {
  output_rgb_(rgb_1, pin_start, PIN_R1_OFFSET, PIN_G1_OFFSET, PIN_B1_OFFSET);
  
  if (num_outs >= 2) {
    output_rgb_(rgb_2, pin_start, PIN_R2_OFFSET, PIN_G2_OFFSET, PIN_B2_OFFSET);
  }
  if (num_outs >= 3) {
    output_rgb_(rgb_3, pin_start, PIN_R3_OFFSET, PIN_G3_OFFSET, PIN_B3_OFFSET);
  }
  if (num_outs >= 4) {
    output_rgb_(rgb_4, pin_start, PIN_R4_OFFSET, PIN_G4_OFFSET, PIN_B4_OFFSET);
  }
}

void zero_rgb() {
  zero_rgb_(rgb_1);
  zero_rgb_(rgb_2);
  zero_rgb_(rgb_3);
  zero_rgb_(rgb_4);
}

// Process button press
void process_button(byte button) {
  // Only process just pressed buttons
  if (button == BUTTON_NONE) return;
  if (!getButtonJustPressed()) return;
  
  // Update lcd for button choice
  update_lcd = true;
  
  // Select button is special
  if (button == BUTTON_SELECT) {
    if (menu == NO_MENU) {
      // Enter Menu
      menu = MENU_INFO;
      output_enabled = false;
    } else {
      // Exit Menu
      menu = NO_MENU;
      output_enabled = true;
      lcd.clear();
      
      // Write new dmx address
      if (menu_opts_changed) {
        save_options();
        dmx_slave.setStartAddress(dmx_addr);
        menu_opts_changed = false;
      }
    }
  }
  
  // Ignore buttons when not in menu
  if (menu == NO_MENU) {
    clearButtonJustPressed();
    return;
  }
  
  // Process up/down button
  if (button == BUTTON_UP || button == BUTTON_DOWN) {
    // Do menu increment when menu option is within list
    // Otherwise do special up/down for special meny list
    if (menu < N_MENU_OPTIONS) {  
      menu += (button == BUTTON_DOWN) ? 1 : -1;
      menu = menu % N_MENU_OPTIONS;
    } else if (menu == MENU_IN_RESET) {
      if (button == BUTTON_DOWN) {
        software_reset();
      } else if (button == BUTTON_UP) {
        menu = MENU_RESET;
      }
    }
  }
  
  // Process left/right button
  if (button == BUTTON_RIGHT || button == BUTTON_LEFT) {
    if (menu == MENU_ADDR) {
      dmx_addr += (button == BUTTON_RIGHT) ? 1 : -1;
      if (dmx_addr < 1) dmx_addr = DMX_ADDRESSES;
      if (dmx_addr > DMX_ADDRESSES) dmx_addr = 1;
      
      // Update dmx offset address
      menu_opts_changed = true;
    } else if (menu == MENU_MRGB) {
      rgb_mode = !rgb_mode;
      
      menu_opts_changed = true;
    } else if (menu == MENU_OUTS) {
      num_outs += (button == BUTTON_RIGHT) ? 1 : -1;
      if (num_outs < 1) num_outs = MAX_LED_OUTS;
      if (num_outs > MAX_LED_OUTS) num_outs = 1;
      
      menu_opts_changed = true;
    } else if (menu == MENU_RESET) {
      if (button == BUTTON_RIGHT) {
        menu = MENU_IN_RESET;
      }
    } else if (menu == MENU_IN_RESET) {
      if (button == BUTTON_LEFT) {
        menu = MENU_RESET;
      }
    }
  }
  
  // Set button as pressed
  clearButtonJustPressed();
}

// OUTPUT LCD
void output_lcd() {
  //lcd.setCursor(12,1);
  //lcd.print(lastFrameReceivedTime);
  
  if (!update_lcd) return;
  
  if (output_enabled) {
    byte num = 1;
    int* rgb = rgb_1;
    
    if (num_outs >= 4 && lastArduinoLoopTime % (DISP_CYCLE_FREQ * num_outs) >= (DISP_CYCLE_FREQ * 3)) {
      num = 4;
      rgb = rgb_4;
    } else if (num_outs >= 3 && lastArduinoLoopTime % (DISP_CYCLE_FREQ * num_outs) >= (DISP_CYCLE_FREQ * 2)) {
      num = 3;
      rgb = rgb_3;
    } else if (num_outs >= 2 && lastArduinoLoopTime % (DISP_CYCLE_FREQ * num_outs) >= (DISP_CYCLE_FREQ * 1)) {
      num = 2;
      rgb = rgb_2;
    }
    
    lcd.setCursor(0,0);
    lcd.print(num);
    lcd.setCursor(1, 0);
    lcd.print(":");
    lcd.setCursor(2, 0);
    lcd.print("R");
    lcd.print(rgb[INDEX_R]);
    lcd.print("  ");
    lcd.setCursor(7, 0);
    lcd.print("G");
    lcd.print(rgb[INDEX_G]);
    lcd.print("  ");
    lcd.setCursor(12, 0);
    lcd.print("B");
    lcd.print(rgb[INDEX_B]);
    lcd.print("  ");
    
    int timeMod = lastArduinoLoopTime % (DISP_CYCLE_FREQ * 3);
    lcd.setCursor(0,1);
    if (timeMod < DISP_CYCLE_FREQ) {
      lcd.print("Addr ");
      lcd.print(dmx_addr);
    } else if (timeMod < (DISP_CYCLE_FREQ * 2)) {
      if (rgb_mode) {
        lcd.print("RGB     ");
      } else {
        lcd.print("HSV     ");
      }
    } else {
      lcd.print(num_outs);
      lcd.print("-outs  ");
    }
    
    if (dmx_timedout) {
      lcd.setCursor(10,1);
      lcd.print("NO DMX");
    } else {
      lcd.setCursor(10,1);
      lcd.print("      ");
    }
  } else {
    lcd.clear();
    
    if (menu < N_MENU_OPTIONS) {
      lcd.setCursor(0, 0);
      lcd.print("Menu");
      
      lcd.setCursor(0, 1);
      if (menu == MENU_INFO) {
        lcd.print("Exit");
      } else if (menu == MENU_ADDR) {
        lcd.print("DMX Addr: ");
        lcd.print(dmx_addr);
      } else if (menu == MENU_MRGB) {
        lcd.print("Mode: ");
        if (rgb_mode) {
          lcd.print("RGB");
        } else {
          lcd.print("HSV");
        }
      } else if (menu == MENU_OUTS) {
        lcd.print("Num Outs: ");
        lcd.print(num_outs);
      } else if (menu == MENU_RESET) {
        lcd.print("Reset");
      }
    } else if (menu == MENU_IN_RESET) {
      lcd.setCursor(0,0);
      lcd.print("Do Reset?");
      lcd.setCursor(0,1);
      lcd.print("Y:Down N:Up/Down");
    } else {
      lcd.print("????");
    }
  }
  
  update_lcd = false;
}

byte rgb_dmx() {
  // RGB 1
  byte retVal = rgb_dmx_(rgb_1, 1);
  
  // RGB 2
  if (num_outs >= 2) {
    byte retVal2 = rgb_dmx_(rgb_2, 4);
    retVal = retVal || retVal2;
  }
  
  // RGB 3
  if (num_outs >= 3) {
    byte retVal3 = rgb_dmx_(rgb_3, 7);
    retVal = retVal || retVal3;
  }
  
    // RGB 3
  if (num_outs >= 4) {
    byte retVal3 = rgb_dmx_(rgb_4, 10);
    rgb_4[INDEX_B] = rgb_4[INDEX_R];
    rgb_4[INDEX_G] = rgb_4[INDEX_R];
    retVal = retVal || retVal3;
  }
  
  //retVal = true;
  return retVal;
}

byte rgb_dmx_(int rgb[], int start_addr) {
  int r = dmx_slave.getChannelValue (start_addr);
  int g = dmx_slave.getChannelValue (start_addr + 1);
  int b = dmx_slave.getChannelValue (start_addr + 2);
  byte retVal = (r == rgb[INDEX_R]) || (g == rgb[INDEX_G]) || (b == rgb[INDEX_B]);
  if (rgb_mode) {
    rgb[INDEX_R] = r;
    rgb[INDEX_G] = g;
    rgb[INDEX_B] = b;
  } else {
    hsv_to_rgb(r, g, b, rgb);
  }
  
  return retVal;
}

// Load/Save DMX Addresses
void save_options() {
  byte dmx_high_bit = dmx_addr / 255;
  byte dmx_data = dmx_addr % 255;
  
  EEPROM.write(STORE_DMX, dmx_high_bit);
  EEPROM.write(STORE_DMX + 1, dmx_data);
  EEPROM.write(STORE_DMX + 2, num_outs);
  EEPROM.write(STORE_DMX + 3, rgb_mode);
  
  menu_opts_changed = false;
}

void load_options() {
  byte dmx_high_bit = EEPROM.read(STORE_DMX);
  byte dmx_data = EEPROM.read(STORE_DMX + 1);
  
  dmx_addr = dmx_high_bit * 255 + dmx_data;
  num_outs = EEPROM.read(STORE_DMX + 2);
  rgb_mode = EEPROM.read(STORE_DMX + 3);
}

// Callback when recieved new DMX frame
void OnFrameReceiveComplete (void)
{
  lastFrameReceivedTime = millis ();
}

// ONLY CALL IF CERTAIN YOU WANT TO RESET & REBOOT
void software_reset()
{
  // Show reset on LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Reset Selected");
  
  // Do Reset
  dmx_addr = DEFAULT_DMX_ADDR;
  num_outs = DEFAULT_LED_OUTS;
  rgb_mode = DEFAULT_RGB_MODE;
  save_options();
  
  // Update LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Reset Done");
  lcd.setCursor(0,1);
  lcd.print("Rebooting");
  
  wdt_enable(WDTO_2S);
  while(1)
  {
  }
}
