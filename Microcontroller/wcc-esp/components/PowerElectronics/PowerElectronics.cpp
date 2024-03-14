#include <stdio.h>
#include "PowerElectronics.hpp"
#include "Arduino.h"
#include <array>
#include "driver/ledc.h"


PowerElectronics::PowerElectronics(ledc_timer_bit_t resolution, int freq, ledc_timer_t timer) : 
  RESOLUTION    (resolution),
  MAX_DUTY      ((1 << RESOLUTION) - 1),
  FREQ          (freq),
  TIMER         (timer)
{

    timer_config.duty_resolution = RESOLUTION;
    timer_config.freq_hz = FREQ;
    timer_config.timer_num = TIMER;
    timer_config.speed_mode = LEDC_LOW_SPEED_MODE;
    timer_config.clk_cfg = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

}

bool PowerElectronics::config(ledc_channel_t channel, int pin, int invert)
{
    ledc_channel_config_t channel_config;
    channel_config.speed_mode     = LEDC_LOW_SPEED_MODE;
    channel_config.channel        = channel;
    channel_config.timer_sel      = TIMER;
    channel_config.intr_type      = LEDC_INTR_DISABLE;
    channel_config.gpio_num       = pin;
    channel_config.duty           = 0;
    channel_config.hpoint         = 0;
    channel_config.flags.output_invert = invert;
    
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));

    return true;
}

void PowerElectronics::setDUTY(float duty_cycle, ledc_channel_t index)
{
    uint32_t duty = (((int)MAX_DUTY) * duty_cycle) / 100;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, index, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, index));
}
