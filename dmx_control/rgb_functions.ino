#include "defs.h"

void init_outputs(int pin_start, int r_offset, int g_offset, int b_offset) {
  pinMode(pin_start + r_offset, OUTPUT);
  pinMode(pin_start + g_offset, OUTPUT);
  pinMode(pin_start + b_offset, OUTPUT);
}

void output_rgb_(int rgb[], int pin_start, int r_offset, int g_offset, int b_offset) {
  for (byte i = 0; i != 3; ++i) {
    if (rgb[i] < 0) rgb[i] = 0;
    if (rgb[i] > 255) rgb[i] = 255;
  }
  
  analogWrite(pin_start + r_offset, rgb[INDEX_R]);
  analogWrite(pin_start + g_offset, rgb[INDEX_G]);
  analogWrite(pin_start + b_offset, rgb[INDEX_B]);
}

void zero_rgb_(int rgb[]) {
  rgb[INDEX_R] = 0;
  rgb[INDEX_G] = 0;
  rgb[INDEX_B] = 0;
}

// Load random RGB Values
void setup_rgb_random(int rgb[]) {
  rgb[0] = 7;
  rgb[1] = 13;
  rgb[2] = 19;
}

byte rgb_random(int rgb_1[], int index_r, int index_g, int index_b) {
  rgb_1[index_r] = (rgb_1[index_r] * 7) % 255;
  rgb_1[index_g] = (rgb_1[index_g] * 7) % 255;
  rgb_1[index_b] = (rgb_1[index_b] * 7) % 255;
  return true;
}
