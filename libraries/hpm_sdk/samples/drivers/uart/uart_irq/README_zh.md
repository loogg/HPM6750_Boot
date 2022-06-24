# 通用异步收发器UART
## 概述
***
**uart**通用异步收发器UART驱动示例工程展示了UART的配置使用方法，实现串口数据的接收和发送
- 示例代码中，进行UART串口资源的初始化配置，设置串口时钟及收发FIFO深度，配置中断，实现串口收据的发送与接收

## 端口设置
***
-  串口波特率设置为``115200bps``，``1个停止位``，``无奇偶校验位``

## 运行现象
***
当工程正确运行后，串口终端会输出如下信息：
```
> uart driver example
> fifo mode
> tx fifo level: 16 bytes
> rx fifo level: 1 bytes
```
通过串口手动输入字符串，如：hello world，则串口终端会收到如下信息：
```
> hello world
```

