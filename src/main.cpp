/*
  - Home Assistant send unavailble
  - Refactor
*/

#include <Arduino.h>

#include "config.h"
#include "lcd.h"
#include "ota.h"
#include "bmp.h"
#include "mqtt.h"

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];

// Time Setup
uint32_t targetTime = 0; // to update time every few seconds

// Screen Setup
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite tempSprite = TFT_eSprite(&tft);
TFT_eSprite humSprite = TFT_eSprite(&tft);
TFT_eSprite timeSprite = TFT_eSprite(&tft);
TFT_eSprite arcSprite = TFT_eSprite(&tft);
TFT_eSprite background = TFT_eSprite(&tft);

// BMP280 Setup
TwoWire I2CBME = TwoWire(0);
Adafruit_BMP280 bmp(&I2CBME);

void MQTTcallback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  if (strcmp(topic, humTopic) == 0) {
    updateHumidity(humSprite, message);
  }
  if (strcmp(topic, bklcmdTopic) == 0) {
    backlightToggle(client, bklstateTopic, message);
  }
}

void initMQTT(PubSubClient &client, const char* humTopic, const char* bklcmdTopic) {

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(MQTTcallback);
  reconnectMQTT(client, humTopic, bklcmdTopic);
}

void setup(void) {

  Serial.begin(115200);

  tft.init();
  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.fillScreen(BACKGROUND);

  initWiFi();
  initMQTT(client, humTopic, bklcmdTopic);
  initOTA();
  initBMP(I2CBME, bmp);

  configTime(GMT_OFFSET, DAYLIGHT_OFFSET, NTP_SERVER);

  pinMode(TFT_BL, OUTPUT);
  backlightToggle(client, bklstateTopic, "ON");
  initDisplay(background, arcSprite, timeSprite, tempSprite, humSprite);
}

void loop() {

  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnectMQTT(client, humTopic, bklcmdTopic);
  }
  client.loop();

  if (targetTime < millis()) {
    // Set next update for 3 second later
    targetTime = millis() + 3000;

    updateTime(timeSprite);

    float temperature = bmp.readTemperature();
    updateTemp(tempSprite, temperature);
    updateTempDial(arcSprite, temperature);
    pushToHA(client, temperature, bmp.readPressure());
  }

  updateScreen(background, arcSprite, timeSprite, tempSprite, humSprite);
}
