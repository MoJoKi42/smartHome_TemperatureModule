#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "radio.h"
#include "twi.h"
#include "RFM69.h"
#include "RFM69registers.h"
#include "sht4x.h"
#include "adc.h"

// Commands :
//
// -> Compile
// make all
//
// -> Flashing
// avrdude -p m324pb -c usbasp -e -U flash:w:main.hex
// avrdude -p m324pb -c dragon_isp -e -U flash:w:main.hex
//
// -> Write Fuses
// avrdude -c usbasp -p m324pb -U lfuse:w:0x42:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m
// avrdude -c dragon_isp -p m324pb -U lfuse:w:0x42:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m
//
// -> VS Code Include Pfad
// "/usr/lib/avr/include"
//

// Port definitions
#define LED_R_off    PORTC &= ~(1<<PC2)
#define LED_R_on     PORTC |=  (1<<PC2)
#define LED_G_off    PORTC &= ~(1<<PC3)
#define LED_G_on     PORTC |=  (1<<PC3)
#define LED_B_off    PORTC &= ~(1<<PC4)
#define LED_B_on     PORTC |=  (1<<PC4)

#define PowerON      PORTD |=  (1<<PD4)
#define PowerOFF     PORTD &= ~(1<<PD4)

#define CS_ADC_en    DDRD |=  (1<<PD6)
#define CS_ADC_dis   DDRD &=  ~(1<<PD6)
#define CS_RFM_en    DDRD |=  (1<<PD7)  // see "RFM69.h" on line 39
#define CS_RFM_dis   DDRD &=  ~(1<<PD7) // see "RFM69.h" on line 39


// Prototypes
void sleep_init(void);
void sleep_start(uint16_t duration);


// RFM69 definitions
#define MaxRetries          1
#define MaxRetryWaitTime    250  // max 250ms  
#define NETWORKID           33
const char EncryptKey[16] = "1234567812345678";
uint8_t    deviceID       = 0x20;
uint8_t    baseID         = 0x01;   // ID of base station



// ToDo:
// - Datenheader anpassen
// - Schauen welche Delays verkürzt werden können!



