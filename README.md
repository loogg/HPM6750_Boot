# HPM6750 Boot

## 说明

- 本仓库为基于 `RT-Thread v4.0.5` 版本实现的 HPM6750 Bootloader，可直接在 `HPM6750EVKMINI` 上使用。

- 识别 `download` 分区中的固件并搬运到 `app` 分区中运行。

- 支持通过 `RS485` 强制进入 Bootloader 进行升级，可下载固件到 `download` 分区和 `app` 分区。

- 支持读取 `SD` 卡中的固件进行升级。

- 支持连接 `WIFI` 热点通过 `Web` 升级。

- 使用 RT-Thread 固件打包工具将 bin 文件打包成 rbl 文件。该 Bootloader 不支持压缩和加密形式的固件。

- RT-Thread 固件打包工具在 tools/packing 目录下。

- RS485 升级工具在 tools/rs485_update 目录下。

- 使用 `RT-Thread Studio` 导入工程

## 资源占用

- XPI0

- SDRAM 16M

- RW007
  - SPI1:
    - CS: PE03
    - SCLK: PD31
    - MOSI: PE04
    - MISO: PD30
    - INT: PE01
    - RST: PE02

- SDXC0

- RS485: MAX13487 自动收发使能
  - UART13:
    - TX: PZ09
    - RX: PZ08

- 电池备份域
  - HPM_BGPR->BATT_GPR7

- 分区使用：

  | 分区 | 偏移地址 | 占用空间 |
  | ---- | ---- | ---- |
  | Bootloader | 0 | 1 * 1024 * 1024 |
  | app | 1 * 1024 * 1024 | 1 * 1024 * 1024 |
  | download | 2 * 1024 * 1024 | 1 * 1024 * 1024 |

## APP 链接文件修改

删除链接文件中 `BootROM` 的 `Section`, 只保留 `APP Section`

![pic2](./figures/2.png)

修改完后的如下：

![pic3](./figures/3.png)

## 固件升级

### Bootloader 启动过程

![pic1](./figures/1.png)

**注意** ：SD 卡优先级最高，即 Bootloader 处理流程中 SD 卡检查通过就直接进行 SD 卡升级固件。

![pic4](./figures/4.png)

### APP 中下载到 download 分区

![app_update](./figures/app_update.gif)

### SD 卡固件升级

SD 卡根目录下放入 `rtthread.rbl` 文件。

![sd_update](./figures/sd_update.gif)

### Web 升级

- 按住按键然后再 `上电` 或 `重启`，强制进入 Bootloader

- 也可通过 RS485 方式强制进入 Bootloader

#### 上传打包固件(rbl) 到 `download` 分区

  ![web_download](./figures/web_download.gif)

#### 上传原始固件(bin) 到 `app` 分区

  ![web_app](./figures/web_app.gif)

### RS485 升级

- 配置好串口并打开串口

- 点击 `开始同步` 按钮后再 `上电` 或 `重启`，强制进入 Bootloader

- 点击 `结束同步` 按钮

- 选择下载到 `download` 分区还是 `app` 分区

- 选择文件开始升级

- 地址一栏填 `0` 即为广播

#### 下载打包固件(rbl) 到 `download` 分区

  ![rs485_download](./figures/rs485_download.gif)

#### 下载原始固件(bin) 到 `app` 分区

  ![rs485_app](./figures/rs485_app.gif)

#### RS485 固件升级协议

![ModbusProtocol](./figures/ModbusProtocol.jpg)

使用 `0x50` 作为固件升级的特殊功能码。

分包传输固件数据，每包数据最大 4096 字节。

- `Data` 字段协议定义：

  | 命令 | 字节数 | 数据 |
  | ---- | ---- | ---- |
  | 2 Bytes | 2 Bytes | N Bytes |

- 命令:

  | 命令 | 说明 |
  | ---- | ---- |
  | 0x0001 | 同步 |
  | 0x0002 | 停留 Bootloader 确认 |
  | 0x0003 | 启动升级 |
  | 0x0004 | 写 IAP 数据 |
  | 0x0005 | 执行升级运行 |

- 0x0001 同步

  上位机控制器上电前开始持续发送同步命令，控制器上电 2s 后停止，控制器停留 Bootloader 中等待上位机进一步操作。

  发送：

  | 命令 | 字节数 | 数据 |
  | ---- | ---- | ---- |
  | 00 01 | 00 00 | / |

  响应：

  无

- 0x0002 停留 Bootloader 确认

  发送：

  | 命令 | 字节数 | 数据 |
  | ---- | ---- | ---- |
  | 00 02 | 00 00 | / |

  响应：

  | 命令 | 字节数 | 数据 |
  | ---- | ---- | ---- |
  | 00 02 | 00 00 | / |

- 0x0003 启动升级

  该操作处理时间较长，上位机等待响应的时间需要放大。

  发送：

  | 命令 | 字节数 | 数据 |
  | ---- | ---- | ---- |
  | 00 03 | 00 05 | flash类型(1B)</br>固件总大小(4B) |

  flash类型：

  | 值 | 说明 |
  | ---- | ---- |
  | 1 | app 分区 |
  | 2 | download 分区 |

  响应：

  | 命令 | 字节数 | 数据 |
  | ---- | ---- | ---- |
  | 00 03 | 00 01 | 0：失败</br>1：成功 |

- 0x0004 写 IAP 数据

  发送：

  | 命令 | 字节数 | 数据 |
  | ---- | ---- | ---- |
  | 00 04 | 4+N | 包序号(2B)</br>数据长度(2B)</br>数据(NB) |

  响应：

  | 命令 | 字节数 | 数据 |
  | ---- | ---- | ---- |
  | 00 04 | 00 03 | 包序号(2B)</br>状态(1B)：失败(0)/成功(1) |

- 0x0005 执行升级运行

  发送：

  | 命令 | 字节数 | 数据 |
  | ---- | ---- | ---- |
  | 00 05 | 00 00 | / |

  响应：

  | 命令 | 字节数 | 数据 |
  | ---- | ---- | ---- |
  | 00 05 | 00 00 | / |

## 联系人信息

- 维护：马龙伟
- 邮箱：<2544047213@qq.com>
