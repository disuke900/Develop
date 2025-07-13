#include <WiFi.h>
#include <time.h>
#include <HardwareSerial.h>
#include <FastLED.h>

// LED設定
#define LED_PIN 21
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];


void setLED(CRGB color) {
  leds[0] = color;
  FastLED.show();
}

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
#define WIFI_PASSPHRASE "64646631"
#define DEVICE_NAME "sample002"
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


// WiFi接続
void connect_wifi() {
    Serial.print("WiFi Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);

    int retry_count = 0;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        flashLED(CRGB::Red, 100);
        delay(500);

        retry_count++;
        if (retry_count >= 10) {  // 500ms × 10 = 5秒
            Serial.println("\nWiFi connection failed. Retrying in 5 seconds...");
            WiFi.disconnect();           // 念のため切断
            delay(5000);                // 5秒待って
            WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);  // 再接続
            retry_count = 0;            // リセット
        }
    }

    Serial.println("\nConnected");
    Serial.println("IPv4: " + WiFi.localIP().toString());
    Serial.println("");

    flashLED(CRGB::Green, 200);
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
        if (mqtt_client.connect(DEVICE_NAME)) {
            Serial.println("Connected");
            flashLED(CRGB::Yellow, 500);
            return true;
        } else {
            Serial.println("Failed, rc=" + String(mqtt_client.state()));
            Serial.println("");
            flashLED(CRGB::Red, 100);
            return false;
        }
    }
    return false;
}

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  setLED(CRGB::Black);
  flashLED(CRGB::Blue, 200);
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

  Serial.printf("root_ca= %s\n", root_ca);
  Serial.printf("certificate= %s\n", certificate);
  Serial.printf("private_key= %s\n", private_key);

  if (connect_awsiot()) { // AWS IoT 接続
    flashLED(CRGB::Green, 300);
  }

}

// 点灯の状態
bool blink = false;

int publishCount = 0;
unsigned long lastPublishTime = 0;

void loop() {
    mqtt_client.loop();

    unsigned long nowMillis = millis();

    if (publishCount < 10 && nowMillis - lastPublishTime >= 5000) {
        lastPublishTime = nowMillis;

        time_t now = time(nullptr);
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "{\"time\": \"%s\"}", ctime(&now));

        mqtt_client.publish("mqtt/test/response", buffer);
        Serial.println("Published to mqtt/test/response:");
        Serial.println(buffer);
        flashLED(CRGB::Orange, 200);
        publishCount++;
    }
    delay(100); // 軽めのdelay
}