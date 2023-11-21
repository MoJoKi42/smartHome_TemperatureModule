#define RADIO_HEADER_LEN      7


//Datentyp, CRC8, Datenlänge, Daten, DeviceID

#define RADIO_DATATYPE_MQTT_JSON    0x4A
#define RADIO_DATATYPE_RESERVE1     0x2D    // not implemented!
#define RADIO_DATATYPE_RESERVE2     0x2D    // not implemented!
#define RADIO_DATATYPE_RESERVE3     0x2D    // not implemented!


// -- generate header for a radio transmission --
// buffer:      destination buffer for finished HEADER+DATA
//              must be larger than data and header length together!
// data:        source data
// data_length: length of source data
// packet_num:  number of current packet
// packet_cnt:  number of all packets for the full source data
// datatype:    see "RADIO_DATATYPE_..."
// source_id:   address/id of the sender
// dest_id:     address/id of the receiver
uint8_t radio_generate_header(uint8_t* buffer, uint8_t* data, uint8_t data_length, uint8_t packet_num, uint8_t packet_cnt, uint8_t datatype, uint8_t source_id, uint8_t dest_id) {
    
    // header
    buffer[0] = datatype;
    buffer[1] = source_id;
    buffer[2] = dest_id;
    buffer[3] = packet_num;
    buffer[4] = packet_cnt;
    buffer[5] = data_length;
    buffer[6] = 0x00;   // crc8 = 0x00
    
    // data
    for (int i=0; i<data_length; i++)
        buffer[7+i] = data[i];
    
    // crc8
    // do crc8
    buffer[6] = 0x00;           // EDIT !!!
    
    
    
    
    return RADIO_HEADER_LEN + data_length;       // return full length
}
