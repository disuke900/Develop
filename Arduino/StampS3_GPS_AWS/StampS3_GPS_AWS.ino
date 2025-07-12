#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <LittleFS.h>
#include <FastLED.h>
#include <FS.h>

// // Wi-Fi設定（iPhoneテザリングのSSIDとパスワードを入力）
// const char* ssid = "iphone1";
// const char* password = "qwertyui";

// Wi-Fi設定
const char* ssid = "neko01";
const char* password = "64646631";

// AWS IoT エンドポイント（自分のに置き換えて）
const char* awsEndpoint = "d0518902q7so9rnorfjf-ats.iot.us-east-1.amazonaws.com";

// デバイス証明書ファイル
const char* certFile = "/certificate.pem.crt";
const char* keyFile  = "/private.pem.key";
const char* caFile   = "/AmazonRootCA1.pem";

// MQTT設定
const char* mqttTopic = "daisuke/gps";
WiFiClientSecure net;
PubSubClient client(net);

// GPS設定
HardwareSerial GPSSerial(1);
TinyGPSPlus gps;

// LED設定
#define LED_PIN 21
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// 送信間隔
unsigned long lastSend = 0;
unsigned long interval = 1000;

void flashLED(CRGB color, int duration_ms) {
  leds[0] = color;
  FastLED.show();
  delay(duration_ms);
  leds[0] = CRGB::Black;
  FastLED.show();
}

void connectAWS() {

  // ルートCA読み込み
  File ca = LittleFS.open(caFile, "r");
  if (!ca) {
    Serial.println("CAファイルが開けません");
    return;
  }
  // CA証明書を設定
  size_t size = ca.size();
  char* buf = (char*)malloc(size + 1);
  if (buf) {
    ca.readBytes(buf, size);
    buf[size] = '\0';
    net.setCACert(buf);
    free(buf);
  }
  ca.close();

  // クライアント証明書を設定
  File cert = LittleFS.open(certFile, "r");
  if (cert) {
    size_t size = cert.size();
    char* buf = (char*)malloc(size + 1);
    if (buf) {
      cert.readBytes(buf, size);
      buf[size] = '\0';
      net.setCertificate(buf);
      free(buf);
    }
    cert.close();
  }

  // 秘密鍵を設定
  File key = LittleFS.open(keyFile, "r");
  if (key) {
    size_t size = key.size();
    char* buf = (char*)malloc(size + 1);
    if (buf) {
      key.readBytes(buf, size);
      buf[size] = '\0';
      net.setPrivateKey(buf);
      free(buf);
    }
    key.close();
  }
}

void reconnectAWS() {
  connectAWS();  // 証明書の読み込み

  client.setServer(awsEndpoint, 8883);

  while (!client.connected()) {
    Serial.print("AWS IoTへ再接続中...");
    if (client.connect("M5StampS3Client")) {
      Serial.println("接続成功");
    } else {
      Serial.printf("失敗 rc=%d\n", client.state());
      delay(2000);
    }
  }
}

//Check LittleFS data
void printFileContent(const char* filename) {
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.printf("ファイル %s が開けません\n", filename);
    return;
  }
  Serial.printf("ファイル %s の内容:\n", filename);
  while (file.available()) {
    Serial.write(file.read());
  }
  Serial.println();
  file.close();
}

void listFiles() {
  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("ディレクトリを開けませんでした");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    Serial.print("File: ");
    Serial.println(file.name());
    file = root.openNextFile();
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("起動開始");
  
  // Little-fsを初期化
  if (!LittleFS.begin()) {
    Serial.println("LittleFSマウント失敗");
    return;
  }
  listFiles();

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB::Black;
  FastLED.show();

  WiFi.begin(ssid, password);
  Serial.print("WiFi接続中");

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi接続成功");
    flashLED(CRGB::Yellow, 300);
  } else {
    Serial.println("WiFi接続失敗");
    flashLED(CRGB::Red, 500);
    return;
  }

  printFileContent("/certificate.pem.crt");
  printFileContent("/private.pem.key");
  printFileContent("/AmazonRootCA1.pem");

  GPSSerial.begin(9600, SERIAL_8N1, 44, 43);
  client.setServer(awsEndpoint, 8883);

  // setup内で1回接続
  reconnectAWS();

}

void loop() {
  // loop内でも接続を確認
  if (!client.connected()) {
    reconnectAWS();
  }
  client.loop();

  bool hasRaw = false;
  while (GPSSerial.available()) {
    char c = GPSSerial.read();
    Serial.write(c);
    gps.encode(c);
    hasRaw = true;
  }

  unsigned long now = millis();
  if (now - lastSend < interval) return;
  lastSend = now;

  if (!hasRaw) {
    Serial.println("GPS信号なし");
    flashLED(CRGB::Red, 200);
    interval = 1000;
    return;
  }

  if (!gps.location.isValid()) {
    Serial.println("位置情報無効");
    flashLED(CRGB::Orange, 200);
    interval = 1000;
    return;
  }

  double lat = gps.location.lat();
  double lng = gps.location.lng();
  Serial.printf("緯度: %.6f 経度: %.6f\n", lat, lng);

  // メッセージ作成
  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"lat\": %.6f, \"lng\": %.6f}", lat, lng);

  // MQTT Publish
  if (client.publish(mqttTopic, payload)) {
    Serial.println("送信成功");
    flashLED(CRGB::Blue, 200);
  } else {
    Serial.println("送信失敗");
    flashLED(CRGB::Red, 200);
  }

  interval = 20000;
}