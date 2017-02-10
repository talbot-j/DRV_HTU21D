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
    if ( hum_sensor.init() ) {
        Serial.println("\n\nHumidity Sensor Init'd!");
    }
    else {
        Serial.println("\n\nERR: Hum Sensor FAILED Init!");
    }
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

    delay(1000);
}