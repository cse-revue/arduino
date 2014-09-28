// Using this: http://en.wikipedia.org/wiki/HSL_and_HSV#Converting_to_RGB
// And this: http://axonflux.com/handy-rgb-to-hsl-and-rgb-to-hsv-color-model-c
void hsv_to_rgb(int h_int, int s_int, int v_int, int rgb[]) {
  // Convert H to [0, 360) range and divide by 60
  // Convert S to [0, 1] range
  // Convert V to [0, 1] range
  float h = h_int / 255.0;
  float s = s_int / 255.0;
  float v = v_int / 255.0;
  float r = 0;
  float g = 0;
  float b = 0;
  
  int i = floor(h * 6);
  float f = h * 6 - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);
  
  switch(i % 6){
    case 0: r = v, g = t, b = p; break;
    case 1: r = q, g = v, b = p; break;
    case 2: r = p, g = v, b = t; break;
    case 3: r = p, g = q, b = v; break;
    case 4: r = t, g = p, b = v; break;
    case 5: r = v, g = p, b = q; break;
  }
  
  rgb[INDEX_R] = r * 255;
  rgb[INDEX_G] = g * 255;
  rgb[INDEX_B] = b * 255;
}
