#include <lvgl.h>
#include <M5Dial.h>
#include "M5Dial-LVGL.h"
#include <Arduino.h>
#include "../lib/M5Dial-LVGL-Arduino/M5Dial-LVGL.h"
#include "ui.h"

void main_task(void *) {
    m5dial_lvgl_init();

    main_menu();

    vTaskDelete(nullptr);
}

void setup() {
  Serial.begin(9600);
  xTaskCreatePinnedToCore(main_task, "main_task", 8192, nullptr, 1, nullptr, 1);
}

void loop() {
  m5dial_lvgl_next();
}










