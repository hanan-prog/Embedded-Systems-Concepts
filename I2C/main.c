#ifndef F_CPU
#define F_CPU 3333333
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>


void initTWI() {
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


int main() {
    volatile int16_t accel[3];
    initTWI();

    // TODO Any further initialization you need
    PORTA.DIRSET = PIN4_bm | PIN5_bm | PIN6_bm;
    
    while (1) {
        readAccelerometerBytes((uint8_t *) accel, 6);
        
        // Accelerometer values only have 12 meaningful bits
        // Need to throw out least significant 4 bits read in for each value
        accel[0]>>=4;
        accel[1]>>=4;
        accel[2]>>=4;
        
        // green Led if -1 or 1g on x axis 
        if (accel[0] >= 900 || accel[0] <= -900) {
            PORTA.OUT |= PIN4_bm;
        } else {
            PORTA.OUT &= ~PIN4_bm;
        }
        
        if (accel[1] >= 900 || accel[1] <= -900) {
            PORTA.OUT |= PIN5_bm;
        } else {
            PORTA.OUT &= ~PIN5_bm;
        }
        
        if (accel[2] >= 900 || accel[2] <= -900) {
            PORTA.OUT |= PIN6_bm;
        } else {
            PORTA.OUT &= ~PIN6_bm;
        }

        _delay_ms(1000);
    }
}