int main(void) {

    // Outputs
    DDRC  |= (1<<PC2)|(1<<PC3)|(1<<PC4);            // Output LEDs
    DDRD  |= (1<<PD4);                              // Output PowerON Pin
    PORTB |= (1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3);   // Enable Address PullUps
    CS_ADC_dis;
    CS_RFM_dis;
    
    PowerON;
    _delay_ms(200);
    
    // get DeviceID 
    deviceID |= (~PINB & 0x0F);
    if (deviceID == 0x20) {
        while(1) {
            LED_R_on;       // address 0x20 not allowed!
            _delay_ms(200);
            LED_R_off;
            _delay_ms(200);
        }
    }
    
    // init Sleep
    sleep_init();
    
    
    while (1) {
        
        // enable LED
        LED_B_on;
        
        // init RFM69
        rfm69_init(433, deviceID, NETWORKID);
        setHighPower(1);    // if model number rfm69hw
        setPowerLevel(30);  // 0-31; 5dBm to 20 dBm
        encrypt(EncryptKey);
        
        // init SHT41
        sht4x_init_twi();
        
        // measurement SHT41
        sht4x_send_cmd(0xFD);
        _delay_ms(200);
        uint8_t data[6];
        float temperature;
        float humidity;
        if (sht4x_read_data(data)) {
            temperature =  0.0;   // got invalid data
            humidity = 0.0;       // got invalid data
        } else {
            temperature =  sht4x_get_temperature_from_data(data);
            humidity =     sht4x_get_humidity_from_data(data);
        }
        
        // measurement batteryVoltage
        adc_init();
        float batteryVoltage = adc_read_avg(0, 10);
        batteryVoltage = batteryVoltage * (5.0 * 1.1 / 1023.0);
        
        
        // generate json string
        char str[200];
        sprintf(str, "{\"deviceID_hex\":%x,\"uptime\":%d,\"batteryVoltage\":%0.3f,\"temperature\":%0.2f,\"humidity\":%0.2f}",
                         deviceID,           0,            batteryVoltage,          temperature,          humidity);
        
        
        // get number of single packets
        uint16_t data_length = strlen(str);
        uint8_t MAX_DATA_LEN = RF69_MAX_DATA_LEN - RADIO_HEADER_LEN;
        uint8_t packet_cnt = data_length/MAX_DATA_LEN;
        if (data_length % MAX_DATA_LEN)
            packet_cnt++;
        
        // send packets
        uint8_t successful_transmitted = 1;
        for (int packet=0; packet < packet_cnt; packet++) {
            
            // calculate packet length
            uint8_t packet_lenght;
            if (packet == packet_cnt-1 && data_length % MAX_DATA_LEN) {
                packet_lenght = data_length % MAX_DATA_LEN;
            } else {
                packet_lenght = MAX_DATA_LEN;
            }
            
            // add data header
            uint8_t buffer[RF69_MAX_DATA_LEN];
            uint8_t buffer_len = radio_generate_header(buffer, str + (packet*MAX_DATA_LEN), packet_lenght, packet, packet_cnt, RADIO_DATATYPE_MQTT_JSON, deviceID, baseID);
        
            // send data
            uint8_t statusACK = 0;
            uint8_t retry = 0; uint8_t timeRetry = 0;
            while (retry < MaxRetries) {
                
                // send
                send(baseID, buffer, buffer_len, 1);
                
                // check ACK + wait
                timeRetry = 0;
                while (timeRetry < MaxRetryWaitTime) {
                    statusACK = ACKReceived(baseID);
                    if (statusACK) { break; }
                    _delay_ms(1);
                    timeRetry++;
                }
                
                // abort next retry when ACK received
                if (statusACK) { break; }
                retry++;
            }
            
            // reset "successful_transmitted"-variable when one ACK is missing
            if (!statusACK)
                successful_transmitted = 0;
        }
        
        // power off
        PowerOFF;
        
        // show ACK/NACK with LEDs
        LED_B_off;
        if (successful_transmitted) {
            LED_G_on;
        } else {
            LED_R_on;
        }
        _delay_ms(100);
        LED_R_off;
        LED_G_off;
        
        // sleep
        sleep_start(8);
        PowerON;
        _delay_ms(100);
    }
    
}


////////////////////////////////////////////////////////////////////////////////////////////
// sleep functions
////////////////////////////////////////////////////////////////////////////////////////////

// init sleep registers
void sleep_init(void) {
    
    ASSR = (1<<AS2);                                // select asynchronous operation of Timer2
    TCCR2A = 0x00;                                  // Normal mode
    TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20) ;    // Prescaler = 1024
    TIMSK2 = (1<<TOIE2);                            // Timer0 overflow interrup enable
    TCNT2 =  0;                                     // Preload Value
    
    while((ASSR & (1<<OCR2AUB)) || (ASSR & (1<<OCR2BUB)));  // wait
    while((ASSR & (1<<TCR2AUB)) || (ASSR & (1<<TCR2BUB)));  // wait
    while((ASSR & (1<<TCN2UB)));                            // wait
}

// Timer2 overflow Interrupt (sleep functions)
ISR(TIMER2_OVF_vect) { }

// go sleeping function
// duration = 8...2048 seconds (in 8 seconds steps)
void sleep_start(uint16_t duration) {
    
    // set sleep duration
    uint8_t sleep_counter = 0;
    if (duration >= 2048) {
        sleep_counter = 255;
    } else {
        sleep_counter = duration / 8;
    }
    
    sei();                                  // enable interrups
    for (int i=0; i<sleep_counter; i++) {
        set_sleep_mode(SLEEP_MODE_PWR_SAVE);    // set sleep mode
        sleep_mode();                           // go sleeping
    }
    cli();                                  // disable intrrups
}


