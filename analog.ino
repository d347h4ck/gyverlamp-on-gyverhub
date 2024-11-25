#if (USE_ADC == 1)
void setupADC() {
  if (cfg.adcMode != GL_ADC_NONE) {
    phot.setDt(80);
    phot.setK(31);

    volt.setDt(80);
    volt.setK(31);

    if (cfg.adcMode == GL_ADC_BRI) switchToPhot();
    else if (cfg.adcMode == GL_ADC_VOL) switchToVol();
  }  
}


void checkAnalog() {
  if (cfg.role && adcTmr.isReady()) {   // только мастер или слейв по таймауту опрашивает АЦП!
    switch (cfg.adcMode) {
      case GL_ADC_NONE: break;
      case GL_ADC_BRI: checkPhot(); break;
      case GL_ADC_VOL: checkVoltage(); break;
    }
  }
}

void checkPhot() {
  static GTimer tmr(MS, 1000);  
  if (tmr.isReady()) phot.setRaw(analogRead(A0));
  phot.compute();
}

void checkVoltage() {
  static GTimer tmr(MS, 1000);
  if (tmr.isReady()) volt.setRaw(analogRead(A0));
  volt.compute();
}

void switchToPhot() {
  pinMode(PHOT_VCC, OUTPUT);
  digitalWrite(PHOT_VCC, 1);
}
void switchToVol() {
  digitalWrite(PHOT_VCC, 0);
  pinMode(PHOT_VCC, INPUT);
}
void disableADC() {
  digitalWrite(PHOT_VCC, 0);
  pinMode(PHOT_VCC, INPUT);
}
#else
void setupADC() {}
void checkAnalog() {}
void checkMusic() {}
void checkPhot() {}
void checkVoltage() {}
byte getSoundVol() {
  return 0;
}
void switchToMic() {}
void switchToPhot() {}
void switchToVol() {}
void disableADC() {}
#endif
