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
#include "Arduino.h"
#include <stdint.h>
#include <stdbool.h>

class DRV_HTU21D {
    public:
        DRV_HTU21D();
        bool init( void );
        void reset(void);
        void setConfig( void );
        uint16_t getConfig(void);
        float getTemp_C(void);
        float getTemp_F(void);
        float getHumidity(void);
        void setResolution( uint8_t );
        void setHeater( bool );
    private:
        bool read_HUT_Config(void);
        uint8_t check_crc8(uint16_t, uint8_t);
        uint8_t user_register;
        bool config_changed;
};