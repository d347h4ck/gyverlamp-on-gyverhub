void changePreset(int dir) {
  cfg.curPreset += dir;
  if (cfg.curPreset >= cfg.presetAmount) cfg.curPreset = 0;
  if (cfg.curPreset < 0) cfg.curPreset = cfg.presetAmount - 1;
  DEBUG("Preset changed to ");
  DEBUGLN(cfg.curPreset);
}

void setPreset(byte pres) {
  cfg.curPreset = constrain(pres, 0, cfg.presetAmount - 1);
  DEBUG("Preset set to ");
  DEBUGLN(cfg.curPreset);
}

void setPower(bool state) {
  if (cfg.state != state) {
    cfgFile.update();
  }
  cfg.state = state;
  if (!state) {
    delay(100);     // чтобы пролететь мин. частоту обновления
    FastLED.clear(true);
    FastLED.show();
  }
  DEBUGLN(state ? "Power on" : "Power off");
}
