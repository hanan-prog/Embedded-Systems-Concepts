#ifndef F_CPU
#define F_CPU 3333333
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

// Code in the getting started guide appears to do same thing, but they do not
// know how to code cleanly.
// This is all based on formula in Table 23-1 of the ATmega 3208 data sheet
// Need to shift value left 6 bits to follow format specified in data sheet:
// 16-bit number, 10 bits are whole number part, bottom 6 are fractional part
// The +0.5 is to force the result to be rounded *up* rather than down.
// SAMPLES_PER_BIT: 16, for normal asynchronous mode. Given in data sheet.
#define SAMPLES_PER_BIT 16
#define USART_BAUD_VALUE(BAUD_RATE) (uint16_t) ((F_CPU << 6) / (((float) SAMPLES_PER_BIT) * (BAUD_RATE)) + 0.5)
#define BUF_SIZE 128
#define BLE_RADIO_PROMPT "CMD> "

void twiInit() {
    // SDA && SCL output bits 
    PORTA.DIRSET = PIN2_bm | PIN3_bm;
    
     //Enable Pull-Ups
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN3CTRL = PORT_PULLUPEN_bm;
    
    // host initialization
    TWI0.CTRLA = TWI_SDAHOLD_50NS_gc;
    
    TWI0.MSTATUS = TWI_RIF_bm | TWI_WIF_bm | TWI_CLKHOLD_bm | TWI_RXACK_bm |
            TWI_ARBLOST_bm | TWI_BUSERR_bm | TWI_BUSSTATE_IDLE_gc;
     
    TWI0.MBAUD = 10;

    TWI0.MCTRLA = TWI_ENABLE_bm;
}

void readAccelerometerBytes(uint8_t *dest, uint8_t len) {
    //Send Address
    TWI0.MADDR = (0x19 << 1) | 0;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));

    // specifiying a register address 
    TWI0.MDATA = 0x02; 
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
    
    TWI0.MCTRLB = TWI_MCMD_STOP_gc; 

    //Restart the TWI Bus in READ mode
    TWI0.MADDR |= 1;
    TWI0.MCTRLB = TWI_MCMD_REPSTART_gc;
    
    while (!(TWI0.MSTATUS & TWI_RIF_bm));

    TWI0.MSTATUS = TWI_CLKHOLD_bm;
    uint8_t count = 0;
    while (count < len) {
        while (!(TWI0.MSTATUS & TWI_RIF_bm));
        dest[count] = TWI0.MDATA;
        count++;

        if (count != len) {
            //If not done, then ACK and read the next byte
            TWI0.MCTRLB = TWI_ACKACT_ACK_gc | TWI_MCMD_RECVTRANS_gc;
        }
    }
    
    TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
}



void usartInit() {
    // TODO Fill in with USART peripheral initialization code
    USART0.BAUD = (uint16_t)USART_BAUD_VALUE(9600);
    USART0.CTRLB |= USART_TXEN_bm | USART_RXEN_bm | USART_RXMODE_NORMAL_gc;
    PORTA.DIR &= ~PIN1_bm;
    PORTA.DIR |= PIN0_bm;
}

void usartWriteChar(char c) {
    while (!(USART0.STATUS & USART_DREIF_bm));
    USART0.TXDATAL = c;
}

void usartWriteCommand(const char *cmd) {
    for (uint8_t i = 0; cmd[i] != '\0'; i++) {
        usartWriteChar(cmd[i]);
    }
}

char usartReadChar() {
    while (!(USART0.STATUS & USART_RXCIF_bm)){;}
    return USART0.RXDATAL;
}

void usartReadUntil(char *dest, const char *end_str) {
    // Zero out dest memory so we always have null terminator at end
    memset(dest, 0, BUF_SIZE);
    uint8_t end_len = strlen(end_str);
    uint8_t bytes_read = 0;
    while (bytes_read < end_len || strcmp(dest + bytes_read - end_len, end_str) != 0) {
        dest[bytes_read] = usartReadChar();
        bytes_read++;
    }
}

