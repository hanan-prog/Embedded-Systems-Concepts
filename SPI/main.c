#define F_CPU 3333333
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>





void spio_init() {
    PORTA.DIR |= PIN4_bm; // MOSI
    PORTA.DIR |= PIN6_bm; // SCK
    PORTA.DIR |= PIN7_bm; // SS
     
    SPI0.CTRLA =  
        SPI_CLK2X_bm
    | SPI_ENABLE_bm
    | SPI_MASTER_bm
    | SPI_PRESC_DIV4_gc;
}



int main(void) {
    uint8_t data = 0; 
    spio_init();
    int i;
    
//    while(1) {
//        PORTA.OUT &= ~PIN7_bm; // SS low 
//
//        for (i = 6; i >= 0; i--) {
//            if (data & (1 << i)) {
//                PORTA.OUT |= PIN4_bm; // MOSI high
//            } else {
//                PORTA.OUT &= ~PIN4_bm;  // MOSI low
//            }
//
//            
//            PORTA.OUT |= PIN6_bm; // SCK high 
//            _delay_us(500); 
//
//            
//            PORTA.OUT &= ~PIN6_bm; // SCK low
//            _delay_us(500); 
//        }
//        
//        
//        PORTA.OUT |= PIN7_bm; // Set SS pin value to HIGH
//        
//        data++;
//        if (data > 63) {
//            data = 0;
//        }
//        
//        _delay_ms(1000);
//    }
    
    while (1) {
        PORTA.OUT &= ~PIN7_bm; // SET SS pin value to LOW 
      
        
        SPI0.DATA = data;
        data++;
        if (data > 63) {
            data = 0;
        }
        
        while (!(SPI0.INTFLAGS & SPI_IF_bm)) /* Waits until data are exchanged*/
        {
            ;
        }
        PORTA.OUT |= PIN7_bm; // Set SS pin value to HIGH
        
        _delay_ms(1000);
    }
}
