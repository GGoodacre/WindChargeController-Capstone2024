#pragma once
#include "Arduino.h"
#include <SPI.h>

#ifndef SPI_PINS
  #define VSPI_MISO   36
  #define VSPI_MOSI   21
  #define VSPI_SCLK   35
  #define VSPI_SS     3

  #define VSPI FSPI
#endif

static const char* SPI_TAG = "SPI";

class SPImaster 
{
    public:
    SPImaster(float max_current, float r_shunt);

    bool config(int cs);
    bool config(int cs, int config, int adc_config);
    void reset(int cs);

    bool read(int cs, int register_addr, float& data);
    bool write(int cs, int register_addr, int data);
    bool read(int cs, int register_addr, long long& data);

    private:

    SPIClass *spi = new SPIClass(VSPI);
    static const int spiClk = 1000000;
    void flipBytes(int &data);

    const std::array<int, 18> register_size;

    const float CURRENT_MAX;
    const float R_SHUNT;
    const float SHUNT_TEMPCO_TEMPCO;

    const float SHUNTVOLTAGE_LSB;
    const float CURRENT_LSB;
    const float BUSVOLTAGE_LSB;
    const float POWER_LSB;
    const float ENERGY_LSB;
    const float CHARGE_LSB;
    const float TEMPERATURE_LSB;
    const float SHUNT_CAL_VALUE;

};

#ifndef REGISTER_ADDRESS
  #define CONFIG            0x0
  #define ADC_CONFIG        0x1
  #define SHUNT_CAL         0x2
  #define SHUNT_TEMPCO      0x3
  #define VSHUNT            0x4
  #define VBUS              0x5
  #define DIETEMP           0x6
  #define CURRENT           0x7
  #define POWER             0x8
  #define ENERGY            0x9
  #define CHARGE            0xA
  #define DIAG_ALRT         0xB
  #define SOVL              0xC
  #define SUVL              0xD
  #define BOVL              0xE
  #define BUVL              0xF
  #define TEMP_LIMIT        0x10
  #define PWR_LIMIT         0x11
  #define MANUFACTURER_ID   0x3E
  #define DEVICE_ID         0x3F
  #define READ_REGISTER     0x1
  #define WRITE_REGISTER    0x0
  #define WRITE_SIZE        0x03
  #define WRITE_REGISTER_SHIFT    0x12
  #define WRITE_RW_SHIFT          0x10
  #define READ_REGISTER_SHIFT     0x2
  #define READ_RW_SHIFT           0x0
#endif

#ifndef CONFIG_IDS
  #define CONFIG            0x0
  #define ADC_CONFIG        0x1
  #define SHUNT_CAL         0x2
  #define SHUNT_TEMPCO      0x3
  #define VSHUNT            0x4
  #define VBUS              0x5
  #define DIETEMP           0x6
  #define CURRENT           0x7
  #define POWER             0x8
  #define ENERGY            0x9
  #define CHARGE            0xA
  #define DIAG_ALRT         0xB
  #define SOVL              0xC
  #define SUVL              0xD
  #define BOVL              0xE
  #define BUVL              0xF
  #define TEMP_LIMIT        0x10
  #define PWR_LIMIT         0x11
  #define MANUFACTURER_ID   0x3E
  #define DEVICE_ID         0x3F
  #define READ_REGISTER     0x1
#endif

#ifndef CONFIG_PARAM
  #define CONFIG_RST 0x0 << 15 
  #define CONFIG_RSTACC 0x0 << 14   
  #define CONFIG_CONVDLY 0x0 << 6  
  #define CONFIG_TEMPCOMP 0x0 << 5   
  #define CONFIG_ADCRANGE 0x0 << 4 

  #define ADC_CONFIG_MODE 0x7 << 12
  #define ADC_CONFIG_VBUSCT 0x0 << 9    
  #define ADC_CONFIG_VSHCT 0x0 << 6   
  #define ADC_CONFIG_VTCT 0x0 << 3   
  #define ADC_CONFIG_AVG 0x0 << 0   

  #define WRITE_REGISTER    0x0
  #define WRITE_SIZE        0x03
  #define WRITE_REGISTER_SHIFT    0x12
  #define WRITE_RW_SHIFT          0x10
  #define READ_REGISTER_SHIFT     0x2
  #define READ_RW_SHIFT           0x0
#endif
