#pragma once
#include "Control.hpp"
#include "QuickPID.h"
#include <array>


#define PS_TOTAL_MEASUREMENT_DEVICES 1

enum PS_PWM_INDEX {
    PWM_POWERSUPPLY
};

enum PS_DEVICE_INDEX {
    DEVICEINDEX_POWERSUPPLY,
};

#ifndef CS_PINS
  #define POWERSUPPLY_DEVICE_PIN  10
#endif

#ifndef POWERELECPINS
    #define POWERSUPPLY_PWM_PIN  15
#endif

#ifndef PWM_PARAMETERS
  #define POWERSUPPLY_INDEX   LEDC_CHANNEL_5
  #define PS_TOTAL_PWM_VALUES 1
  #define PS_PWM_FREQUENCY    1000
#endif

#define PS_MAX_CURRENT 100
#define PS_R_SHUNT 0.0001

#define POWERSUPPLY_SETPOINT 15

#define POWERSUPPLY_Kp 5
#define POWERSUPPLY_Ki 0
#define POWERSUPPLY_Kd 0

class PowerSupplyControl : public Control<PS_TOTAL_PWM_VALUES, PS_TOTAL_MEASUREMENT_DEVICES> {
    public:
        PowerSupplyControl();
        ~PowerSupplyControl();

        void setCurrent(float current) { _pwm_values[PWM_POWERSUPPLY].setpoint = current; };
    private:
        
        void PID_control();
};

