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
/// MIT License
/// 
/// Copyright (c) 2018 J. Talbot
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///
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
