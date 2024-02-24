#include "PowerElectronics.hpp"
#include "SPImaster.hpp"
#include <array>


enum ALGORITHMS {
    MPPT,
    MCPT,
    MANUAL
};


struct _device_values {
    double vshunt;
    double vbus;
    double dietemp;
    double current;
    double power;
    double energy;
    double charge;

    bool en;
    std::array<bool, 7> en_measurement;
    int device_pin;
};

#define TOTAL_MEASUREMENT_DEVICES 7

class HardwareControl {
    public:
        HardwareControl();
        ~HardwareControl();

        void setAlgoriithm(int type) { _algorithm = type; };
        int getAlgoriithm() { return _algorithm; };

        void setPWM(int id, int value) { if(_algorithm == MANUAL) _pwm_values[id] = value; };
        int getPWM(int id) { return _pwm_values[id]; };

        void setDeviceEN(int id, bool en) { _devices[id].en = en; };
        bool getDeviceEN(int id) {return _devices[id].en; };

        void setMeasurementEN(int id, bool en) { _devices[id].en = en; };
        bool getMeasurementEN(int id) {return _devices[id].en; };

        _device_values getDeviceParams(int id) { return _devices[id]; };
        std::array<_device_values, TOTAL_MEASUREMENT_DEVICES> getAllDeviceParams() { return _devices; };

        bool update();
    private:

        PowerElectronics _pwm;
        SPImaster _spi;

        int _algorithm;
        std::array<int, 3> _pwm_values;
        std::array<_device_values, TOTAL_MEASUREMENT_DEVICES> _devices;

        void update_measurements();
        void update_pwm();

        void mppt();
        void mcpt();
};

enum PWM_INDEX {
    PWM_RECTIFIER,
    PWM_SEPIC,
    PWM_LOAD
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

enum DEVICE_INDEX {
    DEVICEINDEX_R1_THREEPHASE,
    DEVICEINDEX_R2_THREEPHASE,
    DEVICEINDEX_R3_THREEPHASE,
    DEVICEINDEX_R4_RECTIFIED,
    DEVICEINDEX_R7_BUCK,
    DEVICEINDEX_R8_BATTERY,
    DEVICEINDEX_R9_DUMP,
};