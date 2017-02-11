//------------------------------------------------------------------------------
/// @addtogroup htu21d_driver HUT21D Driver
///
/// @file drv_htu21d.cpp
///
/// @author     Joshua R. Talbot
///
/// @date       09-FEB17
///
/// @version    1.0 - initial release
///
/// @license
///
//------------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "Arduino.h"    

#include "drv_htu21d.h"

#if defined(__AVR__)
    #include <util/delay.h>
#endif

#include "Wire.h"

// HTU21D Device Specific Definitions
#define DRV_HTU21D_I2CADDR         (0x40)
// HUT21D Command and Code
#define DRV_HTU21D_READTEMP        (0xE3)
#define DRV_HTU21D_READHUM         (0xE5)

#define DRV_HTU21D_READTEMP_NHM    (0xF3)
#define DRV_HTU21D_READHUM_NHM     (0xF5)

#define DRV_HTU21D_WRITE_USR_REG   (0xE6)
#define DRV_HTU21D_READ_USR_REG    (0xE7)
#define DRV_HTU21D_SOFTRESET       (0xFE)

#define DRV_HTU21D_MAXRES          0
#define DRV_HTU21D_LORES           1
#define DRV_HTU21D_MIDRES          2
#define DRV_HTU21D_HIRES           3

/* poly:  x^8 + x^5 + x^4 + 1 */    
#define CRC8_POLYNOMINAL (0b100110001)

/**
 * @brief      Constructs the HTU21D Driver
 */
DRV_HTU21D::DRV_HTU21D() {
    config_changed = false;
    user_register = 0b00000010;
}

/**
 * @brief      Initialize the Humidity Sensor
 *
 * @return     true if init successful.
 */
bool DRV_HTU21D::init(void) {
    Wire.begin();

    reset();

    Wire.beginTransmission(DRV_HTU21D_I2CADDR);
    Wire.write(DRV_HTU21D_READ_USR_REG);
    Wire.endTransmission();
    Wire.requestFrom(DRV_HTU21D_I2CADDR, 1);
    return (Wire.read() == 0x2); // after reset should be 0x2
}

/**
 * @brief      Reset the Humidity Sensor
 */
void DRV_HTU21D::reset(void) {
    Wire.beginTransmission(DRV_HTU21D_I2CADDR);
    Wire.write(DRV_HTU21D_SOFTRESET);
    Wire.endTransmission();
    delay(15);
}

/**
 * @brief      Gets temperature in Celsius from the sensor.
 *
 * @return     Temperature in Celsius.
 */
float DRV_HTU21D::getTemp_C(void) {
    
    // OK lets ready!
    Wire.beginTransmission(DRV_HTU21D_I2CADDR);
    Wire.write(DRV_HTU21D_READTEMP);
    Wire.endTransmission();

    delay(50); // add delay between request and actual read!

    Wire.requestFrom(DRV_HTU21D_I2CADDR, 3);
    while (!Wire.available()) {}

    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    /** @todo use the crc as a check for valid data. */
    uint8_t crc = Wire.read();
    float tempC_f = -999;

    if ( 0 == check_crc8( ( ((uint16_t)msb)<<8|lsb ), crc )) {
        if ( lsb & 0x02 ) {
            tempC_f = -990;
        }
        else {
            lsb = lsb & ~(0x03);
            uint16_t tempC; // = Wire.read();
            tempC = ( (uint16_t) msb )<< 8;
            tempC |= (uint16_t) lsb;
            tempC_f = (float) tempC;
            tempC_f = ((175.72*tempC_f)/65536) - 46.85;
        }
    }
    return tempC_f;
}
 
/**
 * @brief      Gets temperature in Fahrenheit from the sensor.
 *
 * @return     Temperature in Fahrenheit.
 */
float DRV_HTU21D::getTemp_F(void) {
    float tempC = getTemp_C();
    float tempF = (tempC * 9.0/5.0) + 32;
    return tempF;
}

/**
 * @brief      Gets the humidity in percentage from the sensor.
 *
 * @return     The humidity %.
 */
float DRV_HTU21D::getHumidity(void) {

    Wire.beginTransmission(DRV_HTU21D_I2CADDR);
    Wire.write(DRV_HTU21D_READHUM);
    Wire.endTransmission();

    delay(50); // add delay between request and actual read!

    Wire.requestFrom(DRV_HTU21D_I2CADDR, 3);
    while (!Wire.available()) {}

    //uint8_t msb = Wire.read();
    //uint8_t lsb = Wire.read();
    uint16_t data_for_crc;
    data_for_crc = Wire.read();
    data_for_crc <<= 8;
    data_for_crc |= Wire.read();

    //(((uint16_t)msb)<<8) | (lsb);
    /** @todo use the crc as a check for valid data. */
    uint8_t crc = Wire.read();
    
    float hum_f = -999;
    if ( crc == calc_crc8(data_for_crc) ) {
        hum_f = -990;
        if ( data_for_crc & 0x0002 ) {
            data_for_crc = data_for_crc & ~(0x03);
            //uint16_t hum; // = Wire.read();
            //hum = ( (uint16_t) msb )<< 8;
            //hum |= (uint16_t) lsb;
            hum_f = (float) data_for_crc;
            hum_f = ((125.0*hum_f)/65536) - 6;
        }
    } 
    return hum_f;
}

