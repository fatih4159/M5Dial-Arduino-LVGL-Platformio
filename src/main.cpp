#include "main.h"


void main_task(void *)
{

  m5dial_lvgl_init();
  M5Dial.Display.setBrightness(50);
  M5Dial.Display.setRotation(2);

  ui_init();

  vTaskDelete(nullptr);
}


void setup()
{
  Serial.begin(9600);
  xTaskCreatePinnedToCore(main_task, "main_task", 1200*40, nullptr, 1, nullptr, 1);
}
 bool isConnectedChanged = false;
void loop()
{
  m5dial_lvgl_next();
}
