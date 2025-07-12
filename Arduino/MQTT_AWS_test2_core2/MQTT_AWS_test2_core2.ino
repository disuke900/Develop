#include <M5Core2.h>
#include <WiFi.h>
#include <FastLED.h>
#include <time.h>

// LED設定
#define LED_PIN 21
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

void flashLED(CRGB color, int duration_ms) {
  leds[0] = color;
  FastLED.show();
  delay(duration_ms);
  leds[0] = CRGB::Black;
  FastLED.show();
}

// AWS IoT 
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#define WIFI_SSID "neko01"
#define WIFI_PASSPHREASE "64646631"
#define DEVICE_NAME "sample002"
#define AWS_IOT_ENDPOINT "d0518902q7so9rnorfjf-ats.iot.us-east-1.amazonaws.com"
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


// WiFi接続
void connect_wifi() {
    Serial.print("WiFi Connecting to ");
    Serial.println(WIFI_SSID);
    M5.Lcd.print("WiFi Connecting to ");
    M5.Lcd.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSPHREASE);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        M5.Lcd.print(".");
        delay(500);
    }
    Serial.println("Connected");
    Serial.println("IPv4: " + WiFi.localIP().toString());
    Serial.println("");
    M5.Lcd.println("Connected");
    M5.Lcd.printf("IPv4: %s", WiFi.localIP().toString().c_str());
    M5.Lcd.println("");
}

// AWS IoT 接続
bool connect_awsiot() {
    https_client.setCACert(root_ca);
    https_client.setCertificate(certificate);
    https_client.setPrivateKey(private_key);

    // SNIを有効化（重要）
    https_client.setHandshakeTimeout(5);
    mqtt_client.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);

    while (!mqtt_client.connected())  {
        Serial.println("MQTT connection...");
        M5.Lcd.println("MQTT connection...");
        if (mqtt_client.connect(DEVICE_NAME)) {
            Serial.println("Connected");

            mqtt_client.subscribe(SUBSCRIBE_TOPIC, QOS);
            Serial.println("Subscribed.");
            M5.Lcd.println("Subscribed.");
            delay(1000); // 表示したメッセージ確認できるようにするため

            return true;
        } else {
            Serial.println("Failed, rc=" + String(mqtt_client.state()));
            Serial.println("");
            M5.Lcd.printf("Failed, rc=%d", mqtt_client.state());
            M5.Lcd.println("");
            return false;
        }
    }
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB::Black;
  FastLED.show();
  
  connect_wifi(); // WiFi 接続

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync...");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println(" done.");
  Serial.println(ctime(&now));

  M5.Lcd.printfln("root_ca= ", root_ca)
  M5.Lcd.printfln("certificate= ", certificate)
  M5.Lcd.printfln("private_key= ", private_key)
  Serial.println("root_ca= "+ root_ca)
  Serial.println("certificate= "+ certificate)
  Serial.println("private_key= "+ private_key)

  if (connect_awsiot()) { // AWS IoT 接続
      flashLED(CRGB::Green, 300);
      M5.Lcd.fillScreen(WHITE);
  }
}

// 点灯の状態
bool blink = false;

// 現在の点灯状態をpublichする
void publish() {
    StaticJsonDocument<200> json_document;
    char json_string[100];
    json_document["blink"] = blink;
    serializeJson(json_document, json_string);
    mqtt_client.publish(PUBLISH_TOPIC, json_string);
}


void loop() {
    mqtt_client.loop(); // Subscribeする時は、必須
    publish(); // 現在の状態をpublishする
    delay(500);
}