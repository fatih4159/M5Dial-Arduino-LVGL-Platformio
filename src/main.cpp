#include "main.h"


void main_task(void *)
{

  m5dial_lvgl_init();

  main_menu();

  vTaskDelete(nullptr);
}


void setup()
{
  Serial.begin(9600);
  xTaskCreatePinnedToCore(main_task, "main_task", 8192, nullptr, 1, nullptr, 1);
}
 bool isConnectedChanged = false;
void loop()
{
  m5dial_lvgl_next();
}
