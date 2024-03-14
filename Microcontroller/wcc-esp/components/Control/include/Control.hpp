#pragma once
#include <stdio.h>
#include "PowerElectronics.hpp"
#include "SPImaster.hpp"
#include <array>
#include "QuickPID.h"


struct _device_values_t {
    float vshunt;
    float vbus;
    float dietemp;
    float current;
    float power;
    float energy;
    float charge;

    bool en;
    std::array<bool, 7> en_measurement;
    int device_pin;
};

struct _pwm_values_t {
    float duty_cycle;

    ledc_channel_t channel;
    int device_pin;
    int invert;

    QuickPID pid;
    float setpoint;
};

enum ALGORITHMS
{
    MANUAL,
    PID
};

enum DEVICE_VALUE_INDEX {
    DEVICEVALUE_VOLTAGE_SHUNT,
    DEVICEVALUE_VOLTAGE_BUS,
    DEVICEVALUE_DIE_TEMP,
    DEVICEVALUE_CURRENT,
    DEVICEVALUE_POWER,
    DEVICEVALUE_ENERGY,
    DEVICEVALUE_CHARGE
};

static const char* CONTROL_TAG = "CONTROL";

template<size_t pwm_size, size_t device_size>
class Control {
    public:
        Control(ledc_timer_bit_t resolution, int freq, ledc_timer_t timer, float max_current, float r_shunt);
        Control();
        ~Control();

        void setAlgorithm(int type) { _algorithm = type; };
        int getAlgorithm() { return _algorithm; };

        void setPWM(int id, float value) { _pwm_values[id].duty_cycle = value; };
        float getPWM(int id) { return _pwm_values[id].duty_cycle; };

        void setSetPoint(int id, float value) { _pwm_values[id].setpoint = value; };
        int getSetPoint(int id) { return _pwm_values[id].setpoint; };

        void setPWM_PID_Kp(int id, int value) { _pwm_values[id].pid.SetTunings(value, 0, 0); };

        void setDeviceEN(int id, bool en) { _devices[id].en = en; };
        bool getDeviceEN(int id) {return _devices[id].en; };

        void setMeasurementEN(int id, bool en) { _devices[id].en = en; };
        bool getMeasurementEN(int id) {return _devices[id].en; };

        _device_values_t getDeviceParams(int id) { return _devices[id]; };
        std::array<_device_values_t, device_size> getAllDeviceParams() { return _devices; };

        virtual bool update();
        bool testDevice(int id);
    protected:

        void config();

        PowerElectronics _pwm;
        SPImaster _spi;

        int _algorithm;
        std::array<_pwm_values_t, pwm_size> _pwm_values;
        std::array<_device_values_t, device_size> _devices;

        virtual void update_measurements();
        virtual void update_pwm();

        virtual void PID_control() = 0;
};

template<size_t pwm_size, size_t device_size>
Control<pwm_size, device_size>::Control(ledc_timer_bit_t resolution, int freq, ledc_timer_t timer, float max_current, float r_shunt) : 
    _pwm (PowerElectronics(resolution, freq, timer)),
    _spi (SPImaster(max_current, r_shunt))
{
}

template<size_t pwm_size, size_t device_size>
Control<pwm_size, device_size>::~Control()
{
}

template<size_t pwm_size, size_t device_size>
bool Control<pwm_size, device_size>::update()
{
    update_measurements();
    update_pwm();
    return false;
}

template <size_t pwm_size, size_t device_size>
void Control<pwm_size, device_size>::config()
{
    
    for(int i = 0; i < device_size; i++)
    {
        _spi.config(_devices[i].device_pin);
    }
    for(int i = 0; i < pwm_size; i++)
    {
        _pwm.config(_pwm_values[i].channel, _pwm_values[i].device_pin, _pwm_values[i].invert);
        _pwm.setDUTY(_pwm_values[i].duty_cycle, _pwm_values[i].channel);
    }
}

/*
REGISTER DESCRIPTIONS
VSHUNT: Differential voltage measured across the shunt output. Two's complement value. Conversion factor: 312.5 nV/LSB when ADCRANGE = 0 78.125 nV/LSB when ADCRANGE = 1
Size: 24
VBUS: Bus voltage output. Two's complement value, however always positive. Conversion factor: 195.3125 µV/LSB
Size: 24
DIETEMP: Internal die temperature measurement. Two's complement value. Conversion factor: 7.8125 m°C/LSB
Size: 16
CURRENT: Calculated current output in Amperes. Two's complement value.
Size: 24
POWER: Calculated power output. Output value in watts. Unsigned representation. Positive value.
Size: 24
ENERGY: Calculated energy output. Output value is in Joules. Unsigned representation. Positive value.
Size: 40
CHARGE: Calculated charge output. Output value is in Coulombs. Two's complement value.
Size: 40
*/

template<size_t pwm_size, size_t device_size>
void Control<pwm_size, device_size>::update_measurements()
{
    float rx;
    for(int i = 0; i < device_size; i++)
    {
        if(_devices[i].en)
        {
            if(_devices[i].en_measurement[DEVICEVALUE_VOLTAGE_SHUNT])
            {
                _spi.read(_devices[i].device_pin, VSHUNT, rx);
                _devices[i].vshunt = rx;
            }
            if(_devices[i].en_measurement[DEVICEVALUE_VOLTAGE_BUS])
            {
                _spi.read(_devices[i].device_pin, VBUS, rx);
                _devices[i].vbus = rx;
            }
            if(_devices[i].en_measurement[DEVICEVALUE_DIE_TEMP])
            {
                _spi.read(_devices[i].device_pin, DIETEMP, rx);
                _devices[i].dietemp = rx;               
            }
            if(_devices[i].en_measurement[DEVICEVALUE_CURRENT])
            {
                _spi.read(_devices[i].device_pin, CURRENT, rx);
                _devices[i].current = rx;                 
            }
            if(_devices[i].en_measurement[DEVICEVALUE_POWER])
            {
                _spi.read(_devices[i].device_pin, POWER, rx);
                _devices[i].dietemp = rx;                 
            }
            if(_devices[i].en_measurement[DEVICEVALUE_ENERGY])
            {
                 _spi.read(_devices[i].device_pin, ENERGY, rx);
                _devices[i].dietemp = rx;                 
            }
            if(_devices[i].en_measurement[DEVICEVALUE_CHARGE])
            {
                _spi.read(_devices[i].device_pin, CHARGE, rx);
                _devices[i].dietemp = rx;                  
            }
        }
    }
}

template<size_t pwm_size, size_t device_size>
void Control<pwm_size, device_size>::update_pwm()
{
    switch(_algorithm)
    {
        case(MANUAL):
            for(int i = 0; i < pwm_size; i++)
            {
                _pwm.setDUTY(_pwm_values[i].duty_cycle, _pwm_values[i].channel);
            }
            break;
        case(PID):
            PID_control();
            break;
    }
}

template<size_t pwm_size, size_t device_size>
bool Control<pwm_size, device_size>::testDevice(int id)
{ 
    //Checks to see if we can read the manufacturer id from the device
    long long data;
    _spi.read(_devices[id].device_pin, MANUFACTURER_ID, data);
    if(data == 0x5449)
    {
        return true;
    }
    else
    {
        return false;
    }
}