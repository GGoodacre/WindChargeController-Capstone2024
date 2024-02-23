#include "PowerElectronics.hpp"
#include "SPImaster.hpp"
#include <array>

/*
REGISTER DESCRIPTIONS
VSHUNT: Differential voltage measured across the shunt output. Two's complement value. Conversion factor: 312.5 nV/LSB when ADCRANGE = 0 78.125 nV/LSB when ADCRANGE = 1
VBUS: Bus voltage output. Two's complement value, however always positive. Conversion factor: 195.3125 µV/LSB
DIETEMP: Internal die temperature measurement. Two's complement value. Conversion factor: 7.8125 m°C/LSB
CURRENT: Calculated current output in Amperes. Two's complement value.
POWER: Calculated power output. Output value in watts. Unsigned representation. Positive value.
ENERGY: Calculated energy output. Output value is in Joules. Unsigned representation. Positive value.
CHARGE: Calculated charge output. Output value is in Coulombs. Two's complement value.
*/
/*
struct _device_values {
    int vshunt,
    int vbus,
    int dietemp,
    int current,
    int power,
    int energy,
    int charge
};


class HardwareControl {
    public:
        void setAlgoriithm(int type) { _algorithm = type; };
        int getAlgoriithm() { return _algorithm; };

        void setPWM(int id, int value) { if(_algorithm == MANUAL) _pwm_values[id] = value; };
        int getPWM(int id) { return _pwm_values[id]; };

        bool update();

        _device_values getDeviceParams(int id) { return _devices[id]; };
    private:
        PowerElectronics _pwm;
        SPImaster _spi;

        int _algorithm;
        array<int> _pwm_values[3];
        array<_device_values> _devices[7];

        void mppt();
        void mcpt();
};

enum ALGORITHMS {
    MPPT,
    MCPT,
    MANUAL
}

enum PWM_INDEX {
    RECTIFIER,
    SEPIC,
    LOAD
}

enum MEASUREMENT_INDEX {
    VOLTAGE_SHUNT,
    VOLTAGE_BUS,
    DIE_TEMP,
    CURRENT,
    POWER,
    ENERGY,
    CHARGE
}
*/