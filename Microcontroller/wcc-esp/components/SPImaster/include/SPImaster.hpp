#include "Arduino.h"
#include <SPI.h>

#ifndef SPI_PINS
  #define VSPI_MISO   MISO
  #define VSPI_MOSI   MOSI
  #define VSPI_SCLK   SCK
  #define VSPI_SS     SS

  #define VSPI FSPI
#endif

static const char* SPI_TAG = "SPI";

class SPImaster 
{
    public:
    SPImaster();

    bool config(int cs);
    bool write(int cs, int register_addr, int data);
    bool read(int cs, int register_addr, long& data);

    private:

    SPIClass *spi = NULL;
    static const int spiClk = 10000;
    void flipBytes(int &data);
    std::array<int, 18> register_size = {
        16,
        16,
        16,
        16,
        24,
        24,
        16,
        24,
        24,
        40,
        40,
        16,
        16,
        16,
        16,
        16,
        16,
        16
    };

};

#ifndef CS_PINS
  #define R1_AC_THREEPHASE  1
  #define R2_AC_THREEPHASE  2
  #define R3_AC_THREEPHASE  3
  #define R4_DC_RECTIFIED   4
  #define R7_DC_BUCK        7
  #define R8_DC_BATTERY     8
  #define R9_DC_DUMP        9
#endif

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
  #define CONFIG_ADCRANGE 0x1 << 4 

  #define ADC_CONFIG_MODE 0xF << 12
  #define ADC_CONFIG_VBUSCT 0x0 << 9    
  #define ADC_CONFIG_VSHCT 0x0 << 6   
  #define ADC_CONFIG_VTCT 0x0 << 3   
  #define ADC_CONFIG_AVG 0x1 << 0   

    /*
    SHUNT_CAL = 13107.2 x 10^6 x CURRENT_LSB x R_SHUNT
    CURRENT_LSB = CURRENT_MAX/2^19
    SHUNT_CAL = SHUNT_CAL*4 if ADCRANGE = 1
    */ 
  #define CURRENT_MAX 10
  #define R_SHUNT 0.01

  #define SHUNT_TEMPCO_TEMPCO 0x0

  #define WRITE_REGISTER    0x0
  #define WRITE_SIZE        0x03
  #define WRITE_REGISTER_SHIFT    0x12
  #define WRITE_RW_SHIFT          0x10
  #define READ_REGISTER_SHIFT     0x2
  #define READ_RW_SHIFT           0x0
#endif
