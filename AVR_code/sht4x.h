#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <math.h>
#include "twi.h"

#define SHT41_address   0x88 // 0x44
#define SHT41_read      0x01
#define SHT41_write     0x00



// crc8 checksum
uint8_t sht4x_crc8(const uint8_t *data, uint8_t len) {
    
  // adapted from SHT21 sample code from
  // http://www.sensirion.com/en/products/humidity-temperature/download-center/
  uint8_t crc = 0xff;
  uint8_t byteCtr;
  for (byteCtr = 0; byteCtr < len; ++byteCtr) {
    crc ^= data[byteCtr];
    for (uint8_t bit = 8; bit > 0; --bit) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31;
      } else {
        crc = (crc << 1);
      }
    }
  }
  return crc;
}


// init
void sht4x_init_twi(void) {
    TWIInit();
    return;
}


// send command
uint8_t sht4x_send_cmd(uint8_t cmd) {
    TWIStart();
    TWIWrite(SHT41_address | SHT41_write);
    TWIWrite(cmd);
    TWIStop();
    return 0;
}


// read data
// data size must be 6 bytes large!
uint8_t sht4x_read_data(uint8_t* data) {
    
    // read data
    TWIStart();
        TWIWrite(SHT41_address | SHT41_read);
        for (int i=0; i<6; i++) {
          if (i<5) {
              data[i] = TWIReadACK();
          } else {
              data[i] = TWIReadNACK();
          }
            
        }
    TWIStop();
    
    // crc8 checksum check
    if (sht4x_crc8(&data[0], 2) != data[2] || sht4x_crc8(&data[3], 2) != data[5]) {
        return 1;   // fail
    } else {
        return 0;   // OK
    }
}


// get temperature
float sht4x_get_temperature_from_data(uint8_t* data) {
//     int32_t val;
//     val = (data[0] << 8) + data[1];
//     return (uint16_t)(-45 + (175 * val) / 65535);
    
    float t_ticks = (uint16_t)data[0] * 256 + (uint16_t)data[1];
    float temperature = -45 + 175 * t_ticks / 65535;
    return temperature;
}


// get humidity
float sht4x_get_humidity_from_data(uint8_t* data) {
//     int32_t val;
//     val = (data[3] << 8) + data[4];
//     val = -6 + (125 * val) / 65535;
//     if (val > 100) { return 100; }
//     if (val <   0) { return 0; }
//     return (uint16_t)val;
    
    float rh_ticks = (uint16_t)data[3] * 256 + (uint16_t)data[4];
    float humidity = -6 + 125 * rh_ticks / 65535;
    if (humidity < (float)0.0)   { return 0.0; }
    if (humidity > (float)100.0) { return 100.0; }
    return humidity;
}



