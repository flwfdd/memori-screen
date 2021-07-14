/*
 * @Author: flwfdd
 * @Date: 2021-07-01 16:50:40
 * @LastEditTime: 2021-07-09 22:50:40
 * @Description: 外围硬件及传感器
 * _(:з」∠)_
 */

#ifndef MEMORI_SENSOR_H
#define MEMORI_SENSOR_H

#include "RTClib.h"
#include "Wire.h"

struct environment
{
    String P; //气压 Pa
    String L; //亮度 lx
    String T; //温度 ℃
    String H; //湿度 %
    String A; //海拔 m
};            //环境

extern environment env;
extern DateTime now_time;
const char daysOfTheWeek[7][5] = {"Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat"};

void sensor_init(void); //初始化
void sensor_update(void);
void set_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);

#endif