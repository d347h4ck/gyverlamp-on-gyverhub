void perlinTick(byte thisScale, byte thisBright, int thisLength) {
  if (cfg.deviceType > 1) {
    FOR_j(0, cfg.length) {
      FOR_i(0, cfg.width) {
        setPix(i, j, ColorFromPalette(paletteArr[CUR_PRES.palette - 1],
                                      scalePal(inoise8(
                                        i * (thisScale / 5) - cfg.width * (thisScale / 5) / 2,
                                        j * (thisScale / 5) - cfg.length * (thisScale / 5) / 2,
                                        (now.weekMs >> 1) * CUR_PRES.speed / 255)),
                                        255, LINEARBLEND));
      }
    }

  } else {
    FOR_i(0, cfg.length) {
      leds[i] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1],
                                 scalePal(inoise8(i * (thisScale / 5) - cfg.length * (thisScale / 5) / 2,
                                          (now.weekMs >> 1) * CUR_PRES.speed / 255)),
                                          255, LINEARBLEND);
    }
  }
}

void colorTick(byte thisScale, byte thisBright, int thisLength) {
  fill_solid(leds, cfg.length * cfg.width, CHSV(CUR_PRES.color, thisScale, 30));
  CRGB thisColor = CHSV(CUR_PRES.color, thisScale, thisBright);
  if (CUR_PRES.fromCenter) {
    fillStrip(cfg.length / 2, cfg.length / 2 + thisLength / 2, thisColor);
    fillStrip(cfg.length / 2 - thisLength / 2, cfg.length / 2, thisColor);
  } else {
    fillStrip(0, thisLength, thisColor);
  }
}

void changeColorTick(byte thisScale, byte thisBright, int thisLength){
  CRGB thisColor = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal((now.weekMs >> 5) * CUR_PRES.speed / 255), 10, LINEARBLEND);
  fill_solid(leds, cfg.length * cfg.width, thisColor);
  thisColor = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal((now.weekMs >> 5) * CUR_PRES.speed / 255), thisBright, LINEARBLEND);
  if (CUR_PRES.fromCenter) {
    fillStrip(cfg.length / 2, cfg.length / 2 + thisLength / 2, thisColor);
    fillStrip(cfg.length / 2 - thisLength / 2, cfg.length / 2, thisColor);
  } else {
    fillStrip(0, thisLength, thisColor);
  }
}

void gradientTick(byte thisScale, byte thisBright, int thisLength){
  if (CUR_PRES.fromCenter) {
    FOR_i(cfg.length / 2, cfg.length) {
      byte bright = 255;
      CRGB thisColor = ColorFromPalette(
                        paletteArr[CUR_PRES.palette - 1],   // (x*1.9 + 25) / 255 - быстрый мап 0..255 в 0.1..2
                        scalePal((i * (thisScale * 1.9 + 25) / cfg.length) + ((now.weekMs >> 3) * (CUR_PRES.speed - 128) / 128)),
                        bright, LINEARBLEND);
      if (cfg.deviceType > 1) fillRow(i, thisColor);
      else leds[i] = thisColor;
    }
    if (cfg.deviceType > 1) FOR_i(0, cfg.length / 2) fillRow(i, leds[(cfg.length - i)*cfg.width - 1]);
    else FOR_i(0, cfg.length / 2) leds[i] = leds[cfg.length - i - 1];

  } else {
    FOR_i(0, cfg.length) {
      byte bright = 255;
      CRGB thisColor = ColorFromPalette(
                        paletteArr[CUR_PRES.palette - 1],   // (x*1.9 + 25) / 255 - быстрый мап 0..255 в 0.1..2
                        scalePal((i * (thisScale * 1.9 + 25) / cfg.length) + ((now.weekMs >> 3) * (CUR_PRES.speed - 128) / 128)),
                        bright, LINEARBLEND);
      if (cfg.deviceType > 1) fillRow(i, thisColor);
      else leds[i] = thisColor;
    }
  }
}

