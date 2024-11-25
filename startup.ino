void checkButton() {
#if (USE_BTN == 1)
  DEBUGLN(cfg.WiFimode ? "local mode" : "AP mode");
  if (btn.isHold()) {          // кнопка зажата
    FastLED.clear();
    byte count = 0;
    bool state = 0;

    while (btn.state()) {     // пока зажата кнопка
      fill_solid(leds, constrain(count, 0, 8), CRGB::Red);
      count++;
      if (count == 9) {               // на счёт 9 поднимаем яркость и флаг
        FastLED.setBrightness(120);
        state = 1;
      } else if (count == 16) {       // на счёт 16 опускаем флаг выходим
        state = 0;
        break;
      }
      FastLED.show();
      delay(300);
    }
    if (state) {
      DEBUGLN("change mode");
      cfg.WiFimode = !cfg.WiFimode;
      EEPROM.put(0, cfg);
      EEPROM.commit();
      delay(100);
      ESP.restart();
    }
  }
  FastLED.setBrightness(50);
  FastLED.clear();
  FastLED.show();
#endif
}

void startStrip() {
  FastLED.addLeds<STRIP_CHIP, STRIP_PIN, STRIP_COLOR>(leds, MAX_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(STRIP_VOLT, 500);
  FastLED.setBrightness(50);
  FastLED.show();
}

void startWiFi() {
  if (!cfg.WiFimode) setupAP();   // режим точки доступа
  else setupLocal();              // подключаемся к точке

  FastLED.clear();
  FastLED.show();
}

void tryReconnect() {
  if (connTmr.isReady()) {
    DEBUGLN("Reconnect");
    startWiFi();
  }
}

void setupAP() {
  blink16(CRGB::Yellow);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.softAP(AP_NameChar, WiFiPassword);
  DEBUGLN("Setting AP Mode");
  DEBUG("AP IP: ");
  DEBUGLN(WiFi.softAPIP());
  delay(500);
}

void setupLocal() {
  if (cfg.ssid[0] == '\0' && cfg.pass[0] == '\0') {
    DEBUGLN("WiFi is not configured");
    setupAP();
  } else {
    DEBUGLN("Connecting to AP...");
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(100);
    uint32_t tmr = millis();
    bool connect = false;
    int8_t count = 0, dir = 1;
    byte failCount = 0;
    while (1) {
      WiFi.begin(cfg.ssid, cfg.pass);
      while (millis() - tmr < 10000) {
        if (WiFi.status() == WL_CONNECTED) {
          connect = true;
          break;
        }
        FastLED.clear();
        leds[count] = CRGB::Yellow;
        FastLED.show();
        count += dir;
        if (count >= 15 || count <= 0) dir *= -1;
        delay(50);
      }
      if (connect) {
        connTmr.stop();
        blink16(CRGB::Green);
        DEBUG("Connected! Local IP: ");
        DEBUGLN(WiFi.localIP());
        hub.sendCLI(WiFi.localIP().toString());
        delay(500);
        return;
      } else {
        DEBUGLN("Failed!");
        blink16(CRGB::Red);
        failCount++;
        tmr = millis();
        if (failCount >= 3) {
          connTmr.start();    // попробуем позже
          setupAP();
          return;
        }
      }
    }
  }
}