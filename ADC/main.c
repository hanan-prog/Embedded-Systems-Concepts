#define F_CPU 3333333
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>



volatile uint8_t cycle = 11111; 
volatile uint8_t adc_val;

void init_timer() {
    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTC_gc;
    

    TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
    TCA0.SINGLE.PER = 11111;
    
    // 2HZ square wave
    TCA0.SINGLE.CMP0 = 5555;
    
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
}

void init_adc() {
    
    /* Disable digital input buffer */
    PORTD.PIN1CTRL &= ~PORT_ISC_gm;
    PORTD.PIN1CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    
    /* Disable pull-up resistor */
    PORTD.PIN1CTRL &= ~PORT_PULLUPEN_bm;

    ADC0.MUXPOS = ADC_MUXPOS_AIN1_gc;

    ADC0.CTRLC |= ADC_PRESC_DIV4_gc;        // prescale
    ADC0.CTRLC |= ADC_REFSEL_VDD_gc;     // internal 
    
    ADC0.CTRLA |= ADC_RESSEL_10BIT_gc;
    
    /* Enable interrupts */
    ADC0.INTCTRL |= ADC_RESRDY_bm;
    
    ADC0.CTRLA |= ADC_FREERUN_bm;
    
    /* Start conversion */
    ADC0.COMMAND = ADC_STCONV_bm;
    
    
}


ISR(ADC0_RESRDY_vect) {
    adc_val = ADC0.RES;
    ADC0.INTFLAGS = ADC_RESRDY_bm;
}



int main(void) {
    PORTC.DIRSET = PIN0_bm;
    PORTD.DIRCLR = PIN1_bm;
    init_timer();
    init_adc();
    
    sei();
    
    while (1) {
        ;
    }
}
