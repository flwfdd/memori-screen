/*
 * @Author: flwfdd
 * @Date: 2021-06-27 15:47:22
 * @LastEditTime: 2021-07-13 20:38:19
 * @Description: 显卡驱动（雾
 * _(:з」∠)_
 */
#ifndef MEMORI_SCREEN_H
#define MEMORI_SCREEN_H

#include "Arduino.h"
#include "TFT_eSPI.h"
#include "memori_sensor.h"
#include "ArduinoJson.h"

//配置字体文件存储位置
#define USE_SD
//#define USE_SPIFFS

#ifdef USE_SD
#include "SD.h"
extern fs::SDFS mFS;
#endif
#ifdef USE_SPIFFS
#include "SPIFFS.h"
extern fs::SPIFFSFS mFS;
#endif

extern TFT_eSPI tft;

class Font
{
public:
    String font_type = "FZQK";     //字体
    uint32_t size = 32;            //字号
    uint32_t color = TFT_WHITE;    //字色
    uint32_t bg_color = TFT_BLACK; //背景色
    bool opacity = false;          //背景不透明
    void set_by_json(JsonObject dic);
};

class Screen
{
public:
    uint32_t CS_PIN;                                           //片选引脚
    uint32_t rotation = 0;                                     //屏幕旋转
    String template_string;                                    //模板字符串
    String draw_string;                                        //绘制字符串
    DynamicJsonDocument draw_json = DynamicJsonDocument(1024); //绘制JSON
    bool need_draw = false;                                    //需要绘制
    Screen(uint32_t CS);
    Screen(uint32_t CS, uint32_t rt);
    void init(void);
    void enable(void);
    void push(void);
};

extern Screen scr[5];

void draw_char(String s, int32_t x, int32_t y, Font ft);
void draw_char_center(String s, Font ft);
void draw_char_center_x(String s, int32_t y, Font ft);
void draw_string(String s, int32_t x, int32_t y, Font ft, bool auto_newline);
void draw_img(String path);
// void draw_img(String path, uint32_t w = tft.width(), uint32_t h = tft.height(), int32_t x = 0, int32_t y = 0);
void draw_color(uint16_t color);
void draw_by_json(JsonArray dic);
uint16_t color_hex(String s);
void screen_update();
void screen_data_update();
void screen_init();
void screen_file_update();

#endif