#include <avr/io.h>
#include <util/twi.h>
#include "twi.h"



void TWIInit(void) {
    //set SCL speed
    TWSR0 = 0x00; // prescaler
    TWBR0 = 0x01;
    
    //enable TWI
    TWCR0 = (1<<TWEN);
}


void TWIStart(void) {
    TWCR0 = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while ((TWCR0 & (1<<TWINT)) == 0);
}


// send stop signal
void TWIStop(void) {
    TWCR0 = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}


// send data
// return 0 -> ACK
// return 1 -> NACK
uint8_t TWIWrite(uint8_t u8data) {
    TWDR0 = u8data;
    TWCR0 = (1<<TWINT)|(1<<TWEN);
    while ((TWCR0 & (1<<TWINT)) == 0);
    
    uint8_t status = TWSR0 & 0xF8;
    if( status != TW_MT_DATA_ACK) return 1;
    return 0;
}


uint8_t TWIReadACK(void) {
    TWCR0 = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while ((TWCR0 & (1<<TWINT)) == 0);
    return TWDR0;
}


// read byte with NACK
uint8_t TWIReadNACK(void) {
    TWCR0 = (1<<TWINT)|(1<<TWEN);
    while ((TWCR0 & (1<<TWINT)) == 0);
    return TWDR0;
}


uint8_t TWIGetStatus(void) {
    uint8_t status;
    //mask status
    status = TWSR0 & 0xF8;
    return status;
}
