#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>

#define BME280_I2C 0x76

void initBME(TwoWire &I2CBME, Adafruit_BME280 &bme) {

  I2CBME.begin(38, 39);
  bme.begin(BME280_I2C, &I2CBME);

  /* Default settings from the datasheet. */
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BME280::SAMPLING_X16,    /* Temp. oversampling */
                  Adafruit_BME280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BME280::SAMPLING_X16,    /* Humidity oversampling */
                  Adafruit_BME280::FILTER_X16,      /* Filtering. */
                  Adafruit_BME280::STANDBY_MS_500); /* Standby time. */
}