void fillStrip(int from, int to, CRGB color) {
  if (cfg.deviceType > 1) {
    FOR_i(from, to) {
      FOR_j(0, cfg.width) leds[getPix(j, i)] = color;
    }
  } else {
    FOR_i(from, to) leds[i] = color;
  }
}

void fillRow(int row, CRGB color) {
  FOR_i(cfg.width * row, cfg.width * (row + 1)) leds[i] = color;
}

void setPix(int x, int y, CRGB color) {
  if (y >= 0 && y < cfg.length && x >= 0 && x < cfg.width) leds[getPix(x, y)] = color;
}

void setPixOverlap(int x, int y, CRGB color) {
  if (y < 0) y += cfg.length;
  if (x < 0) x += cfg.width;
  if (y >= cfg.length) y -= cfg.length;
  if (x >= cfg.width) x -= cfg.width;
  setPix(x, y, color);
}

// получить номер пикселя в ленте по координатам
uint16_t getPix(int x, int y) {
  int matrixW;
  if (cfg.matrix == 2 || cfg.matrix == 4 || cfg.matrix == 6 || cfg.matrix == 8)  matrixW = cfg.length;
  else matrixW = cfg.width;
  int thisX, thisY;
  switch (cfg.matrix) {
    case 1: thisX = x;                    thisY = y;                    break;
    case 2: thisX = y;                    thisY = x;                    break;
    case 3: thisX = x;                    thisY = (cfg.length - y - 1); break;
    case 4: thisX = (cfg.length - y - 1); thisY = x;                    break;
    case 5: thisX = (cfg.width - x - 1);  thisY = (cfg.length - y - 1); break;
    case 6: thisX = (cfg.length - y - 1); thisY = (cfg.width - x - 1);  break;
    case 7: thisX = (cfg.width - x - 1);  thisY = y;                    break;
    case 8: thisX = y;                    thisY = (cfg.width - x - 1);  break;
  }

  if ( !(thisY & 1) || (cfg.deviceType - 2) ) return (thisY * matrixW + thisX);   // чётная строка
  else return (thisY * matrixW + matrixW - thisX - 1);                            // нечётная строка
}

void setLED(int x, CRGB color) {
  if (x >= 0 && x < cfg.length) leds[x] = color;
}

uint32_t getPixColor(int x, int y) {
  int thisPix = getPix(x, y);
  if (thisPix < 0 || thisPix >= MAX_LEDS) return 0;
  return (((uint32_t)leds[thisPix].r << 16) | ((long)leds[thisPix].g << 8 ) | (long)leds[thisPix].b);
}