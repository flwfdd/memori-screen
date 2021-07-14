/*
 * @Author: flwfdd
 * @Date: 2021-07-01 16:51:08
 * @LastEditTime: 2021-07-13 15:57:02
 * @Description: 
 * _(:з」∠)_
 */
#include "memori_sensor.h"

RTC_DS3231 rtc;
DateTime now_time;
environment env;

#define gy39_add 0x5b

void get_env(void)
{
    //uint32_t m;
    uint16_t data_16[2] = {0};
    uint8_t data[14] = {0};
    //iic_read(0x00, data, 14);
    Wire.beginTransmission(gy39_add);
    Wire.write(0);
    Wire.endTransmission(false);
    delayMicroseconds(10);
    Wire.requestFrom(gy39_add, 14);
    for (uint8_t i = 0; i < 14; i++)
    {
        data[i] = Wire.read();
    }
    //读取亮度
    data_16[0] = (data[0] << 8) | data[1];
    data_16[1] = (data[2] << 8) | data[3];
    env.L = String(((((uint32_t)data_16[0]) << 16) | data_16[1]) / 100) + "lx";
    //读取温度气压等
    env.T = String(((int16_t)(data[4] << 8) | data[5]) / 100.0, 1) + "℃";
    data_16[0] = (data[6] << 8) | data[7];
    data_16[1] = (data[8] << 8) | data[9];
    env.P = String(((((uint32_t)data_16[0]) << 16) | data_16[1]) / 100) + "Pa";
    env.H = String(((data[10] << 8) | data[11]) / 100.0, 1) + "%";
    env.A = String(((data[12] << 8) | data[13])) + "m";
}

//传感器初始化
void sensor_init(void)
{
    //时钟初始化
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
    }
    if (rtc.lostPower())
    {
        Serial.println("RTC lost power, let's set the time!");
    }
    //GY-39初始化
    Wire.begin();
}

//刷新数据
void sensor_update(void)
{
    now_time = rtc.now();
    get_env();
    // Serial.println(now_time.timestamp());
    // Serial.println(env.T);
    // Serial.println(env.A);
    // Serial.println(env.H);
    // Serial.println(env.L);
    // Serial.println(env.P);
}

//设置外置模块时间
void set_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
    Serial.printf("Set time:%d-%d-%d %d:%d:%d\n", year, month, day, hour, min, sec);
    rtc.adjust(DateTime(year, month, day, hour, min, sec));
}
