#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <LittleFS.h>

const char* ssid = "neko01";
const char* password = "64646631";
const char* host = "d0518902q7so9rnorfjf-ats.iot.us-east-1.amazonaws.com";
const int httpsPort = 8883;

// デバイス証明書ファイル
const char* certFile = "/certificate.pem.crt";
const char* keyFile  = "/private.pem.key";
const char* caFile   = "/AmazonRootCA1.pem";

// MQTT設定
const char* mqttTopic = "daisuke/gps";
WiFiClientSecure net;
PubSubClient client(net);

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

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


  WiFiClientSecure client;
  client.setCACert(ca_cert); 
  client.setHandshakeTimeout(30);


  Serial.print("Connecting to ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(httpsPort);

  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
  } else {
    Serial.println("Connected successfully to port 8883!");
    client.stop();
  }
}

void loop() {
  // 何もしない
}
