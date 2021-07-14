/*
 * @Author: flwfdd
 * @Date: 2021-07-13 15:18:24
 * @LastEditTime: 2021-07-13 23:54:32
 * @Description: 网络相关模块
 * _(:з」∠)_
 */

#include "memori_net.h"

WebServer server(80);
DNSServer dnsServer;
IPAddress local_IP(192, 168, 4, 1);
HTTPClient http;

//根据文件名返回content_type
String get_type(const String &filename)
{
    if (filename.endsWith(".htm"))
        return "text/html";
    if (filename.endsWith(".html"))
        return "text/html";
    if (filename.endsWith(".css"))
        return "text/css";
    if (filename.endsWith(".js"))
        return "application/javascript";
    if (filename.endsWith(".png"))
        return "image/png";
    if (filename.endsWith(".gif"))
        return "image/gif";
    if (filename.endsWith(".jpg"))
        return "image/jpeg";
    if (filename.endsWith(".ico"))
        return "image/x-icon";
    if (filename.endsWith(".json"))
        return "application/json";
    return "text/plain";
}

//同步时间
void update_ntp_time()
{
    struct tm ti;
    configTime(3600 * 8, 0, "ntp.aliyun.com", "cn.ntp.org.cn", "time.windows.com");
    if (!getLocalTime(&ti))
        Serial.println("UPDATE time error!!!");
    else
    {
        Serial.println("UPDATE time OK!!!");
        set_time(ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday, ti.tm_hour, ti.tm_min, ti.tm_sec);
    }
}

//以下为网络回调----------------------------

//扫描wifi
void scan_wifi()
{
    DynamicJsonDocument doc(1024);
    //WiFi.disconnect();
    int n = WiFi.scanNetworks();
    Serial.print(n);
    Serial.println(" network(s) found");
    for (int i = 0; i < n; i++)
    {
        //Serial.println(WiFi.SSID(i));
        doc.add(WiFi.SSID(i));
    }
    String s;
    serializeJson(doc, s);
    server.send(200, "application/json", s);
}

//设置wifi
void set_wifi()
{
    String ssid, pwd;
    for (uint8_t i = 0; i < server.args(); i++)
    {
        if (server.argName(i) == "ssid")
            ssid = server.arg(i);
        if (server.argName(i) == "pwd")
            pwd = server.arg(i);
    }
    if (ssid != "" && pwd != "")
    {
        WiFi.begin(ssid.c_str(), pwd.c_str());
        DynamicJsonDocument doc(1024);
        doc["ssid"] = ssid;
        doc["pwd"] = pwd;
        String s;
        serializeJson(doc, s);
        File f = mFS.open("/wifi.json", "w");
        f.print(s);
        f.close();

        server.send(200, "text/plain", "ok");
    }
    else
    {
        server.send(200, "text/plain", "err");
    }
    Serial.println(ssid);
    Serial.println(pwd);
}

//获取状态信息
void get_status()
{
    DynamicJsonDocument doc(1024);
    doc["wifi"] = WiFi.isConnected();

    String s;
    serializeJson(doc, s);
    server.send(200, "application/json", s);
}

//处理未定义网络请求
void handleNotFound()
{
    Serial.println(server.uri());
    if (mFS.exists(server.uri()))
    {
        File f = mFS.open(server.uri(), "r");
        server.streamFile(f, get_type(server.uri()));
    }
    else
    {
        server.send(404, "text/html", "404 Not Found");
    }
}

uint8_t buf[1024];
//从网络下载文件到tf卡
void download(String url, String path)
{
    http.begin(url);
    int32_t status = http.GET();
    if (status == 200)
    {
        File f = mFS.open(path, "wb");
        WiFiClient stream = http.getStream();
        int32_t len = http.getSize(), size;
        Serial.printf("%s size:%d\n", url.c_str(), len);
        while (http.connected() && (len > 0 || len == -1))
        {
            size = stream.available();
            if (size)
            {
                size = (size > sizeof(buf)) ? sizeof(buf) : size;
                stream.readBytes(buf, size);
                f.write(buf, size);
                if (len > 0)
                    len -= size;
            }
            delay(1);
        }
        f.close();
    }
    else
        Serial.println("HTTP error!!!");
}

//执行GET
String get_string(String url)
{
    http.begin(url);
    int32_t status = http.GET();
    if (status == 200)
    {
        return http.getString();
    }
    else
    {
        Serial.println("HTTP error!!!");
        return "";
    }
}

//将环境数据生成为GET请求格式
String get_env_param()
{
    String s = "";
    s += "P=" + env.P;
    s += "&L=" + env.L;
    s += "&T=" + env.T;
    s += "&A=" + env.A;
    s += "&H=" + env.H;
    return s;
}

//WiFi初始化
void net_init()
{
    //配置WiFi模式
    WiFi.mode(WIFI_AP_STA);

    //开启AP
    WiFi.softAP("Memori");
    Serial.println(WiFi.softAPIP());
    Serial.println(WiFi.softAPmacAddress());

    //配置DNS
    dnsServer.start(53, "memori.com", local_IP);

    //连接WiFi
    if (mFS.exists("/wifi.json"))
    {
        File f = mFS.open("/wifi.json", "r");
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, f.readString());
        f.close();
        String ssid = doc["ssid"], pwd = doc["pwd"];
        Serial.println(ssid);
        Serial.println(pwd);
        WiFi.begin(ssid.c_str(), pwd.c_str());
    }

    //配置webserver
    server.on("/", []()
              { server.send(200, "text/html", "Hello Memori!"); });
    server.on("/scan_wifi", scan_wifi);
    server.on("/set_wifi", set_wifi);
    server.on("/get_status", get_status);
    server.onNotFound(handleNotFound);
    server.begin();
}

String data_url = "http://flwfdd.xyz/memori/screen/";
//当前时间 单位秒
uint32_t now = 0;
//下一次更新的时间
uint32_t day_1 = 0, min_10 = 0;
//刷新
void net_update()
{
    server.handleClient();
    dnsServer.processNextRequest();
    if (WiFi.isConnected())
    {
        digitalWrite(LED_BUILTIN, LOW);
        now = millis() / 1000;
        if (day_1 < now) //每日任务
        {
            day_1 = now + 24 * 3600;
            //联网更新时间
            update_ntp_time();
            Serial.println(get_env_param());
        }
        if (min_10 < now)
        {
            min_10 = now + 600;
            //更新屏幕内容描述文件
            String name;
            for (uint32_t i = 0; i < 5; i++)
            {
                name = String(i) + ".json";
                download(data_url + name, "/screen/" + name);
                screen_file_update();
            }
            //更新依赖文件
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, get_string(data_url + "list.json"));
            JsonArray l = doc.as<JsonArray>();
            for (JsonVariant i : l)
            {
                name = i.as<String>();
                if (!mFS.exists("/screen/" + name))
                    download(data_url + name, "/screen/" + name);
            }
        }
    }
    else
        digitalWrite(LED_BUILTIN, HIGH);
}