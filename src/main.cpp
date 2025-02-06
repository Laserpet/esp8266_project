#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#define SDA 2                       // SDA引脚，默认gpio4(D2)
#define SCL 14                       // SCL引脚，默认gpio5(D1)

enum pumpState {
    PUMP_OFF,
    PUMP_ON
};


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
    u8g2.print("PowerStats V0.2");        // 指定缓存区需要打印的字符串    
    u8g2.sendBuffer();          // 将定位信息发送到缓冲区
    connectToWiFi();
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setCallback(mqttCallback);
    connectToMQTTBroker();
    u8g2.clearBuffer();
    u8g2.setCursor(0,14);             // 缓存区定位
    u8g2.print("MQTT CONNECTED");        // 指定缓存区需要打印的字符串
    u8g2.sendBuffer();
    pinMode(4, OUTPUT); // Set GPIO4 as output
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    u8g2.clearBuffer();
    u8g2.setCursor(0,14);             // 缓存区定位
    u8g2.print("Connecting to WiFi");
    u8g2.sendBuffer();
    uint8_t count = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        u8g2.setCursor(count,21);
        u8g2.print(".");
        u8g2.sendBuffer();
        count += 6;  
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
            u8g2.setFont(u8g2_font_wqy12_t_gb2312);
            mqtt_client.publish(mqtt_topic, "Hi Smart_Home I'm ESP8266 ^^");
        } else {
            u8g2.clearBuffer();
            u8g2.setCursor(0,14);
            u8g2.print("Failed to connect");
            u8g2.setCursor(0,30);
            u8g2.print("to MQTT broker, rc=");
            u8g2.setCursor(0,46);
            u8g2.print(mqtt_client.state());
            u8g2.setCursor(0,62);
            u8g2.println(" try again in 5 seconds");
            u8g2.sendBuffer();
            delay(5000);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    String message;
    bool has_Family_total_power = false;
    String Family_total_power;
    String Family_Power_consumption;

    bool has_Family_Power_consumption = false;
    bool has_pump_control = false;

    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
        message += (char) payload[i];
    }
    if (message == "start_pump") {
        digitalWrite(4, HIGH); // Set GPIO4 high
    } else if (message == "stop_pump") {
        digitalWrite(4, LOW); // Set GPIO4 low
    }
    u8g2.clearBuffer();
    delay(100);
    u8g2.setCursor(0,14);             // 缓存区定位

    if (message.startsWith("_sensor_")) {
        int startIndex = message.indexOf('"') + 1;
        int endIndex = message.indexOf('"', startIndex);
        String sensorName = message.substring(startIndex, endIndex);
        String value = message.substring(endIndex + 2); // Skip ": "
        
        if (sensorName == "Family_total_power") {
            has_Family_total_power = true;
            Family_total_power = value;
                // 打印value值
        }
        if (sensorName == "Family_Power_consumption") {
            u8g2.setFont(u8g2_font_wqy12_t_gb2312);
            u8g2.setCursor(0,14);
            u8g2.print("Family_Power_consumption: ");
            u8g2.setFont(u8g2_font_fub30_tf);
            u8g2.setCursor(0,50);             // 缓存区定位
            u8g2.print(value);                // 打印value值
        }        
    } else if (message.startsWith("_action_")) {
        int startIndex = message.indexOf('"') + 1;
        int endIndex = message.indexOf('"', startIndex);
        String actionName = message.substring(startIndex, endIndex);
        String actionValue = message.substring(endIndex + 2); // Skip ": "
        
        if (actionName == "pump_control") {
            if (actionValue == "PUMP_ON") {
                digitalWrite(4, HIGH); // Set GPIO4 high
            } else if (actionValue == "PUMP_OFF") {
                digitalWrite(4, LOW); // Set GPIO4 low
            }
        }
    } else {
        bool isNumeric = true;
        for (unsigned int i = 0; i < message.length(); i++) {
            if (!isDigit(message[i])) {
                isNumeric = false;
                break;
            }
        }
        if (isNumeric) {
            u8g2.setFont(u8g2_font_fub30_tf);
        } else {
            u8g2.setFont(u8g2_font_wqy12_t_gb2312);
        }
        u8g2.print(message);        // 打印整个消息
    }   

        u8g2.setFont(u8g2_font_wqy12_t_gb2312);
        u8g2.setCursor(0,14);
        if(has_Family_total_power){
            u8g2.print("Family_total_power: ");
            u8g2.setFont(u8g2_font_fub30_tf);
            u8g2.setCursor(0,50);             // 缓存区定位    
            u8g2.print(Family_total_power);                // 打印value值        
            has_Family_total_power = false;
        }else{
            if(has_Family_Power_consumption){
                u8g2.print("Family_Power_consumption: ");
                u8g2.setFont(u8g2_font_fub30_tf);
                u8g2.setCursor(0,50);             // 缓存区定位
                u8g2.print(Family_Power_consumption);                // 打印value值
                has_Family_Power_consumption = false;
            }
        }

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