/**
 * @brief      Sets the sensor resolution.
 *
 *   |-----------------------------------------------|
 *   | Bit-7 | Bit-0 |   RH    |  Temp   | Selection |
 *   |-------|-------|---------|---------|-----------|
 *   |   0   |   0   | 12 bits | 14 bits |  MAXRES   |
 *   |   0   |   1   |  8 bits | 12 bits |  LORES    |
 *   |   1   |   0   | 10 bits | 13 bits |  MIDRES   |
 *   |   1   |   1   | 11 bits | 11 bits |  HIRES    |
 *   |-----------------------------------------------|
 *   
 * @param[in]  opt   the resolution option.
 * 
 * @warning This feature is not tested and its unclear if the temperature and humidity calculations require changes based on the expected resolution.  Its best to leave this at the default MAX resolution.  Plus what gains are there for the reduced resolution?
 */
void DRV_HTU21D::setResolution( uint8_t opt ) {
    user_register &= 0b01111110;
    if( DRV_HTU21D_LORES == opt ) {
        user_register |= 1;
    }
    else if ( DRV_HTU21D_MIDRES == opt ) {
        user_register |= 0x80;
    }
    else if ( DRV_HTU21D_HIRES == opt ) {
        user_register |= 0x81;
    }
    config_changed = true;
}

/**
 * @brief      Turns the Heater ON  / OFF
 *
 * @param[in]  on    if true, enable the heater, if false turn off the heater.
 */
void DRV_HTU21D::setHeater( bool on ) {
    uint8_t user_old;
    user_old = user_register;
    if ( on ) {
        user_register |= 0b11111011;
    }
    else {
        user_register &= 0b00000100;
    }
    if ( user_register ^ user_old ) {
        config_changed = true;
        setConfig();
    }
}

/**
 * @brief      Turns the Heater ON  / OFF
 *
 * @param[in]  on    if true, enable the heater, if false turn off the heater.
 */
void DRV_HTU21D::setConfig( void ) {
    
}

bool DRV_HTU21D::getConfig( void ) {

}

/**
 * @brief      Calculates the crc-8 of the input data.
 *
 * @param[in]  in_data  the data that the CRC will be performed upon.
 *
 * @todo this routine could stand for optimizations and refactoring.
 *
 * @return     The CRC result.
 */
uint8_t DRV_HTU21D::calc_crc8 ( uint16_t in_data ) {
    const uint16_t crc_bit_len = 8;
    const uint32_t term_bit_pos = 1<<(crc_bit_len-1);
    uint32_t polynominal = CRC8_POLYNOMINAL;
    
    /* pad the data with the crc length.
     * @note       that the in_data bit length is also changed required so data
     *             was not lost due to the shift, but orginally the input
     *             parameter was a 32bit number, however if a 32 value was
     *             passed to the function (as allowed) then this shift would
     *             destroy some of the in_data - not good so by forcing the
     *             input parameter to a 16 bit value, this shift will not
     *             destroy data without the users knowledge (or at least he
     *             should know.) */
    uint32_t data = ((uint32_t)in_data) << crc_bit_len;

    /* shift the polynomial to align with the bit position */
    polynominal <<= (32-9);

    /* iterate the XOR division */
    for ( uint32_t bit_pos=0x80000000; 
          ( bit_pos > term_bit_pos );  ) 
    {
        /* XOR performed if next bit position is a 1 */
        if ( bit_pos & data ) {
            data ^= polynominal;
        }
        /* shift the bit position marker and polynomial for the next check */
        bit_pos >>= 1;
        polynominal >>= 1;
    }

    return (uint8_t) data;
}

/**
 * @brief      Performs a CRC check, if the check passes the returned result
 *             will be 0.
 *
 * @param[in]  in_data  the data that the CRC will be performed upon.
 * @param[in]  check    the CRC check value (the remainder to insure a zero
 *                      division - if the data is valid.)
 *
 * @todo this routine could stand for optimizations and refactoring.
 *
 * @return     0 if the check passes, non-zero if the check fails.
 */
uint8_t DRV_HTU21D::check_crc8 ( uint16_t in_data, uint8_t check ) {
    const uint16_t crc_bit_len = 8;
    const uint32_t term_bit_pos = 1<<(crc_bit_len-1);
    uint32_t polynominal = CRC8_POLYNOMINAL;
    
    /* pad the data with the crc length.
     * @note       that the in_data bit length is also changed required so data
     *             was not lost due to the shift, but orginally the input
     *             parameter was a 32bit number, however if a 32 value was
     *             passed to the function (as allowed) then this shift would
     *             destroy some of the in_data - not good so by forcing the
     *             input parameter to a 16 bit value, this shift will not
     *             destroy data without the users knowledge (or at least he
     *             should know.) */
    uint32_t data = ( (uint32_t) in_data ) << crc_bit_len;
    data |= check;

    /* shift the polynomial to align with the bit position */
    polynominal <<= (32-9);

    /* iterate the XOR division */
    for ( uint32_t bit_pos=0x80000000; 
          ( bit_pos > term_bit_pos );  ) 
    {
        /* XOR performed if next bit position is a 1 */
        if ( bit_pos & data ) {
            data ^= polynominal;
        }
        /* shift the bit position marker and polynomial for the next check */
        bit_pos >>= 1;
        polynominal >>= 1;
    }

    return data;
}