void twinkleTick(byte thisScale) {
  FOR_i(0, cfg.length * cfg.width) leds[i].fadeToBlackBy(70);
  {
    uint16_t rndVal = 0;
    byte amount = (thisScale >> 3) + 1;
    FOR_i(0, amount) {
      rndVal = rndVal * 2053 + 13849;     // random2053 алгоритм
      int homeX = inoise16(i * 100000000ul + (now.weekMs << 3) * CUR_PRES.speed / 255);
      homeX = map(homeX, 15000, 50000, 0, cfg.length);
      int offsX = inoise8(i * 2500 + (now.weekMs >> 1) * CUR_PRES.speed / 255) - 128;
      offsX = cfg.length / 2 * offsX / 128;
      int thisX = homeX + offsX;

      if (cfg.deviceType > 1) {
        int homeY = inoise16(i * 100000000ul + 2000000000ul + (now.weekMs << 3) * CUR_PRES.speed / 255);
        homeY = map(homeY, 15000, 50000, 0, cfg.width);
        int offsY = inoise8(i * 2500 + 30000 + (now.weekMs >> 1) * CUR_PRES.speed / 255) - 128;
        offsY = cfg.length / 2 * offsY / 128;
        int thisY = homeY + offsY;
        setPix(thisX, thisY, CUR_PRES.fromPal ?
                ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(i * 255 / amount), 255, LINEARBLEND) :
                CHSV(CUR_PRES.color, 255, 255)
              );
      } else {
        setLED(thisX, CUR_PRES.fromPal ?
                ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(i * 255 / amount), 255, LINEARBLEND) :
                CHSV(CUR_PRES.color, 255, 255)
              );
      }
    }
  }
}

void fire2020Tick(byte thisScale, int thisLength){
  FastLED.clear();
  if (cfg.deviceType > 1) {         // 2D огонь
    fire2020(thisScale, thisLength);
  } else {                          // 1D огонь
    static byte heat[MAX_LEDS];
    if (CUR_PRES.fromCenter) thisLength /= 2;

    for (int i = 0; i < thisLength; i++) heat[i] = qsub8(heat[i], random8(0, ((((255 - thisScale) / 2 + 20) * 10) / thisLength) + 2));
    for (int k = thisLength - 1; k >= 2; k--) heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    if (random8() < 120 ) {
      int y = random8(7);
      heat[y] = qadd8(heat[y], random8(160, 255));
    }
    if (CUR_PRES.fromCenter) {
      for (int j = 0; j < thisLength; j++) leds[cfg.length / 2 + j] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scale8(heat[j], 240));
      FOR_i(0, cfg.length / 2) leds[i] = leds[cfg.length - i - 1];
    } else {
      for (int j = 0; j < thisLength; j++) leds[j] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scale8(heat[j], 240));
    }
  }
}

void fire2DTick(byte thisScale, int thisLength){
  if (cfg.deviceType > 1) {         // 2D огонь
    fireRoutine(CUR_PRES.speed / 2);
  } else {                          // 1D огонь
    FastLED.clear();
    static byte heat[MAX_LEDS];
    CRGBPalette16 gPal;
    if (CUR_PRES.color < 5) gPal = HeatColors_p;
    else gPal = CRGBPalette16(CRGB::Black, CHSV(CUR_PRES.color, 255, 255), CRGB::White);
    if (CUR_PRES.fromCenter) thisLength /= 2;

    for (int i = 0; i < thisLength; i++) heat[i] = qsub8(heat[i], random8(0, ((((255 - thisScale) / 2 + 20) * 10) / thisLength) + 2));
    for (int k = thisLength - 1; k >= 2; k--) heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    if (random8() < 120 ) {
      int y = random8(7);
      heat[y] = qadd8(heat[y], random8(160, 255));
    }
    if (CUR_PRES.fromCenter) {
      for (int j = 0; j < thisLength; j++) leds[thisLength / 2 + j] = ColorFromPalette(gPal, scale8(heat[j], 240));
      FOR_i(0, thisLength / 2) leds[i] = leds[thisLength - i - 1];
    } else {
      for (int j = 0; j < thisLength; j++) leds[j] = ColorFromPalette(gPal, scale8(heat[j], 240));
    }
  }
}

