# smartHome_TemperatureModule
Temperature module for my smart home project. It can be powered by one/two 18650 BatteryCell or USB-C. If you use the battery, there is also a small PV charging circuit that makes battery replacement unnecessary.
Data communication works over the 433 Mhz band with the RFM69 module soldered to the board.

Measurements can be taken every 8 seconds (or multiples thereof). There are also two sensor options on board. One option is the SHT41 I2C digital temperature and humidity sensor from Sensirion. The other option is a standard 100R NTC resistor (in my case 44001RC) powered by a constant current source. The measurement is done with a differential ADC.

keep in mind: The firmware is still under development. Some changes need to be made to the data header for transmission.
