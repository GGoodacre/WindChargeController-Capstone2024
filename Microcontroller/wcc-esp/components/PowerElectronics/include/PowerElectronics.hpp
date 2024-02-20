#include "Arduino.h"
#include <ESP32_FastPWM.h>
#include <array>


#ifndef POWERELECPINS
    #define NUMBER_OF_PINS  3
    #define RECTIFIER_PIN   12
    #define SEPIC_PIN       13
    #define LOAD_PIN        14
#endif

#ifndef PWM_PARAMETERS
  #define RESOLUTION LEDC_TIMER_8_BIT
  #define MAX_DUTY  (1 << RESOLUTION) - 1
  #define FREQ         300000
  #define RECTIFIER_INDEX   0
  #define SEPIC_INDEX       1
  #define LOAD_INDEX        2
#endif

class PowerElectronics {
  public: 
    PowerElectronics();
    void setDUTY(int duty_cycle, int index);

  private:

    std::array<ledc_channel_config_t, NUMBER_OF_PINS> channel_config;
    ledc_timer_config_t timer_config;

};