void confettiTick(byte thisScale){
  byte amount = (thisScale >> 3) + 1;
  FOR_i(0, amount) {
    int x = random(0, cfg.length * cfg.width);
    if (leds[x] == CRGB(0, 0, 0)) leds[x] = CUR_PRES.fromPal ?
                                              ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(i * 255 / amount), 255, LINEARBLEND) :
                                              CHSV(CUR_PRES.color, 255, 255);
  }
  FOR_i(0, cfg.length * cfg.width) {
    if (leds[i].r >= 10 || leds[i].g >= 10 || leds[i].b >= 10) leds[i].fadeToBlackBy(CUR_PRES.speed / 2 + 1);
    else leds[i] = 0;
  }
}

void smerchTick(byte thisScale){
  FastLED.clear();
  FOR_k(0, (thisScale >> 5) + 1) {
    FOR_i(0, cfg.length) {
      //byte thisPos = inoise8(i * 10 - (now.weekMs >> 1) * CUR_PRES.speed / 255, k * 10000);
      byte thisPos = inoise8(i * 10 + (now.weekMs >> 3) * CUR_PRES.speed / 255 + k * 10000, (now.weekMs >> 1) * CUR_PRES.speed / 255);
      thisPos = map(thisPos, 50, 200, 0, cfg.width);
      byte scale = 4;
      FOR_j(0, scale) {
        CRGB color = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(j * 255 / scale), (255 - j * 255 / (scale - 1)), LINEARBLEND);
        if (j == 0) {
          setPixOverlap(thisPos, i, color);
        } else {
          setPixOverlap(thisPos - j, i, color);
          setPixOverlap(thisPos + j, i, color);
        }
      }
    }
  }
}
void effectsRoutine() {
  static byte prevEff = 255;
  if (!effTmr.isReady()) return;

  if (!cfg.state) return;
  int thisLength = getLength();
  byte thisScale = getScale();
  byte thisBright = getBright();

  FastLED.setBrightness(thisBright);

  if (prevEff != CUR_PRES.effect) {   // смена эффекта
    FastLED.clear();
    prevEff = CUR_PRES.effect;
    loading = true;
  }
  yield();

  // =================================================== ЭФФЕКТЫ ===================================================
  // uint32_t weekMS = getWeekMS(ntp.hour(), ntp.minute(), ntp.second(), ntp.ms());
  switch (CUR_PRES.effect) {
    case 1: // =================================== ПЕРЛИН ===================================
      {
        perlinTick(thisScale, thisBright, thisLength);
        break;
      }
    case 2: // ==================================== ЦВЕТ ====================================
      {
        colorTick(thisScale, thisBright, thisLength);
        break;
      }
    case 3: // ================================= СМЕНА ЦВЕТА =================================
      { 
        changeColorTick(thisScale, thisBright, thisLength);        
        break;
      }
    case 4: // ================================== ГРАДИЕНТ ==================================
      {
        gradientTick(thisScale, thisBright, thisLength);
        break;
      }
    case 5: // =================================== ЧАСТИЦЫ ===================================
      {
        twinkleTick(thisScale);
        break;
      }
    case 6: // ==================================== ОГОНЬ ====================================
      {
        fire2DTick(thisScale, thisLength);
        break;
      }
    case 7: // ==================================== ОГОНЬ 2020 ====================================
      {
        fire2020Tick(thisScale, thisLength);
        break;
      }
    case 8: // ================================== КОНФЕТТИ ==================================
      {
        confettiTick(thisScale);
        break;
      }
    case 9: // =================================== СМЕРЧ ===================================
      {
        smerchTick(thisScale);
        break;
      }
  }
  yield();
  FastLED.show();
}

// ====================================================================================================================

byte getBright() {
  int maxBr = cfg.bright;   // макс яркость из конфига
  byte fadeBr = 255;
  if (CUR_PRES.fadeBright) fadeBr = CUR_PRES.bright; // ограничен вручную

  return scaleFF(maxBr, fadeBr);
}

int getLength() {
  return cfg.length;
}

byte getScale() {
  return CUR_PRES.scale;
}

void updPal() {
  for (int i = 0; i < 16; i++) {
    paletteArr[0][i] = CRGB(pal.strip[i * 3], pal.strip[i * 3 + 1], pal.strip[i * 3 + 2]);
  }
  if (pal.size < 16) paletteArr[0][pal.size] = paletteArr[0][0];
}

byte scalePal(byte val) {
  if (CUR_PRES.palette == 1) val = val * pal.size / 16;
  return val;
}
