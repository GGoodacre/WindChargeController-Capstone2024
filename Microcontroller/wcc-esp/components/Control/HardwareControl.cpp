#include <stdio.h>
#include "HardwareControl.hpp"

HardwareControl::HardwareControl() : 
    Control(LEDC_TIMER_8_BIT, PWM_FREQUENCY, LEDC_TIMER_0, MAX_CURRENT, R_SHUNT)
{
    _algorithm = MANUAL;
    _sepic_pwm_setpoint = 0;

    _pwm_values[PWM_RECTIFIER].duty_cycle = 100;
    _pwm_values[PWM_RECTIFIER].channel = RECTIFIER_INDEX;
    _pwm_values[PWM_RECTIFIER].device_pin = RECTIFIER_PIN;
    _pwm_values[PWM_RECTIFIER].invert = 1;
    _pwm_values[PWM_RECTIFIER].setpoint = RECTIFIER_SETPOINT;
    _pwm_values[PWM_RECTIFIER].pid = QuickPID(&_rectifier_input, &_pwm_values[PWM_RECTIFIER].duty_cycle, &_pwm_values[PWM_RECTIFIER].setpoint);
    _pwm_values[PWM_RECTIFIER].pid.SetTunings(RECTIFIER_Kp, RECTIFIER_Ki, RECTIFIER_Kd);
    _pwm_values[PWM_RECTIFIER].pid.SetOutputLimits(0, 100);
    _pwm_values[PWM_RECTIFIER].pid.SetMode(1);
    _pwm_values[PWM_RECTIFIER].pid.SetSampleTimeUs(1000);

    _pwm_values[PWM_SEPIC].duty_cycle = 30;
    _pwm_values[PWM_SEPIC].channel = SEPIC_INDEX;
    _pwm_values[PWM_SEPIC].device_pin = SEPIC_PIN;
    _pwm_values[PWM_SEPIC].invert = 0;
    _pwm_values[PWM_SEPIC].setpoint = SEPIC_SETPOINT;
    _pwm_values[PWM_SEPIC].pid = QuickPID(&_sepic_pwm_input, &_pwm_values[PWM_SEPIC].duty_cycle, &_sepic_pwm_setpoint);
    _pwm_values[PWM_SEPIC].pid.SetTunings(SEPIC_Kp, SEPIC_Ki, SEPIC_Kd);
    _pwm_values[PWM_SEPIC].pid.SetMode(1);
    _pwm_values[PWM_SEPIC].pid.SetOutputLimits(20, 80);
    _pwm_values[PWM_SEPIC].pid.SetSampleTimeUs(1000);

    _pwm_values[PWM_PS1].duty_cycle = 100;
    _pwm_values[PWM_PS1].channel = PS1_INDEX;
    _pwm_values[PWM_PS1].device_pin = PS1_PIN;
    _pwm_values[PWM_PS1].invert = 0;
    _pwm_values[PWM_PS1].pid = QuickPID(&_devices[DEVICEINDEX_R8_BATTERY].power, &_pwm_values[PWM_PS1].duty_cycle, &_pwm_values[PWM_PS1].setpoint);
    _pwm_values[PWM_PS1].pid.SetTunings(PS1_Kp, PS1_Ki, PS1_Kd);
    _pwm_values[PWM_PS1].pid.SetMode(1);

    _pwm_values[PWM_PS2].duty_cycle = 0;
    _pwm_values[PWM_PS2].channel = PS2_INDEX;
    _pwm_values[PWM_PS2].device_pin = PS2_PIN;
    _pwm_values[PWM_PS2].invert = 0;

    _devices[DEVICEINDEX_R1_THREEPHASE].en = false;
    _devices[DEVICEINDEX_R1_THREEPHASE].en_measurement = {0, 1, 0, 1, 1, 0, 0};
    _devices[DEVICEINDEX_R1_THREEPHASE].device_pin = R1_AC_THREEPHASE;

    _devices[DEVICEINDEX_R2_THREEPHASE].en = false;
    _devices[DEVICEINDEX_R2_THREEPHASE].en_measurement = {0, 1, 0, 1, 1, 0, 0};
    _devices[DEVICEINDEX_R2_THREEPHASE].device_pin = R2_AC_THREEPHASE;

    _devices[DEVICEINDEX_R3_THREEPHASE].en = false;
    _devices[DEVICEINDEX_R3_THREEPHASE].en_measurement = {0, 1, 0, 1, 1, 0, 0};
    _devices[DEVICEINDEX_R3_THREEPHASE].device_pin = R3_AC_THREEPHASE;

    _devices[DEVICEINDEX_R4_RECTIFIED].en = true;
    _devices[DEVICEINDEX_R4_RECTIFIED].en_measurement = {0, 1, 0, 1, 1, 0, 0};
    _devices[DEVICEINDEX_R4_RECTIFIED].device_pin = R4_DC_RECTIFIED;

    _devices[DEVICEINDEX_R7_BUCK].en = true;
    _devices[DEVICEINDEX_R7_BUCK].en_measurement = {0, 1, 0, 1, 1, 0, 0};
    _devices[DEVICEINDEX_R7_BUCK].device_pin = R7_DC_BUCK;

    _devices[DEVICEINDEX_R8_BATTERY].en = false;
    _devices[DEVICEINDEX_R8_BATTERY].en_measurement = {0, 1, 0, 1, 1, 0, 0};
    _devices[DEVICEINDEX_R8_BATTERY].device_pin = R8_DC_BATTERY;

    _devices[DEVICEINDEX_R9_DUMP].en = false;
    _devices[DEVICEINDEX_R9_DUMP].en_measurement = {0, 1, 0, 1, 1, 0, 0};
    _devices[DEVICEINDEX_R9_DUMP].device_pin = R9_DC_DUMP;

    pinMode(HIGHSIDE_RECTIFIER_PIN, OUTPUT);
    digitalWrite(HIGHSIDE_RECTIFIER_PIN, LOW);

    config();
}

HardwareControl::~HardwareControl()
{
}

void HardwareControl::PID_control()
{   
    _rectifier_input = _devices[DEVICEINDEX_R4_RECTIFIED].vbus;
    _sepic_pwm_input = _pwm_values[PWM_SEPIC].duty_cycle/100;
    _sepic_pwm_setpoint = (SEPIC_SETPOINT + diode_drop) / (SEPIC_SETPOINT + diode_drop + _devices[DEVICEINDEX_R4_RECTIFIED].vbus);
    ESP_LOGI(SEPIC_TAG, "Rectified VBUS: %f   Sepic VBUS: %f   Desired PWM:  %f   Current PWM: %f", _devices[DEVICEINDEX_R4_RECTIFIED].vbus, _devices[DEVICEINDEX_R7_BUCK].vbus, _sepic_pwm_setpoint, _pwm_values[PWM_SEPIC].duty_cycle);
    ESP_LOGI(RECTIFIER_TAG, "Rectified VBUS: %f   Current PWM: %f", _devices[DEVICEINDEX_R4_RECTIFIED].vbus, _pwm_values[PWM_RECTIFIER].duty_cycle);

    if(_pwm_values[PWM_SEPIC].pid.Compute()) _pwm.setDUTY(_pwm_values[PWM_SEPIC].duty_cycle, _pwm_values[PWM_SEPIC].channel);
    if(_pwm_values[PWM_RECTIFIER].pid.Compute()) _pwm.setDUTY(_pwm_values[PWM_RECTIFIER].duty_cycle, _pwm_values[PWM_RECTIFIER].channel);
}
