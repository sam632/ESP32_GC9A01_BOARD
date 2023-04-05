#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

// MQTT vars
const char* humTopic         = "multi-sensor/sensor/multi_sensor_humidity/state";
const char* bklstateTopic    = "homeassistant/switch/lcd_display_backlight/state";
const char* bklcmdTopic      = "homeassistant/switch/lcd_display_backlight/set";
const char* sensorstateTopic = "homeassistant/sensor/lcd_display_sensor/state";

void initWiFi() {

  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    i = i + 1;
    if (i > 15) {
      ESP.restart();
    }
  }
}

void sendMQTTDiscoveryMsg(PubSubClient &client, String name, String unit) {

  String name_string = String(DEVICE_NAME);
  name_string.toLowerCase();
  name_string.replace(" ", "_");

  String type = name;
  type.toLowerCase();

  String topic = "homeassistant/sensor/" + name_string + "_" + type + "/config";
  
  char buffer[256];
  DynamicJsonDocument doc(1024);

  doc["name"] = String(DEVICE_NAME) + " " + name;
  doc["unique_id"] = name_string + "_" + type;
  
  if(unit.length() == 0) {
    topic = "homeassistant/switch/" + name_string + "_" + type + "/config";

    doc["state_topic"] = bklstateTopic;
    doc["command_topic"] = bklcmdTopic;
  }
  else {
    doc["device_class"] = type;
    doc["state_topic"] = sensorstateTopic;
    doc["unit_of_measurement"] = unit;
    doc["value_template"] = "{{ value_json." + type + " }}";
  }

  size_t n = serializeJson(doc, buffer);
  client.publish(topic.c_str(), buffer, n);
}

void reconnectMQTT(PubSubClient &client, const char* humTopic, const char* bklcmdTopic) {

  int i = 0;
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      client.setBufferSize(512);
      sendMQTTDiscoveryMsg(client, "Backlight", "");
      sendMQTTDiscoveryMsg(client, "Temperature", "°C");
      sendMQTTDiscoveryMsg(client, "Pressure", "hPa");
      client.subscribe(humTopic);
      client.subscribe(bklcmdTopic);
    } else {
      i++;
      delay(5000);
    }
    if (i > 15) {
      ESP.restart();
    }
  }
}

void pushToHA(PubSubClient &client, float temperature, float pressure) {
  
  char buffer[256];
  DynamicJsonDocument doc(1024);

  doc["temperature"] = round(temperature*10) / 10.0;
  doc["pressure"] = round(pressure/10) / 10.0;

  size_t n = serializeJson(doc, buffer);

  client.publish(sensorstateTopic, buffer);
}

void backlightToggle(PubSubClient &client, const char* topic, String state) {

  if (state == "ON") {
    digitalWrite(TFT_BL, HIGH);
  }
  if (state == "OFF") {
    digitalWrite(TFT_BL, LOW);
  }
  client.publish(topic, state.c_str());
}