# i2c_slave
## Overview

The i2c_slave sample project shows that I2C sends/receives data using interrupts.
In this example project, the I2C master sends data in an interrupt mode. After receiving the data from the master, the I2C slave sends the data back to the I2C master. Then compare the data sent by the master with the data returned by the slave. If the two sets of data are identical, it indicates that the I2C interrupt sends and receives data correctly.

## Board Setting

- HPM6750EVKMINI:Pin 13 is connected to pin 5 in P1 connector, pin 15 is connected to pin 3 in P1 connector.

## Running the example

- When the I2C data is correctly transmitted using interrupts, the serial port terminal outputs the following information：

	Master got: xx

- When the I2C fails to transmit data using interrupts, the serial port terminal outputs the following information：

	data read is not expected
