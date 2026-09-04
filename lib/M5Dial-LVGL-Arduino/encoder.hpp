#pragma once

#include <driver/pcnt.h>
#include <driver/gpio.h>

class Encoder
{
private:
    gpio_num_t _pin_a;
    gpio_num_t _pin_b;
    pcnt_unit_t _pcnt_unit;

public:
    Encoder();
    ~Encoder();
    // M5Stack M5 Dial: encoder A = GPIO41, encoder B = GPIO40.
    void setup(gpio_num_t pin_a = GPIO_NUM_41, gpio_num_t pin_b = GPIO_NUM_40);
    int getCount(bool clear = false);
    void reset();
};
