/*
 * @Author: flwfdd
 * @Date: 2021-07-13 15:18:01
 * @LastEditTime: 2021-07-13 19:38:02
 * @Description: 网络相关模块
 * _(:з」∠)_
 */

#ifndef MEMORI_NET_H
#define MEMORI_NET_H

#include "Arduino.h"
#include "WiFi.h"
#include "WebServer.h"
#include "DNSServer.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include "FS.h"
#include "memori_screen.h"
#include "memori_sensor.h"

void net_init();
void net_update();

#endif