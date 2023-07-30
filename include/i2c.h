#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>

#define BME280_I2C 0x76
#define BH1750_I2C 0x23

void initBME(TwoWire &I2C, Adafruit_BME280 &bme) {

  bme.begin(BME280_I2C, &I2C);

  /* Default settings from the datasheet. */
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BME280::SAMPLING_X16,    /* Temp. oversampling */
                  Adafruit_BME280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BME280::SAMPLING_X16,    /* Humidity oversampling */
                  Adafruit_BME280::FILTER_X16,      /* Filtering. */
                  Adafruit_BME280::STANDBY_MS_500); /* Standby time. */
}

void initBH1750(TwoWire &I2C, BH1750 &bh1750) {
  bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, BH1750_I2C, &I2C);
}