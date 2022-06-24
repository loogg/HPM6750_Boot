# Audio Wav Decoder
## 概述

音频编解码示例工程展示了如何播放WAV格式音频。
在这个工程中，读取SD卡中的wav格式文件然后进行播放
MCU会通过I2S接口将解码的数据发送给音频编解码器，音频编解码器会解码收到的数据并播放。

## 硬件设置

1. 连接耳机到开发板的HP_MIC接口


## 运行现象

当工程正确运行后，选择音乐名称，然后耳机会播放这首音乐，而串口终端会输出如下信息：
```

I2C bus is ready
audio codec example
SD card has been mounted successfully
Test WAV decode
Please input wav name:
wav_name:music1.wav.
music playing time:202 seconds ...
music end.

```
