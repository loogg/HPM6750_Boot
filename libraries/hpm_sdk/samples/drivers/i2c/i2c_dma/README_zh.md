# i2c_dma
## 概述

i2c_dma示例工程展示了I2C采用DMA的方式发送数据。
在这个示例工程中，I2C master 采用DMA的方式发送数据，I2C slave会接收到master的数据，然后比对master发出的数据与slave接收到
的数据，如果两组数据完全一致，则代表I2C DMA传输数据正确，否则代表I2C DMA传输数据错误。

## 硬件设置
- HPM6750EVKMINI：P1连接器的13引脚与5引脚相连，P1连接器的15引脚与3引脚相连。

## 运行现象

- 当I2C DMA传输数据正确时，串口终端会输出如下信息:Master send data OK!
- 当I2C DMA传输数据错误时，串口终端会输出如下信息:Master send data ERROR!




