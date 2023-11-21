#include <avr/io.h>
#include <stdlib.h>


// Init
void adc_init(void);

// Read
uint16_t adc_read(uint8_t channel);
uint16_t adc_read_avg(uint8_t channel, uint8_t nsamples);





// ADC initialisieren
void adc_init(void) {

    // Die Versorgungsspannung AVcc (i.d.R. 5V) als Referenz wählen:
    // ADMUX = 0x00;
    // External voltage reference at AREF pin, internal reference turned off (Attiny84):
    // ADMUX = (1<<REFS0);
    // oder interne Referenzspannung (1,1V) als Referenz für den ADC wählen:
    // ADMUX = (1<<REFS1) | (1<<REFS0);
    // oder interne Referenzspannung (1,1V) als Referenz für den ADC wählen (Atmega324PB):
    ADMUX = (1<<REFS1);

    // Bit ADFR ("free running") in ADCSRA steht beim Einschalten
    // schon auf 0, also single conversion
    ADCSRA = (1<<ADPS1) | (1<<ADPS0);     // Frequenzvorteiler = 8
    ADCSRA |= (1<<ADEN);                  // ADC aktivieren

    // nach Aktivieren des ADC wird ein "Dummy-Readout" empfohlen, man liest
    // also einen Wert und verwirft diesen, um den ADC "warmlaufen zu lassen" */
    ADCSRA |= (1<<ADSC);                // eine ADC-Wandlung 
    while (ADCSRA & (1<<ADSC) ) {}      // auf Abschluss der Konvertierung warten
  
    // ADCW muss einmal gelesen werden, sonst wird Ergebnis der nächsten
    // Wandlung nicht übernommen
    (void) ADCW;
}

// ADC Einzelmessung
uint16_t adc_read( uint8_t channel ) {

    // Kanal waehlen, ohne andere Bits zu beeinflussen
    ADMUX = (ADMUX & ~(0x1F)) | (channel & 0x1F);
    ADCSRA |= (1<<ADSC);                // eine Wandlung "single conversion"
    while (ADCSRA & (1<<ADSC) ) {}      // auf Abschluss der Konvertierung warten
    return ADCW;                        // ADC auslesen und zurückgeben
}

// ADC Mehrfachmessung mit Mittelwertbbildung
// beachte: Wertebereich der Summenvariablen
uint16_t adc_read_avg( uint8_t channel, uint8_t nsamples ) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < nsamples; ++i ) {
        sum += adc_read( channel );
    }
    return (uint16_t)( sum / nsamples );
}
