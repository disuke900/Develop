#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <FastLED.h>

WiFiClientSecure net;
PubSubClient client(net);

const char* ssid = "neko01";
const char* password = "64646631";

const char* aws_endpoint = "d0518902q7so9rnorfjf-ats.iot.us-east-1.amazonaws.com";
const int port = 8883;

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

// 証明書文字列（実際には長いので省略）
const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

const char client_cert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUesm1h9yTnmZqeT6USPXk7Lh0yCkwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MDcwNjA0MDMy
OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMdppqpkgx+0/zIgoNCT
E4eM9L0uRazG3PU/bSpKASaSUekpjf+UhoLa1LkI8iq96pom83XP3ZCjv49qq4hX
iuek0+lUssVCFWG56mieO1twfgAROFwRgi/POApMOU/Iw6vOtraj13kEknUQR0lj
E3KHPZ931tOJcbro+NnLKlKPRLhYHWPH20eoob3WpEv3gGooLRvL6efxGbOZfOE5
X6wIxZzGMbf1H7tv0ag0L4zpOVg/McAahxDYCjdoNbVbFyrv/WndwlxVYiRUO/Ow
6MF2JJU6yfBgO/ts1BuxaF6/iUEQr8inxJDmtPU+2Ac0w1Dqfu6tqu1LYTOTI7c6
wzcCAwEAAaNgMF4wHwYDVR0jBBgwFoAUhr8Rzf/0bvRQ2t/H9nWeqY7i9kwwHQYD
VR0OBBYEFB6Y2Hn18EPXOZxTZwVKw1/H71xyMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAQO91MwsjuuW7g34ZRY8gapZQw
G2ib4nrvyHB8V0CxzgqdJzfWCVezOHN2RChIaAH2z8JZ1I8I+qBy+UwuGOJZS3Zk
EOOpqzyY5Gsg7B8ydRVXrwGS5a95fHQMguO6lwDVtZUaGDvcE3sekJwR5w1GjWc/
gcXUwNKrvyygQxdb+86gaicnGR2M+2dFAFsDUMRd3FSKr0xFmDZuYgh132qmgE4F
VMy7oXRt/Of3qSpIlFhYQxJjsrs4dF60U6bSzYOTXdNsXzqMsq7W1yvm4JnWNUpO
+9UOVgoX+Hogw8Irz5fSc/3WuKtfaecA6zefVeP7yd+WLqqNreh/zWtEoZqG
-----END CERTIFICATE-----
)KEY";

const char client_key[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEAx2mmqmSDH7T/MiCg0JMTh4z0vS5FrMbc9T9tKkoBJpJR6SmN
/5SGgtrUuQjyKr3qmibzdc/dkKO/j2qriFeK56TT6VSyxUIVYbnqaJ47W3B+ABE4
XBGCL884Ckw5T8jDq862tqPXeQSSdRBHSWMTcoc9n3fW04lxuuj42csqUo9EuFgd
Y8fbR6ihvdakS/eAaigtG8vp5/EZs5l84TlfrAjFnMYxt/Ufu2/RqDQvjOk5WD8x
wBqHENgKN2g1tVsXKu/9ad3CXFViJFQ787DowXYklTrJ8GA7+2zUG7FoXr+JQRCv
yKfEkOa09T7YBzTDUOp+7q2q7UthM5MjtzrDNwIDAQABAoIBAQDAPnWDvYS/85E2
9Uez7KaePmDPZSWcrNblxHDsnQ3A7rzy6LHRnJGU43zCqgI0/ISIelap1GyI/aQ2
kiKuPZ9XNItrXdE+5DeS0FZQ54LX43pFfFR2RhbZGo+vNc7mwOkK4C07pHadX7AG
FVx0PoTIG+NnsnOz2cR/6gvGg9vYXDumMdT2OYo9sYOujlbr5nATtW42iOfK8/UL
q9xs18x/8o6J4Yl1Cg6zE8lLFyRHmZL2uIq66GaoZCbpawjFv1SrFC2tUttt/Ot9
mu7ep0T+H7vhdXYsHJqv4uyH4c0nakVA5bEpnQTaqd6ZsEG9nrI7gFrij5PsLbyC
tGOtwGqpAoGBAPKjfcoteRNytRqiZfSaMjlfaCj+lhq5lc5TRLub3gncu1BKhAFX
B3jUFCOnLRCE/0ZD80Be5wgFFuPJBoIfCei+tUhr0OJ+FdzpxvQRxOI8wW8kPuF2
/5btp34r/c1/RQJkUGosbyC1PmPdyrU9OcpxO/iCKxOTt7sGGiSVqbZ7AoGBANJk
zEbSQ/fm7SDczGkBPf6Q4WdU+KY6xsxT9QR2xrtvbnZzNC81mILwYCW7j6WXAncJ
7QjYn/ha6YeWlI0tL/d1Lz3CtXHUIAw55dcVrw39B+CFFG5wKycsRlY6lift3nu3
CWIeKdMZkBLO8DGYTpQixiI3xwdw+TjhlwkU9wd1AoGAReGWafM8ysAfrsMU8CS0
0FdNDXl9S8clgR30TsAua9K50/8SE7y39zHFdxs/3GEhkGAjn+d1TJcCp6oPCMzd
sr2yQmBi8ch0jYJ6/XJcrPfokhTzU0ZNiG977NfIqjW8b1rq2hlgDCCHDoYjSp2O
lmKUbhc3RtUldmTkhKLiFR8CgYBG6mbYcEMsqg+/hez4b8y/o4H2XHO7RBCCCsKJ
kKeHwhWPo2ExkHUpnXArB79xZycTjKry0mGJSstwQlcsBtEmLb+Qp7KU/ZUXyeQo
zUxGAMXI7dJnLzO8bnX+1pyKNOlK9fUZt2UNnI4taMcZUGccE2SJYBeb8H5wp8lK
7GAfiQKBgQCZj6mxaJK52COLpbRlvCAEYuz30whf+G849q6SzROXKSKU5v12XK0c
OKBl5+cSwGf/2ZmS+IT7o0rbtsGqYPW/UmuLXin1eftRqd6F4lFGuVPVJWG5JQxT
ihVe1wREV5cXGMvwh48x90hFZrS/Y5SLuuQ6RvOl/3MbZ25PDR+RAA==
-----END RSA PRIVATE KEY-----
)KEY";

void setup() {
  flashLED(CRGB::Pink, 200);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    flashLED(CRGB::Orange, 200);
    delay(500);
  } 

  flashLED(CRGB::Green, 200);
  net.setCACert(ca_cert);
  net.setCertificate(client_cert);
  net.setPrivateKey(client_key);

  client.setServer(aws_endpoint, port);
  client.connect("myClientID");
  client.publish("topic/test", "Hello from M5!");
}

unsigned long lastPublishTime = 0;
int publishCount = 0;

void loop() {
  client.loop();  // MQTT通信の維持

  unsigned long now = millis();

  if (publishCount < 5 && now - lastPublishTime >= 5000) {
    lastPublishTime = now;

    // 秒単位の現在時刻（起動からの経過時間）
    String msg = "Uptime: " + String(now / 1000) + " seconds";
    client.publish("topic/test", msg.c_str());
    Serial.println("Published: " + msg);
    flashLED(CRGB::Blue, 200);
    publishCount++;
  }

  // 5回送信したら何もせず待機（MQTTは維持）
}