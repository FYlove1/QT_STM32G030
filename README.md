# 基于STM32的智能家居系统

[![GitHub license](https://img.shields.io/github/license/yourusername/yourrepo)](https://github.com/yourusername/yourrepo/blob/main/LICENSE)
[![GitHub issues](https://img.shields.io/github/issues/yourusername/yourrepo)](https://github.com/yourusername/yourrepo/issues)
[![GitHub stars](https://img.shields.io/github/stars/yourusername/yourrepo?style=social)](https://github.com/yourusername/yourrepo)

## 项目概述

本项目旨在开发一套基于STM32单片机的智能家居系统，包括温控系统、智能农业系统和报警系统。通过采集环境数据并控制相关设备，实现家居环境的智能化管理。

## 场景描述

在现代家居环境中，用户需要一个能够自动调节室内环境、控制家电设备，并能远程监控和操作的系统。本项目通过STM32单片机与QT上位机软件的结合，实现了一个能够自动采集环境温湿度、光线强度，并控制灯光、风扇等设备的智能家居系统。

## 功能模块

### 1. 环境数据采集

- **温湿度采集**：通过DHT11传感器采集环境温湿度数据。
- **光线强度采集**：通过光敏电阻（LDR）采集环境光线强度。

### 2. 设备控制

- **灯光控制**：通过继电器模块控制灯光的开关。
- **风扇控制**：通过GPIO控制风扇的启动和停止。
- **继电器控制**：控制其他家用电器设备的开关。

### 3. 控制方式

- **本地控制**：使用Keil 5开发环境，实现STM32单片机的本地控制逻辑。
- **远程控制**：开发QT上位机软件，通过Wi-Fi实现对开发板的远程控制。

### 4. 网络通信

- **UDP通信**：使用ESP8266模块实现上位机端和STM32单片机之间的远程无线通信和控制。

## 技术实现

### STM32单片机

- **硬件选型**：选择合适的STM32单片机型号，并设计相应的电路。
- **固件编写**：编写STM32固件，实现数据采集和设备控制功能。

### QT上位机软件

- **用户界面设计**：开发QT上位机软件，实现用户界面设计。
- **远程控制逻辑**：通过QT的UDP连接功能，实现远程控制逻辑。

## 代码结构

项目源码分为两个主要部分：STM32单片机固件和QT上位机软件。

### STM32 固件 (`STM32/`)

- `main.c`：主程序，包含设备控制和数据采集的主要逻辑。
- `ESP8266.c`：ESP8266模块的驱动代码，负责UDP通信。
- `DHT11.c`：DHT11温湿度传感器的驱动代码。
- `GPIO.c`：GPIO操作代码，包括LED灯和继电器控制。

### QT 上位机软件 (`QT/`)

- `mainwindow.cpp`：主窗口代码，包含用户界面和远程控制逻辑。
- `udpsocket.cpp`：UDP套接字代码，负责数据的发送和接收。

