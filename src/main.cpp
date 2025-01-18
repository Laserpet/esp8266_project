#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#define SDA 2                       // SDA引脚，默认gpio4(D2)
#define SCL 14                       // SCL引脚，默认gpio5(D1)
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*clock=*/SCL, /*data=*/SDA, /*reset=*/U8X8_PIN_NONE);           // 选择显示屏幕
// WiFi settings
const char *ssid = "Smart_Home";             // Replace with your WiFi name
const char *password = "cyz19950124";        // Replace with your WiFi password

// MQTT Broker settings
const char *mqtt_broker = "homeassistant.chenyz.com.cn";  // EMQX broker endpoint
const char *mqtt_topic = "smart_home/esp8266";     // MQTT topic
const char *mqtt_username = "chenyz";          // MQTT username for authentication
const char *mqtt_password = "cyz19950124";        // MQTT password for authentication
const int mqtt_port = 1883;                  // MQTT port (TCP)
// char payload[100];
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void connectToWiFi();
void connectToMQTTBroker();
void mqttCallback(char *topic, byte *payload, unsigned int length);

void setup() {
    Serial.begin(115200);
    u8g2.begin();               // 初始化
    u8g2.enableUTF8Print();     // UTF8允许
  
    u8g2.clearBuffer();         // 清除缓存，其实初始化里有清除，循环时一定要加上
  
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);    // 选择中文gb2313b
    u8g2.setCursor(0,14);             // 缓存区定位
    u8g2.print("功耗仪 V0.01");        // 指定缓存区需要打印的字符串    
    u8g2.sendBuffer();          // 将定位信息发送到缓冲区
    connectToWiFi();
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setCallback(mqttCallback);
    connectToMQTTBroker();
    
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to the WiFi network");
}

void connectToMQTTBroker() {
    while (!mqtt_client.connected()) {
        String client_id = "LittleEsp8266" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT broker");
            mqtt_client.subscribe(mqtt_topic);
            // Publish message upon successful connection
            mqtt_client.publish(mqtt_topic, "Hi Smart_Home I'm ESP8266 ^^");
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    u8g2.clearBuffer();
    delay(1000);
    u8g2.setCursor(0,14);             // 缓存区定位
    u8g2.print("功耗记录程序 Ver0.2");        // 指定缓存区需要打印的字符串
    u8g2.setCursor(0,31);             // 缓存区定位
    for (unsigned int i = 0; i < length; i++) {
          u8g2.print((char) payload[i]);
      }        // 指定缓存区需要打印的字符串       
    u8g2.sendBuffer();          // 将定位信息发送到缓冲区
      Serial.println();
      Serial.println("-----------------------");
}

void loop() {
    if (!mqtt_client.connected()) {
        connectToMQTTBroker();
    }
    mqtt_client.loop();
}
