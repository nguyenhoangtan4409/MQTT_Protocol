#include <WiFi.h>
#include <PubSubClient.h>
#include "src/my_BMP280/my_BMP280.h"

const char* ssid = "TanTanLap";
const char* pass = "12345567899";

#define HOSTNAME "mqttx_MQTTLab4"

const char *MQTT_HOST = "broker.emqx.io";
const int MQTT_PORT = 1883;
const char *MQTT_USER = "TanTan"; // có thể để trống nếu không yêu cầu xác thực
const char *MQTT_PASS = "12345567899"; 
const char MQTT_SUB_TOPIC[] = "home/MQTTLab4/in";
const char MQTT_PUB_TOPIC[] = "home/MQTTLab4/out";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

#define LED_PIN  2
int current_ledState = LOW;
int last_ledState = LOW;

unsigned long lastSendTime = 0;
float Temp = 0;

void setup_wifi() 
{
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.setHostname(HOSTNAME);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
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

void callback(char* topic, byte *payload, unsigned int length) 
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

  setup_wifi();

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
  
   if (millis() - lastSendTime > 1000) { // Gửi dữ liệu mỗi giây (1000ms)
    send_data();
    lastSendTime = millis(); // Lưu timestamp của lần gửi dữ liệu mới
  }

}
