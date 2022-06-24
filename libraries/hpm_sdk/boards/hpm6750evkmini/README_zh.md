# HPM6750EVKMINI开发板

## 概述
HPM6750是一款主频达816Mhz的双核微控制器。该芯片拥有最大2M字节的连续片上RAM，并集成了丰富的存储接口，如SDRAM，Quad SPI NOR flash， SD/eMMC卡。同时它也提供多种音视频接口包括LCD显示，像素DMA，摄像头以及I2S音频接口。

 ![hpm6750evkmini](../../doc/images/boards/hpm6750evkmini/hpm6750evkmini.png "hpm6750evkmini")
## 板上硬件资源
- HPM6750IVM 微控制器 (主频816Mhz, 2MB片上内存)
- 板载存储
  - 128Mb SDRAM
  - 64Mb Quad SPI NOR Flash
- 显示/摄像头
  - LCD接口
  - 摄像头(DVP)接口
- WiFi
  - RW007
- USB
  - USB type C (USB 2.0 OTG) connector x2
- 音频
  - Mic
  - DAO
- 其他
  - TF卡槽
  - FT2232
  - 蜂鸣器
  - RGB LED
- 扩展口
  - ART-PI
## 拨码开关 S1
- Bit 1，2控制启动模式

| Bit[2:1] | 功能描述|
|----------|------------|
|OFF, OFF| Quad SPI NOR flash 启动 |
|OFF, ON| 串行启动 |
|ON, OFF| 在系统编程 |

## 按键
| 名称 | 功能 |
|PBUTN (S2) | 电源按键, TinyUF2 Boot按键, GPIO 按键|
|WBUTN (S3) | WAKE UP 按键|
|RESET (S4) | Reset 按键|


## 引脚描述

(lab_hpm6750_evk_board)=

