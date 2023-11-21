/*
    Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

    $Id$
*/

#include <avr/io.h>
#include "spi.h"

void spi_init(void)
// Initialize pins for spi communication
{
    // set CS, MOSI and SCK to output
    SPI_DDR |= (1 << CS) | (1 << MOSI) | (1 << SCK);

    
    SPCR0 = ((1<<SPE)|              // SPI Enable
            (0<<SPIE)|              // SPI Interupt Enable
            (0<<DORD)|              // Data Order (0:MSB first / 1:LSB first)
            (1<<MSTR)|              // Master/Slave select
            (1<<SPR1)|              // SPI Clock Rate
            (0<<CPOL)|              // Clock Polarity (0:SCK low / 1:SCK hi when idle)
            (0<<CPHA));             // Clock Phase (0:leading / 1:trailing edge sampling)

    SPSR0 = (1<<SPI2X);              // Double Clock Rate
}

void spi_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len)
// Shift full array through target device
{
    uint8_t i;
    for (i = 0; i < len; i++) {
        SPDR0 = dataout[i];
        while(!(SPSR0 & (1 << SPIF)));
        datain[i] = SPDR0;
    }
}

void spi_transmit_sync (uint8_t * dataout, uint8_t len)
// Shift full array to target device without receiving any byte
{
    uint8_t i;
    for (i = 0; i < len; i++) {
        SPDR0 = dataout[i];
        while(!(SPSR0 & (1 << SPIF)));
    }
}

uint8_t spi_fast_shift (uint8_t data)
// Clocks only one byte to target device and returns the received one
{
    // load data into register
    SPDR0 = data;

    // Wait for transmission complete
    while(!(SPSR0 & (1 << SPIF)));
    
    // return data
    return SPDR0;
}
