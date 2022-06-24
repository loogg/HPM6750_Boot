# SPI_POLLING_SLAVE
## Overview

The spi_polling_slave example shows how to use spi driver as slave to do board to board transfer with polling.
In this example, one spi instance as master and another spi instance on other board as slave. Master sends a piece of data to slave, and receives a piece of data from slave. This example checks if the data received from slave is correct.

## Board Setting

Connect SPI master on board to SPI slave on other board


## Running the example

When the example runs successfully, the log would be seen on the terminal like:
```

SPI-Slave Polling Transfer Example
SPI-Slave transfer format is configured.
SPI-Slave transfer mode:read-dummy-write
------------------------Frame 1------------------------
SPI-Slave transfer waits.
SPI-Slave read command:0x0d
SPI-Slave read address:dummy
SPI-Slave read  data: 0x50 0x50 0xd1 0x51 0xd2 0x52 0xd3 0x53 0xd4 0x54
SPI-Slave write data: 0xb0 0xb1 0xb2 0xb3 0xb4 0xb5 0xb6 0xb7 0xb8 0xb9
SPI-Slave transfer ends.
------------------------Frame 2------------------------
SPI-Slave transfer waits.
SPI-Slave read command:0x1a
SPI-Slave read address:dummy
SPI-Slave read  data: 0xa0 0xa1 0xa2 0xa3 0xa4 0xa5 0xa6 0xa7 0xa8 0xa9
SPI-Slave write data: 0xb0 0xb1 0xb2 0xb3 0xb4 0xb5 0xb6 0xb7 0xb8 0xb9
SPI-Slave transfer ends.
------------------------Frame 3------------------------
SPI-Slave transfer waits.
SPI-Slave read command:0x1a
SPI-Slave read address:dummy
SPI-Slave read  data: 0xa0 0xa1 0xa2 0xa3 0xa4 0xa5 0xa6 0xa7 0xa8 0xa9
SPI-Slave write data: 0xb0 0xb1 0xb2 0xb3 0xb4 0xb5 0xb6 0xb7 0xb8 0xb9
SPI-Slave transfer ends.
------------------------Frame 4------------------------
SPI-Slave transfer waits.

```
