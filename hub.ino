void build(gh::Builder& hub) {
  static byte tab;
  if (hub.Tabs(&tab).text("Управление;Конфиг;Пресеты").noLabel().click()) {
    hub.refresh();
  } 

  hub.show(tab == 0);
  {
    gh::Row r(hub);
    hub.Title(F("Управление лампой")).fontSize(18).size(8);
    gh::Flag stateFlag;
    hub.Switch_("state", &cfg.state).noLabel().noTab().size(2).attach(&stateFlag);
    if (stateFlag) {
      setPower(cfg.state);
      hub.refresh();
    }        
  }
  hub.show(tab == 0);
  {
    gh::Row r(hub);
    hub.Label("Пресет").fontSize(16).noLabel().disabled(!cfg.state).size(6);
    static mString<21 * MAX_PRESETS> pres;
    pres = "";
    FOR_i(0, cfg.presetAmount) {
      pres += presets.presets[i].name;
      if (i < cfg.presetAmount - 1){
        pres += ";";
      }
    }
    gh::Flag curPresFlag;
    hub.Select(&cfg.curPreset).text(pres.c_str()).noLabel().size(4).disabled(!cfg.state).attach(&curPresFlag);
    if (curPresFlag){
      cfgFile.update();
    }
  }
  hub.show(tab == 0);
  {
    gh::Row r(hub);
    hub.GaugeRound(&curVoltage).noLabel().size(2).unit("В").range(8.40, 12.60, .02);
  }
  hub.show(tab == 1);
  {
    gh::Row r(hub);
    hub.Title(F("Настройки WiFi")).fontSize(18).size(8);
    gh::Flag wfModeFlag;
    hub.Switch(&cfg.WiFimode).noLabel().size(2).attach(&wfModeFlag);
    if (wfModeFlag) {
      DEBUG("Wifi Mode Changed ");
      DEBUGLN(cfg.WiFimode);
      cfgFile.updateNow();
      startWiFi();
    }
  }
  hub.show(tab == 1);   
  {
    gh::Row r(hub);
    gh::Flag ssidFlag;
    gh::Flag passFlag;
    hub.Input(cfg.ssid).label("Wifi ssid").attach(&ssidFlag);
    hub.Pass(cfg.pass).label("Wifi password").attach(&passFlag);
  }
  hub.show(tab == 1);    
  {
    gh::Row r(hub);
    hub.Title(F("Настройки MQTT")).fontSize(18).size(8);
    gh::Flag mqttFlag;
    hub.Switch(&cfg.mqtt).noLabel().noTab().size(2).attach(&mqttFlag);
    if (mqttFlag) {
      hub.refresh();
    }
  }
  hub.show(tab == 1); 
  {
    gh::Row r(hub);
    hub.Input(cfg.mqttHost).label("MQTT Host").maxLen(32).disabled(!cfg.mqtt);
    gh::Flag portFlag;
    hub.Input(&cfg.mqttPort).label("MQTT Port").maxLen(16).regex(GH_NUMBERS).disabled(!cfg.mqtt).attach(&portFlag);
    if (portFlag) { 
      if (cfg.mqttPort <= 1000) {
        cfg.mqttPort = 1001;
      }
      if (cfg.mqttPort > 65535) {
        cfg.mqttPort = 65535;
      }
      DEBUG("MQTT Port ");
      DEBUGLN(cfg.mqttPort);            
    }
  }
  hub.show(tab == 1);
  {
    gh::Row r(hub);
    hub.Input(cfg.mqttLogin).label("MQTT User").maxLen(16).disabled(!cfg.mqtt);
    hub.Pass(cfg.mqttPass).label("MQTT password").maxLen(16).disabled(!cfg.mqtt);
  }
  hub.show(tab == 1); 
  {
    gh::Row r(hub);
    hub.Title(F("Конфигурация лампы")).fontSize(18);
  }
  hub.show(tab == 1); 
  {
    gh::Row r(hub);
    static uint16_t maxCur = cfg.maxCur * 100;
    gh::Flag maxCurFlag;
    hub.Spinner(&maxCur).label("Максимальный ток (ма)").range(100, 5000, 100).size(5).attach(&maxCurFlag);
    if (maxCurFlag){
      cfg.maxCur = maxCur / 100;
    }
    static byte matrix = cfg.matrix - 1;
    gh::Flag matrixFlag;
    hub.Select(&matrix).label("Тип матрицы").text("LB_R;LB_U;LT_R;LT_D;RT_L;RT_D;RB_L;RB_U").size(5).attach(&matrixFlag);
    if (matrixFlag){
      cfg.matrix = matrix + 1;
    }
  }
  hub.show(tab == 1); 
  {
    gh::Row r(hub);
    static byte device = cfg.deviceType - 1;
    gh::Flag deviceFlag;
    hub.Select(&device).label("Тип лампы").text("Лента;Зигзаг;Параллель").size(4).attach(&deviceFlag);
    if (deviceFlag){
      cfg.deviceType = device + 1;
    }
    hub.Spinner(&cfg.width).label("Ширина").range(1, 32, 1).size(3);
    hub.Spinner(&cfg.length).label("Длина").range(1, 32, 1).size(3);
  }
  hub.show(tab == 1); 
  {
    gh::Row r(hub);
    hub.Title(F("Яркость и работа")).fontSize(18);
  }
  hub.show(tab == 1);
  {
    gh::Row r(hub);
    hub.Slider(&cfg.bright).label("Максимальная яркость").range(0, 255, 1).size(7);
    hub.Select(&cfg.adcMode).label("Режим работы").text("Не выбрано;Напряжение;Яркость").size(3);
  }
  hub.show(tab == 1); 
  {
    gh::Row r(hub);    
    gh::Flag saveFlag;
    hub.Space().size(8);
    hub.Button().label("Save").hint("Save settings").size(2).attach(&saveFlag);
    if (saveFlag){
      cfgFile.update();
      DEBUGLN("Settings Updated!");
    }
  }      
  static byte presetTab;
  hub.show(tab == 2);  
  {
    gh::Row r(hub);
    static mString<21 * MAX_PRESETS> tabs;
    tabs = "";
    FOR_i(0, cfg.presetAmount) {
      tabs += presets.presets[i].name;
      if (i < cfg.presetAmount - 1){
        tabs += ";";
      }
    }
    // DEBUGLN(tabs.c_str());
    if (hub.Tabs(&presetTab).text(tabs.c_str()).noLabel().size(8).click()) {
      hub.refresh();
    }
    gh::Flag addFlag;
    hub.Button().label("Add preset").size(2).attach(&addFlag);
    if(addFlag){
      cfg.presetAmount += 1;
      strcpy(presets.presets[cfg.presetAmount - 1].name, "NewPreset");
      presetsFile.update();
      cfgFile.update();
      hub.refresh();          
    } 
  }      
  FOR_i(0, cfg.presetAmount) {
    hub.show(tab == 2 && presetTab == i);
    {
      gh::Row r(hub);
      hub.Input(presets.presets[presetTab].name).label("Название пресета").maxLen(20);  
    }
    hub.show(tab == 2 && presetTab == i);
    {
      gh::Row r(hub);
      hub.Label("Эффект").size(5).fontSize(16).noLabel().align(gh::Align(0)).noTab();
      hub.Space().size(1);          
      hub.Select(&presets.presets[presetTab].effect).text("Не выбран;Перлин;Цвет;Смена;Градиент;Частицы;Костер;Огонь;Конфетти;Смерч").noLabel().size(4);
    }
    hub.show(tab == 2 && presetTab == i);
    {
      gh::Row r(hub);
      hub.Label("Уменьшить яркость").fontSize(16).size(5).noLabel().align(gh::Align(0)).noTab();
      hub.Space().size(3);
      gh::Flag fadeFlag;
      hub.Switch(&presets.presets[presetTab].fadeBright).noLabel().noTab().size(2).attach(&fadeFlag);
      if (fadeFlag) {
        hub.refresh();
      }          
    }
    hub.show(tab == 2 && presetTab == i);
    {
      gh::Row r(hub);
      hub.Slider(&presets.presets[presetTab].bright).label("Яркость").range(0, 255, 1).disabled(!presets.presets[presetTab].fadeBright);
    }
    hub.show(tab == 2 && presetTab == i);    
    {
      gh::Row r(hub);
      gh::Flag speedFlag;
      hub.Slider(&presets.presets[presetTab].speed).label("Скорость").range(0, 255, 1).attach(&speedFlag);
      if (speedFlag){
        loading = true;
      }
    }     
    hub.show(tab == 2 && presetTab == i);
    {
      gh::Row r(hub);
      gh::Flag scaleFlag;
      hub.Slider(&presets.presets[presetTab].scale).label("Масштаб").range(0, 255, 1).attach(&scaleFlag);
      if (scaleFlag){
        loading = true;
      }
    }
    hub.show(tab == 2 && presetTab == i);
    {
      gh::Row r(hub);
      hub.Label("Из центра").fontSize(16).size(5).noLabel().align(gh::Align(0)).noTab();
      hub.Space().size(3);
      hub.Switch(&presets.presets[presetTab].fromCenter).noLabel().noTab().size(2);        
    }
    hub.show(tab == 2 && presetTab == i);
    {
      gh::Row r(hub);
      hub.Slider(&presets.presets[presetTab].color).label("Цвет").range(0, 255, 1);
    }
    {
      gh::Row r(hub);
      hub.Label("Палитра").size(4).fontSize(16).noLabel().align(gh::Align(0)).noTab();
      gh::Flag fromPalFlag;
      hub.Switch(&presets.presets[presetTab].fromPal).noLabel().noTab().size(2).attach(&fromPalFlag);
      if (fromPalFlag){
        hub.refresh();
      }        
      hub.Select(&presets.presets[presetTab].palette).text("Не выбран;Своя;Тепло;Пламя;Рыжее пламя;Красное пламя;Пастель пламя;Желтое пламя;Зеленое пламя;Синее пламя;Лава;Вечеринка;Радуга;Павлин;Облака;Океан;Лес;Закат;Полиция;Пепси;Теплая лава;Холодная лава;Горячая лава;Розовая лава;Уютный;Киберпанк;Девчачья;Рождество;Кислота;Синий дым;Жвачка;Леопард;Аврора").noLabel().size(4).disabled(!presets.presets[presetTab].fromPal);
    }
    hub.show(tab == 2 && presetTab == i);
    {
      gh::Row r(hub);
      gh::Flag saveFlag;          
      hub.Space().size(8);
      hub.Button().label("Save").hint("Save preset").size(2).attach(&saveFlag);
      if (saveFlag){
        presetsFile.update();
        hub.refresh();
        DEBUGLN("Presets Updated!");
      }
    }
  }
  
}