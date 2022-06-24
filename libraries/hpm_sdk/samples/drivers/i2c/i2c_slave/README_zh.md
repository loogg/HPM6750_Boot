# i2c_slave
## 概述

i2c_slave示例工程展示了I2C采用中断的方式接收和发送数据。
在这个示例工程中，I2C master 采用中断的方式发送数据，I2C slave接收到master的数据后，会将数据回传给I2C master，然后比对master发出的数据与slave回传的数据，如果两组数据完全一致，则代表I2C 中断接收和发送数据完全正确。

## 硬件设置
- HPM6750EVKMINI:P1连接器的13引脚与5引脚相连，P1连接器的15引脚与3引脚相连。

## 运行现象

- 当I2C 中断传输数据正确时，串口终端会输出如下信息：
	Master got: xx

- 当I2C 中断传输数据错误时，串口终端会输出如下信息：
	data read is not expected




