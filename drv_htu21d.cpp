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
    uint8_t crc = Wire.read();
    float tempC_f;

    if ( lsb & 0x02 ) {
        tempC_f = -999;
    }
    else {
        lsb = lsb & ~(0x03);
        uint16_t tempC; // = Wire.read();
        tempC = ( (uint16_t) msb )<< 8;
        tempC |= (uint16_t) lsb;
        tempC_f = (float) tempC;
        tempC_f = ((175.72*tempC_f)/65536) - 46.85;
    }
    return tempC_f;
}
 
/**
 * @brief      Gets temperature in Celsius from the sensor.
 *
 * @return     Temperature in Celsius.
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

    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    uint8_t crc = Wire.read();

    float hum_f;
    if ( lsb & 0x02 ) {
        lsb = lsb & ~(0x03);
        uint16_t hum; // = Wire.read();
        hum = ( (uint16_t) msb )<< 8;
        hum |= (uint16_t) lsb;
        hum_f = (float) hum;
        hum_f = ((125.0*hum_f)/65536) - 6;
    }
    else {
        hum_f = -999;
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