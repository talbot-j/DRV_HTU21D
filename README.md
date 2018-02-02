# Driver for the HTU21D Humidity Sensor

This driver was designed to communicate to any HTU21D(F) sensors.  For this code the Sparkfun Weather Sheild was used for testing and validation of the code.  Because the sensor communicated usigng I2C it should universal.  This driver includes the CRC checks that most other open sourced drivers out there fail to include.

The drv_htu21d.cpp and drv_htu21d.h files are the driver files.

The DRV_HTU21D.ino is the Arduino Sketch used to test and validate the drivers.
