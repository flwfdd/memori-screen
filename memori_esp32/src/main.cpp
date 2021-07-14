/*
 * @Author: flwfdd
 * @Date: 2021-07-04 18:54:10
 * @LastEditTime: 2021-07-13 22:14:19
 * @Description: 
 * _(:з」∠)_
 */
#include "Arduino.h"
//#include "SPIFFS.h"
#include "SD.h"
#include "time.h"
#include "memori_screen.h"
#include "memori_sensor.h"
#include "memori_net.h"

#define SD_CS_PIN 13

//auto mFS=SPIFFS;
fs::SDFS mFS = SD;

//多线程循环
void loop0(void *p)
{
  while (1)
  {
    sensor_update();
    screen_data_update();
    delay(100);
  }
}

//主函数
void setup()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);

  //配置SD卡
  mFS = SD;
  if (mFS.begin(SD_CS_PIN, SPI, 25000000))
    Serial.println("SD Connected<(￣︶￣)>");

  //开启文件系统
  //SPIFFS.begin();

  //初始化wifi
  net_init();

  //初始化外置模块
  sensor_init();

  //初始化屏幕
  screen_init();

  //开启多核心
  xTaskCreatePinnedToCore(loop0, "loop0", 10000, NULL, 1, NULL, 0);
  File f=mFS.open("/test/test.txt","w");
  f.write('0');
  f.close();
}

uint32_t t0 = 0;
void loop()
{
  net_update();
  screen_update();
  if(millis()-t0>10)Serial.println(millis() - t0);
  t0 = millis();
}
