
# LVGL Coremark Demo
## Overview

Multi-core LVGL coremark example project runs the "LittleVGL coremark" example on core0 and core1.

In this project:
 - There are two buttons shown on the GUI, users can run the dual-core coremark by clicking "Start" button and switch among different frequencies by clicking the button with the current core frequency shown on it.


## Board Setting

  BOOT_PIN should be configured to 0-OFF, 1-OFF


## Generate and Build Multi-core projects

In this project, the core0 application runs in FLASH while the core1 application runs in its own ILM

__Core1__ project must be generated and built first

__Core0__ project must be generated after the core1 project has been built successfully.

### Generate core1 project
__CMAKE_BUILD_TYPE__ must be one of the following options:
- *"sec_core_img"*
- *"sec_core_img_release"*

The *"-t sec_core_img"* should be used when generating the project in the SDK env package

### Generate core0 project
__CMAKE_BUILD_TYPE__ must be one of the following options:
- *"flash_sdram_xip"
- *"flash_sdram_xip_release"

## Running the example

- Download the core0 example to the target, dis-connect the debugger probe and reset the board

When the project runs successfully, there are two buttions:
- "Start"
- "Frequency Switch"

Users can clock the "Start" button to start the dual-core coremark benchmarking
User can click the Frequence Switch button to switch different frequencies
