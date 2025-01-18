#include <ESP8266WiFi.h>
#include <U8g2lib.h>
 
#define SDA 2                       // SDA引脚，默认gpio4(D2)
#define SCL 14                       // SCL引脚，默认gpio5(D1)
int num =0;
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*clock=*/SCL, /*data=*/SDA, /*reset=*/U8X8_PIN_NONE);           // 选择显示屏幕
 
 
void setup(){
  u8g2.begin();               // 初始化
  u8g2.enableUTF8Print();     // UTF8允许
 
  u8g2.clearBuffer();         // 清除缓存，其实初始化里有清除，循环时一定要加上
 
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);    // 选择中文gb2313b
  u8g2.setCursor(0,14);             // 缓存区定位
  u8g2.print("这是一条测试程序");        // 指定缓存区需要打印的字符串    
  u8g2.sendBuffer();          // 将定位信息发送到缓冲区

}
 
void loop(){
  u8g2.clearBuffer();
  delay(3000);
  u8g2.setCursor(0,14);             // 缓存区定位
  u8g2.print("测试程序版本 Ver0.2");        // 指定缓存区需要打印的字符串
  u8g2.setCursor(0,31);             // 缓存区定位
  u8g2.print("测试进程 : ");        // 指定缓存区需要打印的字符串
  u8g2.print(num);        // 指定缓存区需要打印的字符串        
  u8g2.sendBuffer();          // 将定位信息发送到缓冲区
  num++;
}