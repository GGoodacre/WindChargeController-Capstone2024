#include <stdio.h>
#include "PowerSupplyControl.hpp"

PowerSupplyControl::PowerSupplyControl() : 
    Control(LEDC_TIMER_10_BIT, PS_PWM_FREQUENCY, LEDC_TIMER_0, PS_MAX_CURRENT, PS_R_SHUNT)
{
    _algorithm = MANUAL;

    _pwm_values[PWM_POWERSUPPLY].duty_cycle = 70;
    _pwm_values[PWM_POWERSUPPLY].channel = POWERSUPPLY_INDEX;
    _pwm_values[PWM_POWERSUPPLY].device_pin = POWERSUPPLY_PWM_PIN;
    _pwm_values[PWM_POWERSUPPLY].invert = 1;
    _pwm_values[PWM_POWERSUPPLY].setpoint = POWERSUPPLY_SETPOINT;
    _pwm_values[PWM_POWERSUPPLY].pid = QuickPID(&_devices[DEVICEINDEX_POWERSUPPLY].current, &_pwm_values[PWM_POWERSUPPLY].duty_cycle, &_pwm_values[PWM_POWERSUPPLY].setpoint);
    _pwm_values[PWM_POWERSUPPLY].pid.SetTunings(POWERSUPPLY_Kp, POWERSUPPLY_Ki, POWERSUPPLY_Kd);
    _pwm_values[PWM_POWERSUPPLY].pid.SetMode(1);
    _pwm_values[PWM_POWERSUPPLY].pid.SetOutputLimits(0, 100);
    _pwm_values[PWM_POWERSUPPLY].pid.SetSampleTimeUs(100000);
    

    _devices[DEVICEINDEX_POWERSUPPLY].en = true;
    _devices[DEVICEINDEX_POWERSUPPLY].en_measurement = {1, 1, 1, 1, 0, 0, 0};
    _devices[DEVICEINDEX_POWERSUPPLY].device_pin = POWERSUPPLY_DEVICE_PIN;

    config();
}

PowerSupplyControl::~PowerSupplyControl()
{
}

void PowerSupplyControl::PID_control()
{ 
    if(_pwm_values[PWM_POWERSUPPLY].pid.Compute())
    {
        
        for(int i = 0; i < PS_TOTAL_PWM_VALUES; i++)
        {
            ESP_LOGI(CONTROL_TAG, "Duty_Cycle:%f", _pwm_values[i].duty_cycle);
            _pwm.setDUTY(_pwm_values[i].duty_cycle, _pwm_values[i].channel);
        }
    }
}
