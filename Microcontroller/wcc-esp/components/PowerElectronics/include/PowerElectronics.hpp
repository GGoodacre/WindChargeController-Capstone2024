#include "Arduino.h"
//#include <ESP32_FastPWM.h>
#include "driver/ledc.h"
#include <array>

class PowerElectronics {
  public: 
    PowerElectronics(ledc_timer_bit_t resolution, int freq, ledc_timer_t timer);
    bool config(ledc_channel_t channel, int pin, int invert = 1);
    void setDUTY(float duty_cycle, ledc_channel_t index);

  private:

    ledc_timer_config_t timer_config;

    const ledc_timer_bit_t RESOLUTION;
    const int MAX_DUTY; 
    const int FREQ;  
    const ledc_timer_t TIMER;
};

