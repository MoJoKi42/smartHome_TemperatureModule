#include <avr/io.h>
#include <util/twi.h>


void TWIInit(void);
void TWIStart(void);
void TWIStop(void);
uint8_t TWIWrite(uint8_t u8data);
uint8_t TWIReadACK(void);
uint8_t TWIReadNACK(void);
uint8_t TWIGetStatus(void);
