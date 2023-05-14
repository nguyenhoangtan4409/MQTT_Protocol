#include <WiFi.h>
#include "src/WiFiClientSecure/WiFiClientSecure.h"
#include <time.h>
#include <PubSubClient.h>
#include "src/my_BMP280/my_BMP280.h"

  const char* ssid = "TanTanLap";
  const char* pass = "12345567899";

  #define HOSTNAME "MQTTLab4"

const char *MQTT_HOST = "broker.emqx.io";
const int MQTT_PORT = 8883;
const char *MQTT_USER = "TanTan"; // có thể để trống nếu không yêu cầu xác thực
const char *MQTT_PASS = "12345567899"; 
const char MQTT_SUB_TOPIC[] = "home/MQTTLab4/in";
const char MQTT_PUB_TOPIC[] = "home/MQTTLab4/out";

const char* local_root_ca = \
    "-----BEGIN CERTIFICATE-----\n" \ 
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \ 
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \ 
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \ 
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \ 
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \ 
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \ 
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \ 
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \ 
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \ 
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \ 
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \ 
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \ 
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \ 
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \ 
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \ 
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \ 
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \ 
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \ 
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \ 
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \ 
    "-----END CERTIFICATE-----\n" ;



#define LED_PIN  2
int current_ledState = LOW;
int last_ledState = LOW;

float Temp = 0;

WiFiClientSecure net;
PubSubClient client(net);

time_t now;
unsigned long lastSendTime = 0;
void setup_wifi()
{
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.setHostname(HOSTNAME);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}
void connect_to_broker()
{
    while (!client.connected()) {
    Serial.print("Time:");
    Serial.print(ctime(&now));
    Serial.print("Attempting MQTT connection...");
    if (client.connect(HOSTNAME, MQTT_USER, MQTT_PASS)) {
      Serial.println("connected");
      client.subscribe(MQTT_SUB_TOPIC);
    } else {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 3 seconds");
      delay(3000);
    }
  }
  
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, length);
  Serial.println();

  if (*payload == '1') current_ledState = HIGH;
  if (*payload == '0') current_ledState = LOW;
}


void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(500);

  setup_wifi();

  Serial.print("Setting time using SNTP");
  configTime(7 * 3600, 0, "0.vn.pool.ntp.org", "1.vn.pool.ntp.org");

  now = time(nullptr);
  while (now < 1684036922) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  net.setCACert(local_root_ca);

  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callback);
  connect_to_broker();

  pinMode(LED_PIN, OUTPUT);
  init_BMP280(0x76);
  setup_BMP280(normal, SAMPLING_X2, SAMPLING_NONE, FILTER_X16, STANDBY_MS_500);
  read_Compensation_parameter_storage();
}
void send_data() 
{
  float Temp = readTemperaturee();
  Serial.printf("Nhiệt độ: %0.2f doC\n",Temp);
  char TempStr[10];
  dtostrf(Temp, 5, 2, TempStr);
  client.publish(MQTT_PUB_TOPIC, TempStr); 
  delay(500);
}
void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Checking wifi");
    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(10);
    }
    Serial.println("connected");
  }
  else
  {
    if (!client.connected())
    {
      connect_to_broker();
    }
    else
    {
      client.loop();
    }
  }

  if (last_ledState != current_ledState) 
  {
    last_ledState = current_ledState;
    digitalWrite(LED_PIN, current_ledState);
    Serial.printf("Trạng thái LED %d\n",current_ledState);
  }

  if (millis() - lastSendTime > 1000) {
    lastSendTime = millis();
    send_data();
  }
}
