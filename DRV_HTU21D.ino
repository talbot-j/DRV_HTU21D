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

DRV_HTU21D hum_sensor = DRV_HTU21D();

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

uint8_t loops = 0;

void loop() {
    float temp_c, temp_f;

    /* Perform basic tests - read and write user register data, read
     * temperatures (insure temp C and temp F) return the "same" temperature.
     * Other than that a second calibrated device will be required. */
    if ( loops < 1 ) {
        hum_sensor.setHeater( true );
        hum_sensor.setConfig();
        if ( 0x06 != hum_sensor.getConfig() ) {
            Serial.println("ERROR in setHeater(true)!");
            while(1);
        }
        hum_sensor.setHeater( false );
        hum_sensor.setConfig();
        if ( 0x02 != hum_sensor.getConfig() ) {
            Serial.println("ERROR in setHeater(false)!");
            while(1);
        }
        hum_sensor.setHeater( true );
        hum_sensor.setConfig();
        hum_sensor.reset();
        if ( 0x02 != hum_sensor.getConfig() ) {
            Serial.println("ERROR in reset!");
            while(1);
        }
        Serial.println("All Basic Tests pass!");
    }
    else if ( loops < 10 ) {
        hum_sensor.setHeater( true );
        hum_sensor.setConfig();
    }
    else {
        hum_sensor.setHeater( false );
        hum_sensor.setConfig();
    }
    loops++;

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

    delay(1000);
}
