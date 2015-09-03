
void init_led_output(int pin_start, int offset) {
  pinMode(pin_start + offset, OUTPUT);
}

void output_output(int value, int pin_start, int offset) {
  if (value < 0) value = 0;
  if (value > 255) value = 255;
  
  analogWrite(pin_start + offset, value);
}


void output_outputs() {  
  for (int i = 0; i < num_outs; i++) {
      output_output(outputs[i], pin_start,  pin_channel_offeset[i]);
  }
}

void zero_outputs() {
  for (int i = 0; i < MAX_LED_OUTS; i++) {
      outputs[i] = 0;
  }
}

