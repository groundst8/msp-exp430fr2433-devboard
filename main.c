#include <msp430.h>

void delay_ms(unsigned int ms);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                      // Stop Watchdog Timer

    // Configure GPIO
    P1DIR |= BIT0;                                 // Set P1.0 as output
    P1OUT &= ~BIT0;                                // Initialize P1.0 to Low

    while (1)
    {
        P1OUT ^= BIT0;                             // Toggle P1.0 (LED)
        delay_ms(100);                             // Delay for 500 ms
    }
}

void delay_ms(unsigned int ms)
{
    unsigned int delay_counts;
    unsigned int current_tar;

    // Configure Timer_A0
    // SMCLK = 1MHz (default)
    // Total divider = /64 (ID__8 and TAIDEX = 7)
    TA0CTL = TASSEL__SMCLK | ID__8 | MC__CONTINUOUS | TACLR;  // SMCLK/8, Continuous mode, Clear TAR
    TA0EX0 = TAIDEX_7;                                        // Additional divide by 8

    // Timer clock frequency = 1MHz / (8 * 8) = 15.625kHz
    // Timer clock period = 1 / 15.625kHz = 64us

    // Compute delay counts
    // delay_counts = (ms * timer_clock_frequency + 500) / 1000
    // Adding 500 for rounding to the nearest integer
    delay_counts = ((unsigned long)ms * 15625 + 500) / 1000;

    // Get current TAR value
    current_tar = TA0R;

    // Set CCR0 = current TAR + delay_counts
    TA0CCR0 = current_tar + delay_counts;

    // Enable CCR0 interrupt
    TA0CCTL0 = CCIE;

    // Enter LPM3
    __bis_SR_register(LPM3_bits | GIE);            // Enter LPM3 with interrupts enabled
    __no_operation();                              // For debugger

    // After delay is over, execution resumes here

    // Disable CCR0 interrupt
    TA0CCTL0 &= ~CCIE;

    // Stop Timer_A0
    TA0CTL = MC__STOP;
}

// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
    // Exit LPM3
    __bic_SR_register_on_exit(LPM3_bits);

    // Clear interrupt flag
    TA0CCTL0 &= ~CCIFG;
}