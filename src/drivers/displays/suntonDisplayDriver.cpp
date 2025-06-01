#include "displayDriver.h"

#ifdef SUNTON_DISPLAY

#define LGFX_USE_V1
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <LovyanGFX.hpp>
#include <lgfx_user/LGFX_Sunton_ESP32-8048S070.h>

#include "monitor.h"
#include "drivers/storage/storage.h"
#include "wManager.h"

extern monitor_data mMonitor;
extern TSettings Settings;

class LGFX : public lgfx::LGFX_Device
{
public:

  lgfx::Bus_RGB     _bus_instance;
  lgfx::Panel_RGB   _panel_instance;
  lgfx::Light_PWM   _light_instance;
  lgfx::Touch_GT911 _touch_instance;

  LGFX(void)
  {
    {
      auto cfg = _panel_instance.config();

      cfg.memory_width  = 800;
      cfg.memory_height = 480;
      cfg.panel_width  = 800;
      cfg.panel_height = 480;

      cfg.offset_x = 0;
      cfg.offset_y = 0;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _panel_instance.config_detail();

      cfg.use_psram = 1;

      _panel_instance.config_detail(cfg);
    }

    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;
      cfg.pin_d0  = GPIO_NUM_15;  // B0
      cfg.pin_d1  = GPIO_NUM_7;  // B1
      cfg.pin_d2  = GPIO_NUM_6; // B2
      cfg.pin_d3  = GPIO_NUM_5;  // B3
      cfg.pin_d4  = GPIO_NUM_4;  // B4
      cfg.pin_d5  = GPIO_NUM_9;  // G0
      cfg.pin_d6  = GPIO_NUM_46;  // G1
      cfg.pin_d7  = GPIO_NUM_3;  // G2
      cfg.pin_d8  = GPIO_NUM_8; // G3
      cfg.pin_d9  = GPIO_NUM_16; // G4
      cfg.pin_d10 = GPIO_NUM_1;  // G5
      cfg.pin_d11 = GPIO_NUM_14; // R0
      cfg.pin_d12 = GPIO_NUM_21; // R1
      cfg.pin_d13 = GPIO_NUM_47; // R2
      cfg.pin_d14 = GPIO_NUM_48; // R3
      cfg.pin_d15 = GPIO_NUM_45; // R4

      cfg.pin_henable = GPIO_NUM_41;
      cfg.pin_vsync   = GPIO_NUM_40;
      cfg.pin_hsync   = GPIO_NUM_39;
      cfg.pin_pclk    = GPIO_NUM_42;
      cfg.freq_write  = 12000000;

      cfg.hsync_polarity    = 0;
      cfg.hsync_front_porch = 8;
      cfg.hsync_pulse_width = 2;
      cfg.hsync_back_porch  = 43;
      cfg.vsync_polarity    = 0;
      cfg.vsync_front_porch = 8;
      cfg.vsync_pulse_width = 2;
      cfg.vsync_back_porch  = 12;
      cfg.pclk_idle_high    = 1;
      _bus_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = GPIO_NUM_2;
      _light_instance.config(cfg);
    }
    _panel_instance.light(&_light_instance);

