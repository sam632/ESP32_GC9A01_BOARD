#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

// MQTT vars
const char* humTopic         = "multi-sensor/sensor/multi_sensor_humidity/state";
const char* bklstateTopic    = "homeassistant/switch/lcd_display_backlight/state";
const char* bklcmdTopic      = "homeassistant/switch/lcd_display_backlight/set";
const char* sensorstateTopic = "lcd_display/sensor/lcd_display_sensor/state";
const char* availabilityTopic = "lcd_display/status";

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
  
  char buffer[1024];
  DynamicJsonDocument doc(1024);

  doc["name"] = String(DEVICE_NAME) + " " + name;
  doc["unique_id"] = name_string + "_" + type;

  JsonObject availability  = doc.createNestedObject("availability");
  availability["topic"] = availabilityTopic;
  availability["value_template"] = "{{ value }}";
  availability["payload_available"] = "online";
  availability["payload_not_available"] = "offline";

  JsonObject device  = doc.createNestedObject("device");
  device["name"] = DEVICE_NAME;
  device["manufacturer"] = "Sam Adamson";
  device["identifiers"] = "LCDSAXX01";
  
  if(unit.length() == 0) {
    topic = "homeassistant/switch/" + name_string + "_" + type + "/config";

    doc["state_topic"] = bklstateTopic;
    doc["command_topic"] = bklcmdTopic;
  }
  else {
    doc["device_class"] = type;
    doc["state_topic"] = sensorstateTopic;
    doc["unit_of_measurement"] = unit;
    doc["value_template"] = "{{ value_json." + type + " | round(1) }}";
  }

  size_t n = serializeJson(doc, buffer);
  client.publish(topic.c_str(), buffer, n);
}

void reconnectMQTT(PubSubClient &client, const char* humTopic, const char* bklcmdTopic) {

  int i = 0;
  while (!client.connected()) {
    String clientId = String(DEVICE_NAME);
    clientId.toLowerCase();
    clientId.replace(" ", "_");

    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD, availabilityTopic, 1, true, "offline")) {
      client.setBufferSize(1024);
      sendMQTTDiscoveryMsg(client, "Backlight", "");
      sendMQTTDiscoveryMsg(client, "Temperature", "°C");
      sendMQTTDiscoveryMsg(client, "Pressure", "hPa");
      client.subscribe(humTopic);
      client.subscribe(bklcmdTopic);
      client.publish(availabilityTopic, "online", true);
    } else {
      i++;
      delay(3000);
    }
    if (i > 15) {
      ESP.restart();
    }
  }
}

void pushToHA(PubSubClient &client, float temperature, float pressure) {
  
  char buffer[256];
  DynamicJsonDocument doc(512);

  doc["temperature"] = temperature;
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