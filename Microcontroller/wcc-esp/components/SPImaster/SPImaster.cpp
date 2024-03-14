#include "Arduino.h"
#include <SPI.h>
#include "SPImaster.hpp"
#include <bit>
#include <algorithm>
#include <array>

SPImaster::SPImaster(float max_current, float r_shunt) : 
    register_size       ({16,16,16,16,24,24,16,24,24,40,40,16,16,16,16,16,16,16}),
    CURRENT_MAX         (max_current),
    R_SHUNT             (r_shunt),
    SHUNT_TEMPCO_TEMPCO (0x0),
    SHUNTVOLTAGE_LSB    ((CONFIG_ADCRANGE & (0x1 << 4)) ? 78.125e-9 : 312.5e-9), //Need to change currently based off of default configuration 
    CURRENT_LSB         (CURRENT_MAX / (pow(2, 19))),
    BUSVOLTAGE_LSB      (195.3125e-6),
    POWER_LSB           (CURRENT_LSB * 3.2),
    ENERGY_LSB          (POWER_LSB * 16),
    CHARGE_LSB          (CURRENT_LSB),
    TEMPERATURE_LSB     (7.8125e-3),
    SHUNT_CAL_VALUE     ((13107.2*pow(10,6))*CURRENT_LSB*R_SHUNT)
{
    ESP_LOGI(SPI_TAG, "Shunt_Cal_Value: %f  |  Current_LSB: %f", SHUNT_CAL_VALUE, CURRENT_LSB);

    spi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS);
}

bool SPImaster::config(int cs)
{
    //Default Configuration
    config(cs, ( CONFIG_RST | CONFIG_RSTACC | CONFIG_CONVDLY | CONFIG_TEMPCOMP | CONFIG_ADCRANGE ), ( ADC_CONFIG_MODE | ADC_CONFIG_VBUSCT | ADC_CONFIG_VSHCT | ADC_CONFIG_VTCT | ADC_CONFIG_AVG ));
    return true;
}

bool SPImaster::config(int cs, int config, int adc_config)
{
    // Set SlaveSelect as an output and write it to its idle state
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);

    //Sends a reset command to the chip to clear anything previous
    reset(cs);

    //CONFIG REGISTER SETUP
    int config_word = config;
    write(cs, CONFIG, config_word);

    //ADC_CONFIG REGISTER SETUP
    config_word = adc_config;             
    write(cs, ADC_CONFIG, config_word);

    //SHUNT_CAL REGISTER SETUP
    float shunt_cal = SHUNT_CAL_VALUE;
    if (((0x1 << 4) & config)) shunt_cal = shunt_cal*4; // If ADC_RANGE is asserted multiply the shunt_cal value by 4 as per the data sheet
    config_word = (int)shunt_cal;
    write(cs, SHUNT_CAL, config_word);

    //SHUNT_TEMPCO REGISTER SETUP
    config_word = SHUNT_TEMPCO_TEMPCO;
    write(cs, SHUNT_TEMPCO, config_word);

    return true;
}

void SPImaster::reset(int cs)
{
    write(cs, CONFIG, 0x8000); // Reset Command
}

bool SPImaster::read(int cs, int register_addr, float &data)
{
    //Checks to see if the register you are reading is one that can be converted to a float
    if(register_addr < VSHUNT || register_addr > CHARGE)
    {
        return false;
    }
    else
    {
        long long rx;
        read(cs, register_addr, rx);
        switch(register_addr)
        {
            //Does the data conversion for the measurements reference INA229 datasheet for more information
            case(VSHUNT):
                rx = rx << (sizeof(rx)*8 - register_size[VSHUNT]);
                rx = rx >> (sizeof(rx)*8 - register_size[VSHUNT]) >> 4;
                data = (float)rx * SHUNTVOLTAGE_LSB;
                break;
            case(VBUS):
                rx = rx << (sizeof(rx)*8 - register_size[VBUS]);
                rx = rx >> (sizeof(rx)*8 - register_size[VBUS]) >> 4;
                data = (float)rx * BUSVOLTAGE_LSB;
                break;
            case(DIETEMP):
                rx = rx << (sizeof(rx)*8 - register_size[VSHUNT]);
                rx = rx >> (sizeof(rx)*8 - register_size[VSHUNT]);
                data = (float)rx * TEMPERATURE_LSB;
                break;
            case(CURRENT):
                rx = rx << (sizeof(rx)*8 - register_size[VSHUNT]);
                rx = rx >> (sizeof(rx)*8 - register_size[VSHUNT]) >> 4;
                data = (float)rx * CURRENT_LSB;
                break;
            case(POWER):
                data = (float)rx * POWER_LSB;
                break;
            case(ENERGY):
                data = (float)rx * ENERGY_LSB;
                break;
            case(CHARGE):
                rx = rx << (sizeof(rx)*8 - register_size[VSHUNT]);
                rx = rx >> (sizeof(rx)*8 - register_size[VSHUNT]);
                data = (float)rx * CHARGE_LSB;
                break;
        }
        if(data == 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
}

bool SPImaster::write(int cs, int register_addr, int data)
{
    //Cretes tx word
    int buff =  (register_addr << WRITE_REGISTER_SHIFT) | (WRITE_REGISTER << WRITE_RW_SHIFT) | data;
    //words get transferred least significant byte to most significant byte so flip the bytes around
    //please note that it still sends most significant bit first but just in byte chunks
    flipBytes(buff);
    SPImaster::spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));
    digitalWrite(cs, LOW);
    SPImaster::spi->transfer(&buff, WRITE_SIZE);
    digitalWrite(cs, HIGH);
    SPImaster::spi->endTransaction();
    ESP_LOGI(SPI_TAG, "Register: %d  |  Data Written: %d", register_addr, (int)data);
    return true;
}

bool SPImaster::read(int cs, int register_addr, long long &data)
{
    data = 0;
    int size;
    if (register_addr == MANUFACTURER_ID || register_addr == DEVICE_ID)
    {
        size = 3;
    }
    else
    {
        size = (register_size[register_addr])/8 + 1;
    }
    uint8_t tx[size];
    uint8_t rx[size];

    for(int i = 0; i < size; i++)
    {
        rx[i] = 0;
        tx[i] = 0;
    }
    tx[0] = (register_addr << READ_REGISTER_SHIFT) | (READ_REGISTER << READ_RW_SHIFT);

    SPImaster::spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));
    digitalWrite(cs, LOW);

    SPImaster::spi->transferBytes(tx, rx, size);

    digitalWrite(cs, HIGH);
    SPImaster::spi->endTransaction();

    for(int i = 1; i < size; i++)
    {
        data = data | rx[i] << (((size - 1) - i)*8);
    }

    ESP_LOGI(SPI_TAG, "CS:%d,Register:%d,Data_Read:%d", cs, register_addr, (int)data);
    return true;
}

void SPImaster::flipBytes(int &data)
{
    std::array<std::byte, WRITE_SIZE> value;
    int temp = data;
    for(int i = 0; i < WRITE_SIZE; i++)
    {
        value[i] = (std::byte)temp;
        temp = temp >> 8;
    }
    std::reverse(value.begin(), value.end());
    temp = 0;
    for(int i = 0; i < WRITE_SIZE; i++)
    {
        temp = temp + int((unsigned char)value[i] << (8*i));
    }
    data = temp;
}
