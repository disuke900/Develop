#include <Arduino.h>

#include <WiFi.h>
#include <Ambient.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <FastLED.h>

// Wi-Fi設定
const char* ssid = "iphone1";
const char* password = "qwertyui";

// Ambient設定
unsigned int channelId = 92053;
const char* writeKey = "28e23b4f6a09fdf4";

// LED設定
#define LED_PIN 21
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// GPS設定
HardwareSerial GPSSerial(1);
TinyGPSPlus gps;

WiFiClient client;
Ambient ambient;

unsigned long lastSendTime = 0;
unsigned long interval = 1000; // 初期は1秒ごとにチェック

void flashLED(CRGB color, int duration_ms) {
  leds[0] = color;
  FastLED.show();
  delay(duration_ms);
  leds[0] = CRGB::Black;
  FastLED.show();
}

void setLED(CRGB color) {
  leds[0] = color;
  FastLED.show();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== M5StampS3 GPS + Ambient + LED状態表示 ===");

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  setLED(CRGB::Black);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");
  flashLED(CRGB::Green, 500); // 緑点灯：WiFi接続成功

  GPSSerial.begin(9600, SERIAL_8N1, 44, 43); // GPS: RX=44, TX=43
  ambient.begin(channelId, writeKey, &client);
}

void loop() {

  // Wi-Fiが切断されたら再接続を試みる
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi再接続中...");
    WiFi.begin(ssid, password);
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) { // 10秒以内に接続
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWi-Fi再接続成功");
      flashLED(CRGB::Green, 300);
    } else {
      Serial.println("\nWi-Fi再接続失敗");
      flashLED(CRGB::Red, 300);
      return; // Wi-Fi接続できない場合は送信をスキップ
    }
  }

  bool hasRawData = false;

  unsigned long now = millis();
  if (now - lastSendTime < interval) {
    delay(10);
    return;
  }

  lastSendTime = now;

  // GPSデータ読み取り
  while (GPSSerial.available()) {
    char c = GPSSerial.read();
    Serial.write(c); // NMEAデータを表示
    gps.encode(c);
    hasRawData = true;
  }

  if (!hasRawData) {
    flashLED(CRGB::Red, 300); //🔴 データがまったく来ていない
    Serial.println("GPS未受信: データなし");
    interval = 1000;
    return;
  }

  if (!gps.location.isValid()) {
    flashLED(CRGB::Orange, 300); //🟠 データはあるが位置情報なし
    Serial.println("GPS未受信: 位置情報なし");
    interval = 1000;
    return;
  }

  // 🔵 緯度経度取得成功
  double lat = gps.location.lat();
  double lon = gps.location.lng();
  Serial.println("=== GPS位置情報取得 ===");
  Serial.printf("緯度: %.6f\n", lat);
  Serial.printf("経度: %.6f\n", lon);
  flashLED(CRGB::Blue, 300);

  // Ambient送信（数値として登録）
  ambient.set(1, lat);
  ambient.set(2, lon);

  // Ambient送信用に文字列に変換（地図表示用）
  char latbuf[12], lonbuf[12];
  dtostrf(lat, 11, 7, latbuf);
  dtostrf(lon, 11, 7, lonbuf);
  ambient.set(9, latbuf);  // 緯度（文字列）
  ambient.set(10, lonbuf); // 経度（文字列）

  if (ambient.send()) {
    Serial.println("Ambient送信成功");
    flashLED(CRGB::Yellow, 300); // 🟡 送信成功
  } else {
    Serial.println("Ambient送信失敗");
  }

  // 取得成功なら20秒スリープ
  interval = 20000;
}