void blink16(CRGB color) {
  FOR_i(0, 3) {
    fill_solid(leds, 16, color);
    FastLED.show();
    delay(300);
    FastLED.clear();
    FastLED.show();
    delay(300);
  }
}

void showRGB() {
  leds[0] = CRGB::Red;
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Blue;
  FastLED.show();
  FastLED.clear();
  delay(1500);
}

const uint8_t font5x7[][5] = {
  {0x3e, 0x51, 0x49, 0x45, 0x3e}, // 0 0x30 48
  {0x00, 0x42, 0x7f, 0x40, 0x00}, // 1 0x31 49
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 2 0x32 50
  {0x21, 0x41, 0x45, 0x4b, 0x31}, // 3 0x33 51
  {0x18, 0x14, 0x12, 0x7f, 0x10}, // 4 0x34 52
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 5 0x35 53
  {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 6 0x36 54
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 7 0x37 55
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8 0x38 56
  {0x06, 0x49, 0x49, 0x29, 0x1e}, // 9 0x39 57
  {0x00, 0x08, 0x08, 0x08, 0x00}, // 10 -
  {0x00, 0x00, 0x00, 0x00, 0x00}, // 11 empty
};

void drawDigit(byte digit, int X, int Y, CRGB color) {
  FOR_i(0, 5) {
    FOR_j(0, 7) {
      if (font5x7[digit][i] & (1 << 6 - j)) setPix(i + X, j + Y, color);
    }
  }
}
void drawDots(int X, int Y, CRGB color) {
  setPix(X, Y + 2, color);
  setPix(X, Y + 4, color);
}


uint32_t getWeekMS(byte hour, byte min, byte sec, uint16_t ms) {
  return (hour * 3600ul + min * 60 + sec) * 1000 + ms;
}

void timeTicker() {
  if (timeTmr.isReady()) {
    if (cfg.WiFimode && WiFi.status() == WL_CONNECTED && !connTmr.isEnabled()) {  // если вайфай подключен и это не попытка переподключиться
      now.sec = ntp.second();
      now.min = ntp.minute();
      now.hour = ntp.hour();
      now.day = ntp.day();   // вс 0, сб 6
      now.weekMs = now.getWeekS() * 1000ul + ntp.ms();
      now.setMs(ntp.ms());          
    } else {          // если вайфай не подключен
      now.tick();     // тикаем своим счётчиком
    }
  }
}

// void drawClock(byte Y, byte speed, CRGB color) {
//   if (cfg.deviceType == 1) return;   // лента - на выход
//   byte h1, h2, m1, m2;
//   if (gotNTP || gotTime) {
//     h1 = now.hour / 10;
//     if (h1 == 0) h1 = 11;
//     h2 = now.hour % 10;
//     m1 = now.min / 10;
//     m2 = now.min % 10;
//   } else {
//     h1 = h2 = m1 = m2 = 10;
//   }  
//   int pos;
//   if (speed == 0) pos = cfg.width / 2 - 13;
//   else pos = cfg.width - (now.weekMs / (speed * 2)) % (cfg.width + 26);
//   drawDigit(h1, pos, Y, color);
//   drawDigit(h2, pos + 6, Y, color);
//   if (now.getMs() < 500) drawDots(pos + 12, Y, color);
//   drawDigit(m1, pos + 14, Y, color);
//   drawDigit(m2, pos + 20, Y, color);
// }

void plotVoltage() {
  static GTimer tmr(MS, 5000);
  if (tmr.isReady()){    
    float adc_voltage_fil = 0.0;
    float adc_voltage_raw = 0.0;
    float in_voltage_fil = 0.0;
    float in_voltage_raw = 0.0;
    adc_voltage_fil = volt.getFil() / 1024.0;
    adc_voltage_raw = volt.getRaw() / 1024.0;
    in_voltage_fil = adc_voltage_fil / 0.322;
    in_voltage_fil = in_voltage_fil / 0.236;
    in_voltage_raw = adc_voltage_raw / 0.322;
    in_voltage_raw = in_voltage_raw / 0.236;
    DEBUGLN(in_voltage_raw);
    DEBUGLN(in_voltage_fil);   
    hub.sendCLI(sutil::AnyValue(in_voltage_raw));
    hub.sendCLI(sutil::AnyValue(in_voltage_fil));   
  }
}

void adjust(String str) {
    if(str == "+"){
      ina.adjCalibration(20);
    }
    if(str == "++"){
      ina.adjCalibration(100);
    }
    if(str == "-"){
      ina.adjCalibration(-20);
    }
    if(str == "--"){
      ina.adjCalibration(-100);
    }
}