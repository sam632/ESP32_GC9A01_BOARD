/*
  - Home Assistant send unavailble
  - Add the arc to sprite and add ball to end
  - Refactor
*/

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <time.h>
#include <WiFi.h>

#include "config.h"
#include "lcd.h"
#include "ota.h"
#include "bmp.h"

// MQTT vars
const char* bkldiscovTopic = "homeassistant/switch/lcd_display_backlight/config";
const char* bklstateTopic  = "homeassistant/switch/lcd_display_backlight/state";
const char* bklcmdTopic    = "homeassistant/switch/lcd_display_backlight/set";
const char* humTopic       = "multi-sensor/sensor/multi_sensor_humidity/state";
const char* tempTopic      = "multi-sensor/sensor/multi_sensor_temperature/state";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];

// Time vars
uint32_t targetTime = 0; // to only update time every few seconds
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

// Screen vars
bool power = true;
String temperature = "00.0";
String humidity = "00";
String timeNow = "00:00";

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite tempSprite = TFT_eSprite(&tft);
TFT_eSprite humSprite = TFT_eSprite(&tft);
TFT_eSprite timeSprite = TFT_eSprite(&tft);

// BMP280 vars
TwoWire I2CBME = TwoWire(0);
Adafruit_BMP280 bmp(&I2CBME);

void initWiFi() {

  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    tft.print(".");
    delay(1000);
    i = i + 1;
    if (i > 15) {
      ESP.restart();
    }
  }
}

void initMQTT(PubSubClient &client) {

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(MQTTcallback);
  reconnectMQTT(client);
}

void reconnectMQTT(PubSubClient &client) {

  int i = 0;
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      client.subscribe(tempTopic);
      client.subscribe(humTopic);
      client.subscribe(bklcmdTopic);
      sendMQTTDiscoveryMsg(client);
    } else {
      i++;
      delay(5000);
    }
    if (i > 15) {
      ESP.restart();
    }
  }
}

void sendMQTTDiscoveryMsg(PubSubClient &client) {

  DynamicJsonDocument doc(1024);
  char buffer[256];

  doc["name"] = "LCD Display";
  doc["unique_id"] = "lcd_display_backlight";
  doc["state_topic"] = bklstateTopic;
  doc["command_topic"] = bklcmdTopic;

  size_t n = serializeJson(doc, buffer);

  client.publish(bkldiscovTopic, buffer);
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  if (strcmp(topic, humTopic) == 0) {
    humidity = message;
    updateHumidity(humSprite, power, humidity);
  }
  if (strcmp(topic, bklcmdTopic) == 0) {
    backlightToggle(message);
  }
}

void backlightToggle(String state) {

  if (state == "ON") {
    power = true;
    digitalWrite(TFT_BL, HIGH);
    initDisplay(tft);
    updateTemp(tempSprite, tft, power, temperature);
    updateHumidity(humSprite, power, humidity);
    timeNow = getLocalTime();
    updateTime(timeSprite, power, timeNow);
  }
  if (state == "OFF") {
    power = false;
    digitalWrite(TFT_BL, LOW);
    tft.fillScreen(TFT_BLACK);
  }
  client.publish(bklstateTopic, state.c_str());
}

String getLocalTime() {

  struct tm timeinfo;
  char timeNow[6];
  if (!getLocalTime(&timeinfo)) {
    return "Err";
  }
  strftime(timeNow, 6, "%H:%M", &timeinfo);
  return timeNow;
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("BMP280 test");

  tft.init();
  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.fillScreen(BACKGROUND);

  initWiFi();
  initMQTT(client);
  initOTA();
  initBMP(I2CBME, bmp);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  pinMode(TFT_BL, OUTPUT);
  backlightToggle("ON");

  targetTime = millis() + 3000;
}

void loop() {

  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnectMQTT(client);
  }
  client.loop();

  if (targetTime < millis()) {
    // Set next update for 3 second later
    targetTime = millis() + 3000;
    timeNow = getLocalTime();
    updateTime(timeSprite, power, timeNow);

    temperature = String(bmp.readTemperature(), 1);
    updateTemp(tempSprite, tft, power, temperature);
  
    Serial.print("Temperature  = ");
    Serial.println(bmp.readTemperature());
    Serial.print("Pressure = ");
    Serial.println(bmp.readPressure());
  }
}