// Must be called after usartInit()
void bleInit(const char *name) {
    // Put BLE Radio in "Application Mode" by driving F3 high
    PORTF.DIRSET = PIN3_bm;
    PORTF.OUTSET = PIN3_bm;

    // Reset BLE Module - pull PD3 low, then back high after a delay
    PORTD.DIRSET = PIN3_bm | PIN2_bm;
    PORTD.OUTCLR = PIN3_bm;
    _delay_ms(10);
    PORTD.OUTSET = PIN3_bm;

    // The AVR-BLE hardware guide is wrong. Labels this as D3
    // Tell BLE module to expect data - set D2 low
    PORTD.OUTCLR = PIN2_bm;
    _delay_ms(200); // Give time for RN4870 to boot up

    char buf[BUF_SIZE];
    // Put RN4870 in Command Mode
    usartWriteCommand("$$$");
    PORTA.DIRSET = PIN7_bm;
    usartReadUntil(buf, BLE_RADIO_PROMPT);
    PORTA.DIRSET = PIN7_bm;

    // Change BLE device name to specified value
    // There can be some lag between updating name here and
    // seeing it in the LightBlue phone interface
    strcpy(buf, "S-,");
    strcat(buf, name);
    strcat(buf, "\r\n");
    usartWriteCommand(buf);
    usartReadUntil(buf, BLE_RADIO_PROMPT);


    // TODO 1: Send command to remove all previously declared BLE services
    // PZ\r\n
    strcpy(buf, "PZ\r\n");
    usartWriteCommand(buf);
    usartReadUntil(buf, BLE_RADIO_PROMPT);

    // TODO 2: Add a new service. Feel free to use any ID you want from the
    // BLE assigned numbers document. Avoid the "generic" services.
    // PS to add a new service
    // cycling speed and cadence service: 0x1816

    strcpy(buf, "PS,1816\r\n");
    usartWriteCommand(buf);
    usartReadUntil(buf, BLE_RADIO_PROMPT);
    

    // TODO 3: Add a new characteristic to the service for your accelerometer
    // data. Pick any ID you want from the BLE assigned numbers document.
    // Avoid the "generic" characteristics.
       // PC to add a characteristic 
    // PC,hex,0A,02\r\n
    // Aerobic Threshold with hex ID: 0x2A7F, readable and writable, 2 bytes in size
    strcpy(buf, "PC,2A7F,0A,02\r\n");
    usartWriteCommand(buf);
    usartReadUntil(buf, BLE_RADIO_PROMPT);

    // TODO 4: Set the characteristic's initial value to hex "00".
    // SHW: writes a local caharastic value 
    // service handle: 0072
    // value to write: 0
    strcpy(buf, "SHW,0072,00\r\n");
    usartWriteCommand(buf);
    usartReadUntil(buf, BLE_RADIO_PROMPT);
    
}

int main() {
    twiInit();
    usartInit();
    
    // TODO Change the argument to bleInit() to some unique name
    bleInit("Shit");

    int16_t accel[3];
    char buf[BUF_SIZE];

    while (1) {
        readAccelerometerBytes((uint8_t *) accel, 6);
        // Accelerometer readings only contain 12 bits of meaningful data
        accel[0] >>= 4;
        accel[1] >>= 4;
        accel[2] >>= 4;

        // TODO: Update your service's characteristic value according to the
        // accelerometer data.
        // 1g along x axis: Update to hex value "00"
        if (accel[0] >= 900 || accel[0] <= -900) {
            strcpy(buf, "SHW,0072,00\r\n");
            usartWriteCommand(buf);
        } else if (accel[1] >= 900 || accel[1] <= -900) { // 1g along y axis: Update to hex value "01"
            strcpy(buf, "SHW,0072,01\r\n");
            usartWriteCommand(buf);
        } else if (accel[2] >= 900 || accel[2] <= -900) {  // 1g along z axis: Update to hex value "02"
            strcpy(buf, "SHW,0072,02\r\n");
            usartWriteCommand(buf);
        } else {    // Not 1g along any axis: Update to hex value "99"
            strcpy(buf, "SHW,0072,99\r\n");
            usartWriteCommand(buf);
        }
        

        // Wait for service characteristic update operation to complete
        usartReadUntil(buf, BLE_RADIO_PROMPT);
        _delay_ms(1000);
    }
}