    {
      auto cfg = _touch_instance.config();
      cfg.x_min      = 0;
      cfg.y_min      = 0;
      cfg.bus_shared = false;
      cfg.offset_rotation = 0;
      // I2C connection
      cfg.i2c_port   = I2C_NUM_0;
      cfg.pin_sda    = GPIO_NUM_19;
      cfg.pin_scl    = GPIO_NUM_20;
      cfg.pin_int    = GPIO_NUM_NC;
      cfg.pin_rst    = GPIO_NUM_38;
      cfg.x_max      = 800;
      cfg.y_max      = 480;
      cfg.freq       = 100000;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};
void suntonDisplay_AlternateScreenState(void)
{
}

void suntonDisplay_AlternateRotation(void)
{
}

static unsigned long ulTime = millis() - 100000;

void suntonDisplay_NoScreen(unsigned long mElapsed)
{
  mining_data data = getMiningData(mElapsed);

  // Print hashrate to serial
  Serial.printf(">>> Completed %s share(s), %s Khashes, avg. hashrate %s KH/s\n",
                data.completedShares.c_str(), data.totalKHashes.c_str(), data.currentHashRate.c_str());
  //Serial.printf(">>> Temperature: %s\n", data.temp.c_str());

  lv_label_set_text(ui_lblhashrate, data.currentHashRate.c_str());
  lv_bar_set_value(ui_barhashrate, data.currentHashRate.toInt(), LV_ANIM_ON);
  lv_label_set_text(ui_lblvalid, data.valids.c_str());
  lv_label_set_text(ui_lbltemplates, data.templates.c_str());
  lv_label_set_text(ui_lbltotalhashrate, data.totalKHashes.c_str());
  lv_label_set_text(ui_lblbestdiff, data.bestDiff.c_str());
  lv_label_set_text(ui_lblshares32, data.completedShares.c_str());
  lv_label_set_text(ui_lblclock, data.timeMining.c_str());
  lv_label_set_text(ui_lbltemperature, data.temp.c_str());

  lv_label_set_text(ui_lblclock2, data.currentTime.c_str());

  lv_label_set_text(ui_lblIp, WiFi.localIP().toString().c_str());
  lv_label_set_text(ui_lblAddress, String(Settings.BtcWallet).c_str());

  if(millis() - ulTime > 1000 * 60) {
    ulTime = millis();
  
    coin_data cdata = getCoinData(mElapsed);

    lv_label_set_text(ui_lblPrice, cdata.btcPrice.c_str());
    lv_label_set_text(ui_lblGlobalHashrate, cdata.globalHashRate.c_str());
    lv_label_set_text(ui_lblDifficulty, cdata.netwrokDifficulty.c_str());
    lv_bar_set_value(ui_barhalving, cdata.progressPercent, LV_ANIM_ON);
    lv_label_set_text(ui_lblHeight2, cdata.blockHeight.c_str());

    pool_data pdata = getPoolData();

    lv_label_set_text(ui_lblWorkers, String(pdata.workersCount).c_str());
    lv_label_set_text(ui_lblMaxDifficulty, pdata.bestDifficulty.c_str());
    lv_label_set_text(ui_lblTotHashrate, pdata.workersHash.c_str());
  }
}

void suntonDisplay_LoadingScreen(void)
{
  Serial.println("Initializing...");
  Serial.print("Firmware Version: ");
  Serial.println(AUTO_VERSION);
  lv_label_set_text(ui_lblssid, "HanSoloAP");
  lv_label_set_text(ui_lblpassword, "MineYourCoins");
  lv_label_set_text(ui_lblversion, AUTO_VERSION);
  lv_label_set_text(ui_lblversion2, AUTO_VERSION);

  lv_label_set_text(ui_lblPool, (String(Settings.PoolAddress)+":"+String(Settings.PoolPort)).c_str());

  _ui_screen_change(&ui_HomeScreen, LV_SCR_LOAD_ANIM_FADE_ON, 2000, 0, &ui_HomeScreen_screen_init);
}

void suntonDisplay_SetupScreen(void)
{
  Serial.println("Setup...");
}

void suntonDisplay_DoLedStuff(unsigned long frame)
{
  // we will use led function to update lvgl
  lv_timer_handler();
}

void suntonDisplay_AnimateCurrentScreen(unsigned long frame)
{
}

CyclicScreenFunction suntonDisplayCyclicScreens[] = {suntonDisplay_NoScreen};

DisplayDriver suntonDisplayDriver{
    suntonDisplay_Init,
    suntonDisplay_AlternateScreenState,
    suntonDisplay_AlternateRotation,
    suntonDisplay_LoadingScreen,
    suntonDisplay_SetupScreen,
    suntonDisplayCyclicScreens,
    suntonDisplay_AnimateCurrentScreen,
    suntonDisplay_DoLedStuff,
    SCREENS_ARRAY_SIZE(suntonDisplayCyclicScreens),
    0,
    WIDTH,
    HEIGHT};
#endif