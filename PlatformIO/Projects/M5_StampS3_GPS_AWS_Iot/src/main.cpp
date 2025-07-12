#include <WiFi.h>
#include <time.h>
#include <HardwareSerial.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <TinyGPSPlus.h>

// LED設定
#define LED_PIN 21
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// GPS設定
HardwareSerial GPS_serial(1);
TinyGPSPlus gps;
#define GPS_RX 16
#define GPS_TX 17

// AWS IoT
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#define WIFI_SSID "neko01"
#define WIFI_PASSPHRASE "64646631"
#define DEVICE_NAME "M5StampS3"
#define AWS_IOT_ENDPOINT "a1rf2bv1k9ed2q-ats.iot.us-east-1.amazonaws.com"
#define AWS_IOT_PORT 8883
#define PUBLISH_TOPIC DEVICE_NAME"/status"
#define SUBSCRIBE_TOPIC DEVICE_NAME"/blink"
#define QOS 0

WiFiClientSecure https_client;
PubSubClient mqtt_client(https_client);

// certs.cpp
extern const char *root_ca;
extern const char *certificate;
extern const char *private_key;

void setLED(CRGB color) {
  leds[0] = color;
  FastLED.show();
}

void flashLED(CRGB color, int duration_ms) {
  setLED(color);
  delay(duration_ms);
  setLED(CRGB::Black);
}

void connect_wifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);
  int retry_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    flashLED(CRGB::Red, 100);
    delay(500);
    retry_count++;
    if (retry_count >= 10) {
      WiFi.disconnect();
      delay(5000);
      WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);
      retry_count = 0;
    }
  }
  flashLED(CRGB::Green, 200);
}

bool connect_awsiot() {
  https_client.setCACert(root_ca);
  https_client.setCertificate(certificate);
  https_client.setPrivateKey(private_key);
  https_client.setHandshakeTimeout(5);
  mqtt_client.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);
  while (!mqtt_client.connected()) {
    if (mqtt_client.connect(DEVICE_NAME)) {
      mqtt_client.subscribe(SUBSCRIBE_TOPIC, QOS);
      flashLED(CRGB::Yellow, 500);
      return true;
    } else {
      flashLED(CRGB::Red, 100);
      delay(2000);
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  GPS_serial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  setLED(CRGB::Black);

  flashLED(CRGB::Blue, 300); // 起動時

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    flashLED(CRGB::Red, 500);
    return;
  }

  connect_wifi();

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }
  Serial.println(ctime(&now));

  if (connect_awsiot()) {
    flashLED(CRGB::Green, 300);
  }
}

unsigned long lastGpsSendTime = 0;
unsigned long gpsInterval = 20000; // 20秒間隔

void loop() {
  mqtt_client.loop();

  while (GPS_serial.available()) {
    gps.encode(GPS_serial.read());
  }

  unsigned long nowMillis = millis();
  if (gps.location.isValid() && nowMillis - lastGpsSendTime > gpsInterval) {
    lastGpsSendTime = nowMillis;

    StaticJsonDocument<200> doc;
    
    if (gps.location.isValid()) {
      doc["lat"] = gps.location.lat();
      doc["lon"] = gps.location.lng();
      doc["alt"] = gps.altitude.meters();
      doc["time"] = gps.time.value();
      doc["status"] = "ok";
      flashLED(CRGB::Orange, 200);
    } else {
      doc["status"] = "no_fix";
      doc["message"] = "No valid GPS signal";
      flashLED(CRGB::Blue, 100);  // GPS未取得状態を示す色（例: 青）
    }

    char json[200];
    serializeJson(doc, json);
    mqtt_client.publish(PUBLISH_TOPIC, json);
    Serial.println(json);
  }

  if (WiFi.status() != WL_CONNECTED) {
    connect_wifi();
  }

  if (!mqtt_client.connected()) {
    connect_awsiot();
  }

  delay(100);
}
