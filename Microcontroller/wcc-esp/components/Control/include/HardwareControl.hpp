#pragma once
#include "Control.hpp"
#include "QuickPID.h"
#include <array>

enum HARDWARE_ALGORITHMS {
    MPPT,
    MCPT
};

#define TOTAL_MEASUREMENT_DEVICES 7

enum PWM_INDEX {
    PWM_RECTIFIER,
    PWM_SEPIC,
    PWM_PS1,
    PWM_PS2
};

enum DEVICE_INDEX {
    DEVICEINDEX_R1_THREEPHASE,
    DEVICEINDEX_R2_THREEPHASE,
    DEVICEINDEX_R3_THREEPHASE,
    DEVICEINDEX_R4_RECTIFIED,
    DEVICEINDEX_R7_BUCK,
    DEVICEINDEX_R8_BATTERY,
    DEVICEINDEX_R9_DUMP,
};

#ifndef CS_PINS
  #define R1_AC_THREEPHASE  39
  #define R2_AC_THREEPHASE  38
  #define R3_AC_THREEPHASE  37
  #define R4_DC_RECTIFIED   2
  #define R7_DC_BUCK        42
  #define R8_DC_BATTERY     41
  #define R9_DC_DUMP        40
#endif

#ifndef LED_PINS
    #define LED_1 16
    #define LED_2 15
    #define LED_3 7
    #define LED_4 6
    #define LED_5 5
    #define LED_6 4
#endif

#ifndef POWERELECPINS
    //#define RECTIFIER_PIN   33
    #define RECTIFIER_PIN 12
    #define HIGHSIDE_RECTIFIER_PIN 34
    #define SEPIC_PIN       1
    #define PS1_PIN        19
    #define PS2_PIN        20
#endif

#ifndef PWM_PARAMETERS
  #define RECTIFIER_INDEX   LEDC_CHANNEL_0
  #define SEPIC_INDEX       LEDC_CHANNEL_1
  #define PS1_INDEX        LEDC_CHANNEL_3
  #define PS2_INDEX        LEDC_CHANNEL_4
  #define TOTAL_PWM_VALUES 4
  #define PWM_FREQUENCY     10000
#endif

#define MAX_CURRENT 10
#define R_SHUNT 0.01

#define diode_drop 1.4
#define SEPIC_SETPOINT 2
#define RECTIFIER_SETPOINT 70
#define PS1_SETPOINT 1000

#define SEPIC_Kp 1000
#define SEPIC_Ki 600
#define SEPIC_Kd 0

#define RECTIFIER_Kp -20
#define RECTIFIER_Ki 20
#define RECTIFIER_Kd 0

#define PS1_Kp 0
#define PS1_Ki 0
#define PS1_Kd 0

class HardwareControl : public Control<TOTAL_PWM_VALUES, TOTAL_MEASUREMENT_DEVICES> {
    public:
        HardwareControl();
        ~HardwareControl();
    private:
        
        void PID_control();
        float _sepic_pwm_cv;
};

