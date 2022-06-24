# HPM6750EVK

## Overview

The HPM6750 is a dual-core flashless MCU running 816Mhz. It has a 2MB continuous on-chip ram. Also, it provides various memory interfaces, including SDRAM, Quad SPI NOR Flash, SD/eMMC. It integrates rich audio and video interfaces, including LCD, pixel DMA, camera， and I2S audio interfaces.

 ![hpm6750evk](../../doc/images/boards/hpm6750evk/hpm6750evk.png "hpm6750evk")

## Hardware

- HPM6750IVM MCU (816Mhz, 2MB OCRAM)
- Onboard Memory
  - 256Mb SDRAM
  - 128Mb Quad SPI NOR Flash
- Display & Camera
  - LCD connector
  - Camera (DVP)
- Ethernet
  - 1000 Mbits PHY
  - 100 Mbits PHY
- USB
  - USB type C (USB 2.0 OTG) connector x3
- Audio
  - Line in
  - Mic
  - Speaker
  - DAO
- Others
  - TF Slot
  - FT2232
  - Beeper
  - RGB LED
  - CAN
- Expansion port
  - Motor control

## DIP Switch S1

- Bit 1 and 2 controls boot mode

| bit[2:1] | Description                  |
| -------- | ---------------------------- |
| OFF, OFF | Boot from Quad SPI NOR flash |
| OFF, ON  | Serial boot                  |
| ON, OFF  | ISP                          |

- Change the position of bit 3 to select between PWM and 1000Mbit ethernet

| Bit3 | Description    |
| ---- | -------------- |
| OFF  | 1000Mbits ENET |
| ON   | PWM            |

## Button

| Name | FUNCTIONS |
|PBUTN (S2) | Power Button, TinyUF2 Boot Button, GPIO Button |
|WBUTN (S3) | WAKE UP Button |
|RESET (S4) | Reset Button |

## Pin Description

(lab_hpm6750_evk_board)=

1. Description

  ![image-1](../../doc/images/boards/hpm6750evk/hpm6750evk_pwm_output_pin.jpg "image-1")
