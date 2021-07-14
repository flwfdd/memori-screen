# memori_screen

> **mem**ory with **ori**gin

这是一个基于ESP32的项目，灵感来源于辉光管。

硬件上，使用了五块1.14寸的IPS显示屏，并使用tf卡进行存储，还附加了时钟模块、气压、海拔、光强、温度、湿度传感器。

嵌入式固件上，使用`PlatformIO`和`VS Code`编写代码，使用了[Arduino框架](https://github.com/espressif/arduino-esp32)。支持交互式更改WiFi连接，自动同步时间，自动从服务器更新屏幕显示内容并动态绘制。

应该还会做一个网页和相应后端来定制和更新屏幕显示内容，但现在还没有做（摸鱼ing....

总之现在就是开一个仓库临时备份代码(づ￣ 3￣)づ

## 文件夹结构简介

* [memori_esp32](./memori_esp32) ： 嵌入式固件部分，是一个`PlatformIO`项目
* [memori_pcb](./memori_pcb) ： 电路板相关文件

