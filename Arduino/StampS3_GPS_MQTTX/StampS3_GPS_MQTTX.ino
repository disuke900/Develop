#include <WiFi.h>
#include <Ambient.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <FastLED.h>

// WiFi設定（iPhoneテザリングでもOK）
const char* ssid     = "iphone1";
const char* password = "qwertyui";

// Ambient設定
unsigned int channelId = 92012;
const char* writeKey = "13fc706f77fa090f";

// LED設定（内蔵WS2812）
#define LED_PIN 21
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// GPS設定（GPIO20: RX ← GPS TX, GPIO21: TX → GPS RX ※TXは接続しなくてOK）
HardwareSerial GPSSerial(1);
TinyGPSPlus gps;

WiFiClient client;
Ambient ambient;

// LEDを指定色で点灯（duration_ms ミリ秒）
void flashLED(CRGB color, int duration_ms) {
  leds[0] = color;
  FastLED.show();
  delay(duration_ms);
  leds[0] = CRGB::Black; // 消灯
  FastLED.show();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== M5StampS3 GPS → Ambient with FastLED ===");

  // LED初期化
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB::Black;
  FastLED.show();

  // Wi-Fi接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");
  flashLED(CRGB::Green, 500); // 緑点灯：WiFi接続成功

  // GPS初期化（9600bps）
  GPSSerial.begin(9600, SERIAL_8N1, 44, 43);

  // Ambient初期化
  ambient.begin(channelId, writeKey, &client);
}

void loop() {
  // GPSデータ読み取り
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }

  if (gps.location.isUpdated()) {
    double lat = gps.location.lat();
    double lon = gps.location.lng();

    Serial.println("=== GPS OK ===");
    Serial.print("Lat: "); Serial.println(lat, 6);
    Serial.print("Lon: "); Serial.println(lon, 6);
    flashLED(CRGB::Blue, 300); // 青点灯：GPS更新成功

    // Ambient送信
    ambient.set(1, lat);
    ambient.set(2, lon);
    if (ambient.send()) {
      Serial.println("Ambient送信成功");
      flashLED(CRGB::Yellow, 300); // 黄点灯：送信成功
    } else {
      Serial.println("Ambient送信失敗");
    }
  }

  delay(20000); // 20秒間隔送信
}