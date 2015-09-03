// Persistant storage
#include <EEPROM.h>

// DMX Control Includes
#include <Conceptinetics.h>

// LiquidCrystal
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

//Keypad
#include <Keypad.h>

// For Software reset & reboot
#include <avr/wdt.h>

// defs
#include "defs.h"

// Configure a DMX slave controller
// THIS MUST BE HERE FOR GLOBAL SETTING
DMX_Slave dmx_slave ( DMX_SLAVE_CHANNELS, RXEN_PIN );

// DMX Variables
int dmx_addr = DEFAULT_DMX_ADDR;

// DMX Timeout
long lastArduinoLoopTime;
long lastFrameReceivedTime;
byte dmx_timedout;

/* LCD I2C Initialisation
 The circuit:
 * LCD SDA pin to analog pin 4
 * LCD SCL pin to analog pin 5
*/
LiquidCrystal_I2C lcd(LCD_I2C_ADDR,16,2);

// LED Variables
const int pin_start = DEFAULT_PIN_START;
byte update_lcd;

// Keypad Variables And INIT
const byte numRows= 1; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad
char keymap[numRows][numCols]= {{'M', 'U', 'D', 'S'}};
byte rowPins[numRows] = {4};
byte colPins[numCols]= {A0,A1,A2,A3};
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);


// Menu management
byte output_enabled;       // Output RGB
byte menu;                 // Menu type
byte menu_opts_changed;     // Address has changed

// LED Storage values
byte num_outs = DEFAULT_LED_OUTS;
int outputs[MAX_LED_OUTS];

// the setup routine runs once when you press reset:
void setup() { 
  // Load Options
  // FIRST TIME LOAD
  //save_options();
  load_options(); 
  
  // Initialise LED digitial output pins
  for (int i = 0; i < MAX_LED_OUTS; i++) {
    init_led_output(pin_start, pin_channel_offeset[i]);
  }
  zero_outputs();
  output_outputs();
  
  // Configure DMX
  dmx_slave.enable();
  dmx_slave.setStartAddress(dmx_addr);
  
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
  char button = NO_KEY;
  button = myKeypad.getKey();
  process_button(button);
  
  // Load new LED values
  if (output_enabled) {
    byte new_val = false;
    
    // Check for timeout
    //DMX_TIMEOUT
    if (lastArduinoLoopTime - lastFrameReceivedTime >= DMX_TIMEOUT) {
      // Timed-out
      new_val = true;
      dmx_timedout = true;
      zero_outputs();
    } else {
      // Have got DMX
      dmx_timedout = false;
      load_led_dmx();
      new_val = true;  // Always assume new val for now
    }    
    update_lcd = new_val;
  } else {
    zero_outputs();
  }
  
  // Output new values
  output_outputs();
  
  // Output LCD values
  output_lcd();
}

// Process button press
void process_button(char button) {
  // Only process just pressed buttons
  if (button == NO_KEY) return;
  
  // Update lcd for button choice
  update_lcd = true;
  
  // Select button is special
  if (button == 'M') {
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
    return;
  }
  
  // Process up/down button
  if (button == 'U' || button == 'D') {
    // Do menu increment when menu option is within list
    // Otherwise do special up/down for special meny list
    if (menu < N_MENU_OPTIONS) {  
      menu += (button == 'U') ? 1 : -1;
      menu = menu % N_MENU_OPTIONS;
    } else if (menu == MENU_IN_RESET) {
      if (button == 'U') {
        software_reset();
      } else if (button == 'D') {
        menu = MENU_RESET;
      }
    }
  }
  
  // Process set button
  if (button == 'S') {
    if (menu == MENU_ADDR) {
      dmx_addr += 1;
      if (dmx_addr < 1) dmx_addr = DMX_ADDRESSES;
      if (dmx_addr > DMX_ADDRESSES) dmx_addr = 1;
      
      // Update dmx offset address
      menu_opts_changed = true;
    } else if (menu == MENU_OUTS) {
      num_outs += 1;
      if (num_outs < 1) num_outs = MAX_LED_OUTS;
      if (num_outs > MAX_LED_OUTS) num_outs = 1;
      
      menu_opts_changed = true;
    } else if (menu == MENU_RESET) {
      if (button == 'S') {
        menu = MENU_IN_RESET;
      }
    } else if (menu == MENU_IN_RESET) {
      if (button == 'S') {
        menu = MENU_RESET;
      }
    }
  }
}


// OUTPUT LCD
void output_lcd() {
  if (!update_lcd) return;
  
  if (output_enabled) {   
    
    if(num_outs > 3) {
      int timeMod1 = lastArduinoLoopTime % (DISP_CYCLE_FREQ * 2);
      if (timeMod1 < DISP_CYCLE_FREQ) {
        for (int i = 0; i < 3; i++) {
            lcd.setCursor(i*6, 0);
            char let = 'A' + i;
            lcd.print(let);
            if (outputs[i]<100) lcd.print(' ');
            if (outputs[i]<10) lcd.print(' ');
            lcd.print(outputs[i]);
            
        }
      } else {
        for (int i = 3; i < num_outs; i++) {
            lcd.setCursor((i-3)*6, 0);
            char let = 'D' + i;
            lcd.print(let);
            if (outputs[i]<100) lcd.print(' ');
            if (outputs[i]<10) lcd.print(' ');
            lcd.print(outputs[i]);
        }
        if(num_outs < MAX_LED_OUTS) {
          lcd.setCursor((num_outs - 3)*6,0);
          for(int i = 0; i < MAX_LED_OUTS - num_outs; i++) {
            lcd.print("      ");
          }
        }
      }
    } else {
      for (int i = 0; i < num_outs; i++) {
          lcd.setCursor(i*6, 0);
          char let = 'A' + i;
          lcd.print(let);
          if (outputs[i]<100) lcd.print(' ');
          if (outputs[i]<10) lcd.print(' ');
          lcd.print(outputs[i]);
      }
    }
    
    int timeMod = lastArduinoLoopTime % (DISP_CYCLE_FREQ * 3);
    lcd.setCursor(0,1);
    if (timeMod < DISP_CYCLE_FREQ) {
      lcd.print("Addr ");
      lcd.print(dmx_addr);
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
      lcd.print("Y:Up N:Down");
    } else {
      lcd.print("????");
    }
  }
  
  update_lcd = false;
}

void load_led_dmx() {
  for (int i = 0; i < num_outs; i++) {
    load_led_dmx_(i);
  }
}

void load_led_dmx_(int index) {
  outputs[index] = dmx_slave.getChannelValue(index+1);
}

// Load/Save DMX Addresses
void save_options() {
  byte dmx_high_bit = dmx_addr / 255;
  byte dmx_data = dmx_addr % 255;
  
  EEPROM.write(STORE_DMX, dmx_high_bit);
  EEPROM.write(STORE_DMX + 1, dmx_data);
  EEPROM.write(STORE_DMX + 2, num_outs);
  
  menu_opts_changed = false;
}

void load_options() {
  byte dmx_high_bit = EEPROM.read(STORE_DMX);
  byte dmx_data = EEPROM.read(STORE_DMX + 1);
  
  dmx_addr = dmx_high_bit * 255 + dmx_data;
  num_outs = EEPROM.read(STORE_DMX + 2);
}

// Callback when recieved new DMX frame
void OnFrameReceiveComplete (unsigned short channelsReceived)
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
