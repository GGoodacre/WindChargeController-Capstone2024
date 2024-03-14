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
  #define R1_AC_THREEPHASE  1
  #define R2_AC_THREEPHASE  2
  #define R3_AC_THREEPHASE  42
  #define R4_DC_RECTIFIED   41
  #define R7_DC_BUCK        40
  #define R8_DC_BATTERY     39
  #define R9_DC_DUMP        38
#endif

#ifndef POWERELECPINS
    #define RECTIFIER_PIN   19
    #define SEPIC_PIN       44
    #define PS1_PIN        19
    #define PS2_PIN        20
#endif

#ifndef PWM_PARAMETERS
  #define RECTIFIER_INDEX   LEDC_CHANNEL_0
  #define SEPIC_INDEX       LEDC_CHANNEL_1
  #define PS1_INDEX        LEDC_CHANNEL_3
  #define PS2_INDEX        LEDC_CHANNEL_4
  #define TOTAL_PWM_VALUES 4
  #define PWM_FREQUENCY     300000
#endif

#define MAX_CURRENT 10
#define R_SHUNT 0.01

#define SEPIC_SETPOINT 12.4
#define RECTIFIER_SETPOINT 300
#define PS1_SETPOINT 300

#define SEPIC_Kp 5
#define SEPIC_Ki 0
#define SEPIC_Kd 0

#define RECTIFIER_Kp 5
#define RECTIFIER_Ki 0
#define RECTIFIER_Kd 0

#define PS1_Kp 5
#define PS1_Ki 0
#define PS1_Kd 0

class HardwareControl : public Control<TOTAL_PWM_VALUES, TOTAL_MEASUREMENT_DEVICES> {
    public:
        HardwareControl();
        ~HardwareControl();
    private:
        
        void PID_control();
};

