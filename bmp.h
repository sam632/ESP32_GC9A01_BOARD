#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

#define BMP280_ADDRESS 0x76

void initBMP(TwoWire &I2CBME, Adafruit_BMP280 &bmp) {
  I2CBME.begin(38, 39);
  unsigned status;
  status = bmp.begin(BMP280_ADDRESS);
  // if (!status) {
  //   Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
  //                     "try a different address!"));
  //   while (1) delay(10);
  // }
    /* Default settings from the datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}