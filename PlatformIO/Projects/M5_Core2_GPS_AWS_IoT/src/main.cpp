#include <M5Core2.h>
#include <WiFi.h>
#include <time.h>
#include <HardwareSerial.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>

bool is_fs_ready = false;

// LED設定
#define LED_PIN 21
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// GPS設定
HardwareSerial GPS_serial(1);
TinyGPSPlus gps;
#define GPS_RX 14
#define GPS_TX 13

// AWS IoT
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#define WIFI_SSID "neko01"
#define WIFI_PASSPHRASE "64646631"
#define DEVICE_NAME "M5Core2"
#define AWS_IOT_ENDPOINT "a1rf2bv1k9ed2q-ats.iot.us-east-1.amazonaws.com"
#define AWS_IOT_PORT 8883
#define PUBLISH_TOPIC DEVICE_NAME"/status"
#define SUBSCRIBE_TOPIC DEVICE_NAME"/blink"
#define QOS 0

WiFiClientSecure https_client;
PubSubClient mqtt_client(https_client);

int currentLine = 0;
const int lineHeight = 20;
const int maxLines = 10;

void logPrintln(const String &message) {
  Serial.println(message);

  if (currentLine >= maxLines) {
    M5.Lcd.fillScreen(BLACK);
    currentLine = 0;
  }

  M5.Lcd.setCursor(0, currentLine * lineHeight);
  M5.Lcd.println(message);
  currentLine++;
}

#include <string>
std::unique_ptr<char[]> readFileToCharArray(const char *path) {
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.printf("Failed to open file: %s\n", path);
    logPrintln("Failed to open file: " + String(path));
    return nullptr;
  }

  size_t size = file.size();
  std::unique_ptr<char[]> buf(new char[size + 1]);
  file.readBytes(buf.get(), size);
  buf[size] = '\0';
  file.close();
  return buf;
}

void setLED(CRGB color) {
  leds[0] = color;
  FastLED.show();
}

void flashLED(CRGB color, int duration_ms) {
  setLED(color);
  delay(duration_ms);
  setLED(CRGB::Black);
}

void M5DisplaySetup(){
  M5.Lcd.setRotation(1);        // 向き調整（お好みで）
  M5.Lcd.setTextSize(1);        // サイズ調整（お好みで）
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.fillScreen(BLACK);     // 初期化
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
  logPrintln("Wi-Fi Connected: " + WiFi.localIP().toString());
}

bool connect_awsiot() {
  if (!is_fs_ready) {
    logPrintln("LittleFS not ready, skipping AWS connect");
    return false;
  }

  auto ca = readFileToCharArray("/certs/AmazonRootCA1.pem");
  auto cert = readFileToCharArray("/certs/99c1-certificate.pem.crt");
  auto key = readFileToCharArray("/certs/99c1-private.pem.key");

  if (!ca || !cert || !key) {
    logPrintln("Certificate load failed");
    flashLED(CRGB::Red, 1000);
    return false;
  }

  https_client.setCACert(ca.get());
  https_client.setCertificate(cert.get());
  https_client.setPrivateKey(key.get());
  https_client.setHandshakeTimeout(5);
  mqtt_client.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);

  while (!mqtt_client.connected()) {
    if (mqtt_client.connect(DEVICE_NAME)) {
      mqtt_client.subscribe(SUBSCRIBE_TOPIC, QOS);
      flashLED(CRGB::Gold, 500);
      logPrintln("AWS IoT Connected");
      return true;
    } else {
      flashLED(CRGB::Purple, 100);
      logPrintln("AWS IoT Connect Failed");
      delay(2000);
    }
  }
  return false;
}

void setup() {
  M5.begin();
  M5DisplaySetup();
  Serial.begin(115200);

  GPS_serial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  setLED(CRGB::Black);
  flashLED(CRGB::Cyan, 300);
  logPrintln("Boot start...");

  if (!LittleFS.begin(true)) {
    logPrintln("LittleFS mount failed (even after format)");
    flashLED(CRGB::Red, 1000);
    is_fs_ready = false;
  } else {
    logPrintln("LittleFS mounted");
    is_fs_ready = true;
  }

  connect_wifi();

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }
  logPrintln("NTP Time Synced: " + String(ctime(&now)));

  if (connect_awsiot()) {
    flashLED(CRGB::Green, 300);
  }
}

unsigned long lastGpsSendTime = 0;
unsigned long gpsInterval = 20000;

void loop() {
  mqtt_client.loop();

  // GPSデータの読み取り
  while (GPS_serial.available()) {
    char c = GPS_serial.read();
    gps.encode(c);
  }

  // 20秒ごとに送信
  unsigned long nowMillis = millis();
  if (nowMillis - lastGpsSendTime > gpsInterval) {
    lastGpsSendTime = nowMillis;

    StaticJsonDocument<256> doc;
    char jsonBuffer[256];

    if (gps.location.isValid()) {
      doc["lat"] = gps.location.lat();
      doc["lon"] = gps.location.lng();
      doc["alt"] = gps.altitude.meters();

      flashLED(CRGB::Orange, 200);
    } else {
      doc["error"] = "GPS no fix";
      flashLED(CRGB::BlueViolet, 200);
    }

    // JSON化してpublish
    serializeJson(doc, jsonBuffer);
    mqtt_client.publish(PUBLISH_TOPIC, jsonBuffer);
    logPrintln("Published: " + String(jsonBuffer));
  }

  // Wi-Fi再接続
  if (WiFi.status() != WL_CONNECTED) {
    connect_wifi();
  }

  // MQTT再接続
  if (!mqtt_client.connected()) {
    connect_awsiot();
  }

  delay(100);
}