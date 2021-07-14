#include "memori_screen.h"

TFT_eSPI tft;
uint32_t mTFT_CS;
Screen scr[5] = {Screen(5, 3), 17, 16, 4, 0};
Font ft;

uint16_t img[135 * 250]; //屏幕缓存
uint8_t *img_tmp;        //屏幕缓存8位

//构造函数 传入片选引脚
Screen::Screen(uint32_t init_CS)
{
    CS_PIN = init_CS;
    pinMode(CS_PIN, OUTPUT);
}

//构造函数 传入片选引脚和屏幕旋转
Screen::Screen(uint32_t init_CS, uint32_t rt)
{
    CS_PIN = init_CS;
    pinMode(CS_PIN, OUTPUT);
    rotation = rt;
}

//使能屏幕
void Screen::enable(void)
{
    mTFT_CS = CS_PIN;
    if (tft.getRotation() != rotation)
        tft.setRotation(rotation);
}

//初始化屏幕
void Screen::init(void)
{
    enable();
    tft.init();
}

//推送屏幕
void Screen::push(void)
{
    enable();
    tft.pushImage(0, 0, tft.width(), tft.height(), img);
}

//使用JSON配置字体
void Font::set_by_json(JsonObject dic)
{
    for (JsonPair kv : dic)
    {
        String s = kv.key().c_str();
        if (s == "font_type")
            font_type = kv.value().as<const char *>();
        else if (s == "size")
            size = kv.value().as<uint16_t>();
        else if (s == "color")
            color = color_hex(kv.value().as<const char *>());
        else if (s == "bg_color")
            bg_color = color_hex(kv.value().as<const char *>());
        else if (s == "opacity")
            opacity = kv.value().as<bool>();
    }
}

//画一个像素
void draw_pixel(uint32_t x, uint32_t y, uint16_t color)
{
    if (x >= tft.width() || y >= tft.height())
        return;
    img[y * tft.width() + x] = color;
}

//写一个字
void draw_char(String s, int32_t x, int32_t y, Font ft)
{
    uint32_t w = ft.size, index; //字宽，索引序号

    //生成字体文件路径 如/font/FZQK_64_GB2312.FON
    String font_path = "/font/";
    font_path += ft.font_type + "_" + String(ft.size) + "_";
    if (s.length() == 1)
    {
        font_path += "ASCII.FON", w /= 2;
        index = s[0];
    }
    else
    {
        font_path += "GB2312.FON";
        if (s.length() == 2)
        {
            //将二字节UTF-8转为unicode编码数字
            index = s[0] & 0x1F;
            index <<= 6;
            index |= s[1] & 0x3F;
        }
        else
        {
            //将汉字转为unicode编码数字
            index = s[0] & 0xF;
            index <<= 6;
            index |= s[1] & 0x3F;
            index <<= 6;
            index |= s[2] & 0x3F;
        }
        index <<= 1; //乘2得到unicode索引
        File f = mFS.open("/font/unicode.ind");
        f.seek(index);
        uint8_t mindex[2];
        f.read(mindex, 2);
        f.close();
        index = mindex[0];
        index = (index << 8) | mindex[1];
    }

    uint32_t wbsize = w / 8 + ((w % 8 == 0) ? 0 : 1); //计算每行所占字节数
    //获取字点阵
    File f = mFS.open(font_path);
    f.seek(wbsize * ft.size * index);
    f.read(img_tmp, wbsize * ft.size);
    f.close();

    //显示到屏幕上
    for (uint32_t yy = 0; yy < ft.size; yy++)
        for (uint32_t xx = 0; xx < w; xx++)
        {
            if (img_tmp[yy * wbsize + xx / 8] & (128 >> (xx % 8)))
                draw_pixel(x + xx, y + yy, ft.color);
            else if (ft.opacity)
                draw_pixel(x + xx, y + yy, ft.bg_color);
        }
}

//在中心写一个字
void draw_char_center(String s, Font ft)
{
    int32_t w, h = ft.size;
    if (s.length() == 1)
        w = ft.size / 2;
    else
        w = ft.size;
    draw_char(s, (tft.width() - w) / 2, (tft.height() - h) / 2, ft);
}

//在横向中心写一个字
void draw_char_center_x(String s, int32_t y, Font ft)
{
    int32_t w;
    if (s.length() == 1)
        w = ft.size / 2;
    else
        w = ft.size;
    draw_char(s, (tft.width() - w) / 2, y, ft);
}

//写一些字
void draw_string(String s, int32_t x, int32_t y, Font ft, bool auto_newline)
{
    uint32_t len = s.length(), x_ = x, mx;
    for (uint32_t i = 0; i < len;)
    {
        if ((s[i] & 0x80) == 0) //ASCII
        {
            if (s[i] == '\n')
                y += ft.size, x = x_;
            else
            {
                mx = x + ft.size / 2 - 1;
                if (mx >= tft.width())
                    y += ft.size, x = x_;
                draw_char(s.substring(i, i + 1), x, y, ft);
                x += ft.size / 2;
            }
            i++;
        }
        else if ((s[i] & 0xE0) != 0xE0) //二字节UTF-8
        {
            mx = x + ft.size - 1;
            if (mx >= tft.width())
                y += ft.size, x = x_;
            draw_char(s.substring(i, i + 2), x, y, ft);
            i += 2;
            x += ft.size;
        }
        else //三字节UTF-8
        {
            mx = x + ft.size - 1;
            if (mx >= tft.width())
                y += ft.size, x = x_;
            draw_char(s.substring(i, i + 3), x, y, ft);
            i += 3;
            x += ft.size;
        }
    }
}

