//------------------------------------------------------------------------------
/// @addtogroup htu21d_test HUT21D Unit Test
///
/// @file DRV_HTU21D.ino
///
/// @author     Joshua R. Talbot
///
/// @date       09-FEB-17
///
/// @version    1.0 - initial release
///
/// @license
///
/// @brief      This is the non-exhaustive test for the HTU21D driver.  More
///             tests to come...
///

#include <Wire.h>       // required for I2C

#include "drv_htu21d.h" // need the hut21 driver we are testing.

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


DRV_HTU21D hum_sensor = DRV_HTU21D();

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
uint8_t check_crc8 ( uint16_t in_data, uint8_t check ) {
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
  uint32_t data = ( (uint32_t) in_data) << crc_bit_len;
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

  return (uint8_t) data;
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
uint8_t calc_crc8 ( uint16_t in_data ) {
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
    uint32_t data = ( (uint32_t)in_data ) << crc_bit_len;

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


float read_hum( void ) {

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
    data_for_crc = 0x683A;
    //(((uint16_t)msb)<<8) | (lsb);
    /** @todo use the crc as a check for valid data. */
    
    uint8_t crc = Wire.read();
    crc = 0x7C;
    noInterrupts();
    uint8_t crc_check = check_crc8(data_for_crc, crc);
    interrupts();
    Serial.print(" data16 = ");Serial.println(data_for_crc,HEX);
    Serial.print(" crc = ");Serial.println(crc,HEX);
    Serial.print(" checked crc = ");Serial.println(crc_check,HEX);
    
    float hum_f = -999;
    if ( 0 == crc_check ) {
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

void setup() {
    Serial.begin(9600);
    #if 1
    if ( hum_sensor.init() ) {
        Serial.println("\n\nHumidity Sensor Init'd!");
    }
    else {
        Serial.println("\n\nERR: Hum Sensor FAILED Init!");
    }
    #endif

}

void loop() {
    float temp_c, temp_f;

    // Read and display temperature 
    temp_c = hum_sensor.getTemp_C();
    temp_f = hum_sensor.getTemp_F();
    Serial.println("-------------------");
    Serial.print("Temp: ");
    Serial.print(temp_c); Serial.print(" *C\t");
    Serial.print(temp_f); Serial.println(" *F");

    // Read and display humidity %
    Serial.print("Hum: ");
    Serial.print(hum_sensor.getHumidity());Serial.println(" %");
    //float hum = read_hum();
    //Serial.print(hum);Serial.println(" %");
    delay(1000);
}
