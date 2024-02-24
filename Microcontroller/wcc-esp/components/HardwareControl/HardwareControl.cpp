#include <stdio.h>
#include "HardwareControl.hpp"

HardwareControl::HardwareControl()
{
    _spi.config(R1_AC_THREEPHASE);
    _spi.config(R2_AC_THREEPHASE);
    _spi.config(R3_AC_THREEPHASE);
    _spi.config(R4_DC_RECTIFIED);
    _spi.config(R7_DC_BUCK);
    _spi.config(R8_DC_BATTERY);
    _spi.config(R9_DC_DUMP);

    _pwm_values = {0, 30, 0};

    _pwm.setDUTY(_pwm_values[PWM_RECTIFIER], RECTIFIER_INDEX);
    _pwm.setDUTY(_pwm_values[PWM_SEPIC], SEPIC_INDEX);
    _pwm.setDUTY(_pwm_values[PWM_LOAD], PS1_INDEX);
    _pwm.setDUTY((1 - _pwm_values[PWM_LOAD]), PS2_INDEX);

    _algorithm = MANUAL;

    _devices[DEVICEINDEX_R1_THREEPHASE].en = false;
    _devices[DEVICEINDEX_R1_THREEPHASE].en_measurement = {1, 1, 1, 0, 0, 0, 0};
    _devices[DEVICEINDEX_R1_THREEPHASE].device_pin = R1_AC_THREEPHASE;

    _devices[DEVICEINDEX_R2_THREEPHASE].en = false;
    _devices[DEVICEINDEX_R2_THREEPHASE].en_measurement = {1, 1, 1, 0, 0, 0, 0};
    _devices[DEVICEINDEX_R2_THREEPHASE].device_pin = R2_AC_THREEPHASE;

    _devices[DEVICEINDEX_R3_THREEPHASE].en = false;
    _devices[DEVICEINDEX_R3_THREEPHASE].en_measurement = {1, 1, 1, 0, 0, 0, 0};
    _devices[DEVICEINDEX_R3_THREEPHASE].device_pin = R3_AC_THREEPHASE;

    _devices[DEVICEINDEX_R4_RECTIFIED].en = false;
    _devices[DEVICEINDEX_R4_RECTIFIED].en_measurement = {1, 1, 1, 0, 0, 0, 0};
    _devices[DEVICEINDEX_R4_RECTIFIED].device_pin = R4_DC_RECTIFIED;

    _devices[DEVICEINDEX_R7_BUCK].en = false;
    _devices[DEVICEINDEX_R7_BUCK].en_measurement = {1, 1, 1, 0, 0, 0, 0};
    _devices[DEVICEINDEX_R7_BUCK].device_pin = R7_DC_BUCK;

    _devices[DEVICEINDEX_R8_BATTERY].en = false;
    _devices[DEVICEINDEX_R8_BATTERY].en_measurement = {1, 1, 1, 0, 0, 0, 0};
    _devices[DEVICEINDEX_R8_BATTERY].device_pin = R8_DC_BATTERY;

    _devices[DEVICEINDEX_R9_DUMP].en = false;
    _devices[DEVICEINDEX_R9_DUMP].en_measurement = {1, 1, 1, 0, 0, 0, 0};
    _devices[DEVICEINDEX_R9_DUMP].device_pin = R9_DC_DUMP;

}

HardwareControl::~HardwareControl()
{
}

bool HardwareControl::update()
{
    update_measurements();
    update_pwm();
    return false;
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
void HardwareControl::update_measurements()
{
    long rx;
    for(int i = 0; i < TOTAL_MEASUREMENT_DEVICES; i++)
    {
        if(_devices[i].en)
        {
            if(_devices[i].en_measurement[DEVICEVALUE_VOLTAGE_SHUNT])
            {
                _spi.read(_devices[i].device_pin, VSHUNT, rx);
                rx = rx << 8 << 32;
                rx = rx >> 8 >> 32;

                _devices[i].vshunt = ((double)rx) * SHUNTVOLTAGE_LSB;
            }
            if(_devices[i].en_measurement[DEVICEVALUE_VOLTAGE_BUS])
            {
                _spi.read(_devices[i].device_pin, VBUS, rx);
                rx = rx << 8 << 32;
                rx = rx >> 8 >> 32;

                _devices[i].vbus = ((double)rx) * BUSVOLTAGE_LSB;
            }
            if(_devices[i].en_measurement[DEVICEVALUE_DIE_TEMP])
            {
                _spi.read(_devices[i].device_pin, DIETEMP, rx);
                rx = rx << 16 << 32;
                rx = rx >> 16 >> 32;

                _devices[i].dietemp = ((double)rx) * TEMPERATURE_LSB;                
            }
            if(_devices[i].en_measurement[DEVICEVALUE_CURRENT])
            {
                _spi.read(_devices[i].device_pin, DIETEMP, rx);
                rx = rx << 8 << 32;
                rx = rx >> 8 >> 32;

                _devices[i].dietemp = ((double)rx) * CURRENT_LSB;                 
            }
            if(_devices[i].en_measurement[DEVICEVALUE_POWER])
            {
                 _spi.read(_devices[i].device_pin, POWER, rx);

                _devices[i].dietemp = ((double)rx) * POWER_LSB;                 
            }
            if(_devices[i].en_measurement[DEVICEVALUE_ENERGY])
            {
                 _spi.read(_devices[i].device_pin, ENERGY, rx);

                _devices[i].dietemp = ((double)rx) * ENERGY_LSB;                 
            }
            if(_devices[i].en_measurement[DEVICEVALUE_CHARGE])
            {
                _spi.read(_devices[i].device_pin, CHARGE, rx);
                rx = rx << 24;
                rx = rx >> 24;

                _devices[i].dietemp = ((double)rx) * CHARGE_LSB;                  
            }
        }
    }
}

void HardwareControl::update_pwm()
{
    switch(_algorithm)
    {
        case(MANUAL):
            _pwm.setDUTY(_pwm_values[PWM_RECTIFIER], RECTIFIER_INDEX);
            _pwm.setDUTY(_pwm_values[PWM_SEPIC], SEPIC_INDEX);
            _pwm.setDUTY(_pwm_values[PWM_LOAD], PS1_INDEX);
            _pwm.setDUTY((1 - _pwm_values[PWM_LOAD]), PS2_INDEX);
            break;
        case(MPPT):
            mppt();
            break;
        case(MCPT):
            mcpt();
            break;
    }
}

void HardwareControl::mppt()
{
}

void HardwareControl::mcpt()
{
}
