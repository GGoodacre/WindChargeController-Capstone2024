#include <stdio.h>
#include "PowerElectronics.hpp"
#include "Arduino.h"
#include <array>
#include "driver/ledc.h"


PowerElectronics::PowerElectronics()
{

    timer_config.duty_resolution = RESOLUTION;
    timer_config.freq_hz = FREQ;
    timer_config.timer_num = LEDC_TIMER_0;
    timer_config.speed_mode = LEDC_LOW_SPEED_MODE;
    timer_config.clk_cfg = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    channel_config[RECTIFIER_INDEX].speed_mode     = LEDC_LOW_SPEED_MODE;
    channel_config[RECTIFIER_INDEX].channel        = LEDC_CHANNEL_0;
    channel_config[RECTIFIER_INDEX].timer_sel      = LEDC_TIMER_0;
    channel_config[RECTIFIER_INDEX].intr_type      = LEDC_INTR_DISABLE;
    channel_config[RECTIFIER_INDEX].gpio_num       = RECTIFIER_PIN;
    channel_config[RECTIFIER_INDEX].duty           = 0;
    channel_config[RECTIFIER_INDEX].hpoint         = 0;

    channel_config[SEPIC_INDEX].speed_mode     = LEDC_LOW_SPEED_MODE;
    channel_config[SEPIC_INDEX].channel        = LEDC_CHANNEL_1;
    channel_config[SEPIC_INDEX].timer_sel      = LEDC_TIMER_0;
    channel_config[SEPIC_INDEX].intr_type      = LEDC_INTR_DISABLE;
    channel_config[SEPIC_INDEX].gpio_num       = SEPIC_PIN;
    channel_config[SEPIC_INDEX].duty           = 0;
    channel_config[SEPIC_INDEX].hpoint         = 0;

    channel_config[PS1_INDEX].speed_mode     = LEDC_LOW_SPEED_MODE;
    channel_config[PS1_INDEX].channel        = LEDC_CHANNEL_2;
    channel_config[PS1_INDEX].timer_sel      = LEDC_TIMER_0;
    channel_config[PS1_INDEX].intr_type      = LEDC_INTR_DISABLE;
    channel_config[PS1_INDEX].gpio_num       = PS1_PIN;
    channel_config[PS1_INDEX].duty           = 0;
    channel_config[PS1_INDEX].hpoint         = 0;

    channel_config[PS2_INDEX].speed_mode     = LEDC_LOW_SPEED_MODE;
    channel_config[PS2_INDEX].channel        = LEDC_CHANNEL_3;
    channel_config[PS2_INDEX].timer_sel      = LEDC_TIMER_0;
    channel_config[PS2_INDEX].intr_type      = LEDC_INTR_DISABLE;
    channel_config[PS2_INDEX].gpio_num       = PS2_PIN;
    channel_config[PS2_INDEX].duty           = 0;
    channel_config[PS2_INDEX].hpoint         = 0;
    channel_config[PS2_INDEX].flags.output_invert = 0;

    ESP_ERROR_CHECK(ledc_channel_config(&channel_config[RECTIFIER_INDEX]));
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config[SEPIC_INDEX]));
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config[PS1_INDEX]));
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config[PS2_INDEX]));

}

void PowerElectronics::setDUTY(int duty_cycle, int index)
{
    uint32_t duty = (((int)MAX_DUTY) * duty_cycle) / 100;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, ledc_channel_t(index), duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, ledc_channel_t(index)));
}