//画图 文件格式为二进制16bit(565) default: w=tft.width() h=tft.height() x=0 y=0
void draw_img(String path)
{
    uint16_t w = tft.width();
    uint16_t h = tft.height();
    File f = mFS.open("/screen/" + path);
    f.read(img_tmp, w * h * 2);
    f.close();
    memcpy(img, img_tmp, w * h * 2);
}

//全屏填充颜色
void draw_color(uint16_t color)
{
    if (color == TFT_WHITE)
        memset(img, -1, sizeof(img));
    else if (color == TFT_BLACK)
        memset(img, 0, sizeof(img));
    else
        for (int32_t i = (sizeof(img) / 2 - 1); i >= 0; --i)
            img[i] = color;
}

//根据JSON写字
void draw_string_by_json(JsonObject dic)
{
    int32_t x = 0, y = 0;
    bool auto_newline = true;
    if (!dic.containsKey("s"))
        return;
    if (dic.containsKey("x"))
        x = dic["x"];
    if (dic.containsKey("y"))
        y = dic["y"];
    if (dic.containsKey("auto_newline"))
        auto_newline = dic["auto_newline"];
    draw_string(dic["s"], x, y, ft, auto_newline);
}

//根据JSON画图
void draw_img_by_json(JsonObject dic)
{
    if (!dic.containsKey("path"))
        return;
    draw_img(dic["path"]);
}

//根据JSON绘制
void draw_by_json(JsonArray dic)
{
    for (JsonVariant i : dic)
    {
        if (i["type"] == "font")
            ft.set_by_json(i["data"]);
        else if (i["type"] == "string")
            draw_string_by_json(i["data"]);
        else if (i["type"] == "img")
            draw_img_by_json(i["data"]);
        else if (i["type"] == "color")
            draw_color(color_hex(i["data"]["color"]));
    }
}

//测试
void test(void)
{
    double m;
    Font ft;
    ft.size = 128;
    for (int32_t d = 0, top = 0; d < 30000; d++)
    {
        if (d % 10 == 0)
            top = 0;
        for (int32_t i = 0; i < 135; i++)
        {

            m = sin((d + i) / 142.0) * cos((d + i) / 124.0) * 42 + 180;
            tft.drawFastVLine(i, top, m, color_hex("#ff8c94"));
            tft.drawFastVLine(i, m, 240 - m, color_hex("#00e0ff"));
        }
        if (d % 10 == 0)
        {
            draw_char_center_x(String((d / 10) % 10), 0, ft);
            top = 128;
        }
    }
}

//输入形如#aabbcc的颜色值 输出16位颜色
uint16_t color_hex(String s)
{
    int32_t r, g, b;
    char **p = 0;
    r = strtol(s.substring(1, 3).c_str(), p, HEX);
    g = strtol(s.substring(3, 5).c_str(), p, HEX);
    b = strtol(s.substring(5, 7).c_str(), p, HEX);
    return tft.color565(r, g, b);
}

//屏幕初始化
void screen_init()
{
    img_tmp = new uint8_t[135 * 240 * 2];
    for (uint32_t i = 0; i < 5; i++)
    {
        scr[i].init();
        tft.fillScreen(random(0, 65535));
    }
    tft.setSwapBytes(true);
    screen_file_update();
}

//屏幕刷新
void screen_update()
{
    for (uint32_t i = 0; i < 5; i++)
    {
        if (scr[i].need_draw) //是否需要刷新
        {
            scr[i].enable();
            draw_by_json(scr[i].draw_json.as<JsonArray>());
            scr[i].push();
            scr[i].need_draw = false;
        }
    }
}

//载入屏幕显示内容描述文件到内存
void screen_file_update()
{
    for (uint32_t i = 0; i < 5; i++)
    {
        File f = mFS.open("/screen/" + String(i) + ".json");
        scr[i].template_string = f.readString();
        f.close();
    }
}

//替换关键字
void string_replace(String &s)
{
    s.replace("~P~", env.P);
    s.replace("~L~", env.L);
    s.replace("~T~", env.T);
    s.replace("~H~", env.H);
    s.replace("~A~", env.A);
    s.replace("~time~", String(now_time.timestamp(now_time.TIMESTAMP_TIME)));
    s.replace("~date~", String(now_time.timestamp(now_time.TIMESTAMP_DATE)));
    s.replace("~year~", String(now_time.year()));
    s.replace("~mon~", String(now_time.month()));
    s.replace("~day~", String(now_time.day()));
    s.replace("~weekday~", daysOfTheWeek[now_time.dayOfTheWeek()]);
    s.replace("~hour~", String(now_time.hour()));
    s.replace("~hour0~", String(now_time.hour() / 10));
    s.replace("~hour1~", String(now_time.hour() % 10));
    s.replace("~min~", String(now_time.minute()));
    s.replace("~min0~", String(now_time.minute() / 10));
    s.replace("~min1~", String(now_time.minute() % 10));
    s.replace("~sec~", String(now_time.second()));
    s.replace("~sec0~", String(now_time.second() / 10));
    s.replace("~sec1~", String(now_time.second() % 10));
}

//运行在core0上 解析屏幕描述文件为JSON
void screen_data_update()
{
    String s;
    for (uint32_t i = 0; i < 5; i++)
    {
        s = scr[i].template_string;
        string_replace(s);
        if (s == scr[i].draw_string)
            continue;
        scr[i].draw_string = s;
        deserializeJson(scr[i].draw_json, s);
        scr[i].need_draw = true;
    }
}