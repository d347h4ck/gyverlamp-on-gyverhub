#if __cplusplus > 199711L
    #define register
#endif

// ------------ Кнопка -------------
#define BTN_PIN 0           // пин кнопки GPIO0 (D3 на wemos/node), 0 для схемы с ESP-01
#define USE_BTN 0           // 1 использовать кнопку, 0 нет

// ------------ Лента -------------
#define STRIP_PIN 2         // пин ленты GPIO2 (D4 на wemos/node), GPIO5 (D1) для module
#define MAX_LEDS 300        // макс. светодиодов
#define STRIP_CHIP WS2812   // чип ленты
#define STRIP_COLOR GRB     // порядок цветов в ленте
#define STRIP_VOLT 5        // напряжение ленты, V
/*
  WS2811, GBR, 12V
  WS2812, GRB, 5V
  WS2813, GRB, 5V
  WS2815, GRB, 12V
  WS2818, RGB, 12V
*/

// ------------ Датчик напряжения --
#define USE_INA 1
// ------------ WiFi AP ------------
const char AP_NameChar[] = "GyverLamp3";
const char WiFiPassword[] = "12345678";

// ------------ Прочее -------------
#define GL_VERSION 30       // код версии прошивки
#define EE_TOUT 30000       // таймаут сохранения епром после изменения, мс
#define DEBUG_SERIAL_LAMP   // закомментируй чтобы выключить отладку (скорость 115200)
#define NTP_UPD_PRD 5       // период обновления времени с NTP сервера, минут

// ------------ GyverHub -----------
#define GH_INCLUDE_PORTAL

// ------------- АЦП --------------
#define USE_ADC 0           // можно выпилить АЦП
#define PHOT_VCC 14         // питание фоторезистора GPIO14 (D5 на wemos/node)

// ------------ Библиотеки ---------
#include "data.h"         // данные
#include "palettes.h"     // палитры
#include <GyverHub.h>     // GyverHub
#include <FastLED.h>      // лента
#include <GyverTimer.h>   // Таймеры
#include <ESP8266WiFi.h>  // базовая либа есп
#include <FileData.h>     // Аналог EEpRom
#include <LittleFS.h>     // Встроенная ФС
#include <StringUtils.h>  // Строки
#include <mString.h>      // Строки x2
#include <GyverNTP.h>     // Время с сервера
#include "Time.h"         // Время
#include <GyverFilters.h> // Фильтры
#include <GyverButton.h>  // кнопка
#include <GyverINA.h>     // Датчик напряжения
// ------------ Данные -------------

Config cfg;
Presets presets;
FileData cfgFile(&LittleFS, "/cfg.dat", 'B', &cfg, CFG_SIZE);
FileData presetsFile(&LittleFS, "/presets.dat", 'C', &presets, sizeof(presets));
CRGB leds[MAX_LEDS];
GyverHub hub("MyDevices", "GyverLamp 3.0");
GTimer connTmr(MS), effTmr(MS), timeTmr(MS), adcTmr(MS), inaTmr(MS);
GyverNTP ntp(3);
GButton btn(BTN_PIN);
INA219 ina;
byte ina_connected = 0;
byte btnClicks = 0;
Time now;
double curVoltage = 0, curCurrent = 0;
Palette pal;
bool loading;
unsigned char matrixValue[11][16];

FastFilter phot;
FastFilter volt;

void setup(){
  memset(matrixValue, 0, sizeof(matrixValue));
  delay(2000);          // ждём старта есп
  #ifdef DEBUG_SERIAL_LAMP
    Serial.begin(115200);
    DEBUGLN();
  #endif
  #if (USE_INA == 1)
  if (ina.begin()) {
    Serial.println(F("connected!"));
    ina_connected = 1;
    ina.setResolution(INA219_VBUS, INA219_RES_12BIT_X32);
  } else {
    Serial.println(F("not found!"));
    ina_connected = 0;
  }
  #endif
  startStrip();         // старт ленты
  LittleFS.begin();
  connTmr.setTimeout(120000ul);

  FDstat_t stat = cfgFile.read();

  switch (stat) {
    case FD_FS_ERR: DEBUGLN("FS Error");
      break;
    case FD_FILE_ERR: DEBUGLN("Error");
      break;
    case FD_WRITE: DEBUGLN("Config Write");
      break;
    case FD_ADD: DEBUGLN("Config Add");
      break;
    case FD_READ: DEBUGLN("Config Read");
      break;
    default:
      break;
  }

  stat = presetsFile.read();
  switch (stat) {
    case FD_FS_ERR: DEBUGLN("FS Error");
      break;
    case FD_FILE_ERR: DEBUGLN("Error");
      break;
    case FD_WRITE: DEBUGLN("Presets Write");
      break;
    case FD_ADD: DEBUGLN("Presets Add");
      break;
    case FD_READ: DEBUGLN("Presets Read");
      break;
    default:
      break;
  }

  startWiFi();
  if (cfg.mqtt){
    hub.mqtt.config(cfg.mqttHost, cfg.mqttPort, cfg.mqttLogin, cfg.mqttPass);
  }
  ntp.asyncMode(false);
  ntp.begin();
  hub.onBuild(build); // подключаем билдер
  hub.begin();        // запускаем систему
  
  timeTmr.setInterval(30);
  effTmr.setInterval(30);
  adcTmr.setInterval(2000);
  inaTmr.setInterval(5000);  
  hub.onReboot([](gh::Reboot r) {
    cfgFile.updateNow();
    presetsFile.updateNow();    
    DEBUGLN("Restart!");
    hub.sendCLI("Restart!");
  });
  hub.onCLI(adjust);
}

void loop(){
  // static gh::Timer tmr(5000);  // период 5 секунд
  tryReconnect();
  hub.tick();
  yield();
  cfgFile.tick();
  presetsFile.tick();
  yield();
  ntp.tick();
  timeTicker();
  effectsRoutine();
  yield();
  button();           // проверяем кнопку
  // Serial.print(F("Ina: "));
  // Serial.println(ina_connected);
  if (inaTmr.isReady()){
    if (ina_connected) {
      curVoltage = ina.getVoltage();
      hub.sendCLI("Voltage: ");
      hub.sendCLI(sutil::AnyValue(curVoltage));
      curCurrent = ina.getCurrent();
      hub.sendCLI("Cal: ");
      hub.sendCLI(sutil::AnyValue(ina.getCalibration()));     
    }
  }
  yield();
}