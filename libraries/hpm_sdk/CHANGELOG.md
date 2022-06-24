# Change Log

## [0.10.0] - 2022-04-30:
Main changes since 0.9.0

### Changed:
  - drivers: gptmr: split irq enable/disable interface
  - drivers: can: 155 Update SJW setting in CAN driver
  - soc: HPM6750: rename safe stack symbol in link script
  - components: debug_console: wait uart tx flag on writing.

### Added:
  - drivers: enet: add 1588 support
  - drivers: can: Add TDC support
  - drivers: mchtmr: add counter init API.
  - drivers: dma: add dma_disable_channel() API
  - middleware: add wav decoder
  - samples: lwip: add ptp demos
  - samples: tinyusb: add msc, cdc demo
  - samples: audio_codec: add wav player demo
  - samples: add tinyuf2 initial support
  - samples: add initial uart dma rx idle demo

### Fixed:
  - soc: HPM6750: sysctl: fix cpu lp mode API.
  - drivers: uart: correct baudrate calculation.
  - drivers: usb: fix: no response in device mode when a USB cable is pluged out
  - boards: correct the pin setting related to USB

## [0.9.0] - 2022-04-01:
Main changes since 0.8.0

### Changed:
  - drivers: update WDG, UART, DMA, DMAMUX driver
  - drivers: enet remove enet_intf_selection
  - drivers: can: support configure bit timing via low-level bit timing parameters
  - drivers: optimize gpio driver
  - samples: exclude flash targets for mbx core1
  - samples: adjust SES project setting for coremark
  - samples: jpeg: update and integration jpeg decode samples
  - ses: use relpath for linker script in project file
  - ses: add HPM device name in generated project file
  - soc: HPM6750: add an interface selection api

### Added:
  - soc: HPM6750: add ram linker for core1
  - ses: support to use Andes toolchain
  - middleware: add hpm_math (replacing hpm_dsp)
  - samples: add lwip httpd
  - drivers: add section and alignment general instructions in hpm_common.h

### Fixed:
  - boards: hpm6750evk: fix bldc adc pinmux error
  - boards : hpm6750evk : pinmux : fix spi init pins error
  - samples: sdp: fix non-cacheable data initialization issue
  - samples: littlevgl: fix wrong picture patch in README

## [0.8.0] - 2022-03-04:
All changes since 0.7.3

### Changed:
  - rename middleware/sdmmc to middleware/hpm_sdmmc
  - place isr into .isr_vector section for irq non-vector mode
  - change csr functions to support llvm

### Fixed:
  - ses: fix issue in register xml
  - freertor: fix trap handler at non-vector mode
  - sdxc: fix software reset issue

### Added:
  - add sdk doc
  - add more sample doc
  - add multicore demo
  - i2c/uart: add dma support
  - add tensorflow lite for microcontroller

## [0.7.3] - 2022-02-23:
All changes since 0.7.2

### Changed:
  - freertos: change exception handling

## [0.7.2] - 2022-02-21:
All changes since 0.7.1

### Changed:
  - freertos: change freertos irq stack definition, passed in CMakeLists.txt, defined in linker
  - soc: hpm6750: add DISABLE_IRQ_PREEMPTIVE to check if it needs to enable irq preemption

### Fixed:
  - freertos: disable irq preemption

## [0.7.1] - 2022-02-13:

All changes since 0.7.0

### Changed:
  - drivers: adc12: update adc12_prd_config_t
  - samples: can: update case with interrupt and communication between two boards

### Fixed:
  - drivers: can: fix blocking transcation issue
  - samples: mbx: support run this example in SES
  - SES: startup: add fpu enable if abi is set to enable hw fp

### Added:
  - samples: multicore: add flash based multicore example
  - drivers: can: add apis to recvieve message for non-blocking use

## [0.7.0] - 2022-01-30:

All changes since 0.6.2

### Changed:
  - update default CPU frequency to 816MHz from 648MHz
  - update the ip headers
  - drivers: gpio: replace gpio_XXX_pins with gpio_XXX_port
  - drivers: gpio: remove pin level enum definition
  - drivers: i2s: driver update to remove mclk_div
  - drivers: ptpc: update driver
  - drivers: common: update get first set bit API
  - drivers: uart: split one enable with parameter into enable and disable interfaces
  - drivers: pwm: change name of output_channel config API
  - drivers: trgm: split separate API to enable/disable io output
  - soc: HPM6750: initialize noncacheable data in startup
  - soc: HPM6750: l1c: update fence.i call
  - samples: hello_world: add LED flashing
  - samples: littlevgl: remove lvgl example

### Fixed:
  - hpm6750evkmini: correct refresh cycle number of sdram
  - hpm6750evkmini: Fix the SDRAM memory range issue in flash_sdram_xip linker file
  - SES: remove no_relax option for linker, since segger has provide a patch to its ld for ses v6.10.
  - drivers: gpio: fix read pin issue:
  - drivers: usb: fix usb disconnection under linux environment
  - drivers: sdxc: Fixed the compatibility issue on different SD/eMMC cards in the sdcard_fatfs demo
  - drivers: gptmr: incorrect DMAEN configuration condition
  - drivers: gptmr: clear CNTRST bit after set.
  - SDK_DECLARE_EXT_ISR_M cannot work in the c++ file
  - FreeRTOS: fix ISR_STACK setting
  - components: touch: gt911: fix gpio write pin call

### Added:
  - SEG: add register definition file in generated embedded studio project
  - samples: drivers: gpiom: Add example to demonstrate gpiom's function
  - drivers: common: add macro to put data into noncacheable sections
  - middleware: integrate lwip

