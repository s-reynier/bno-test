#include <I2CWrapper.h>
#include <Wire.h>

#ifndef VARIO_TW_SDA_PIN
#define VARIO_TW_SDA_PIN 27
#endif

#ifndef VARIO_TW_SCL_PIN
#define VARIO_TW_SCL_PIN 32
#endif

#define POWER_PIN 12
#define POWER_PIN_STATE HIGH 

I2CWrapper i2CWrapper;

void I2CWrapper::init(void) {

    delay(100);
    Wire.flush();
    Wire.begin (VARIO_TW_SDA_PIN, VARIO_TW_SCL_PIN);
    delay (250);
    Wire.setClock(400000); //Increase I2C data rate to 400kHz
}