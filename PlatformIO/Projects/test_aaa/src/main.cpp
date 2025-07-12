#include <M5Core2.h>

#define LGFX_M5STACK_CORE2
#include <LovyanGFX.hpp>

static LGFX Lcd;

void setup()
{
  M5.begin(true, true, true, true);

  Lcd.init();
  Lcd.setFont(&fonts::lgfxJapanGothic_28);
  Lcd.setBrightness(128);
  Lcd.setCursor(0, 128);
  Lcd.println("はろーテスト壱");
}

void loop()
{
  M5.update();

  float batVol = M5.Axp.GetBatVoltage();
  float batCur = M5.Axp.GetBatCurrent();

  Lcd.setCursor(0, 156);
  Lcd.printf("Baterry: %1.3f\nCurrent: %.3f\n", batVol, batCur);

  if (batCur < 0) {
    delay(500);
    Lcd.setBrightness(0);
    Lcd.sleep();
    delay(500);
    Lcd.wakeup();
    Lcd.setBrightness(128);
  }
}
