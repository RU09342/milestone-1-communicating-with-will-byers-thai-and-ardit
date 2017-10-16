#include <msp430.h>

int TimerCount = 0;
unsigned int byteCount = 0;
unsigned int numOfBytes = 0;
volatile unsigned int i = 0;
int redPWM, greenPWM, bluePWM;
char Message[80];
//UART Message;

int main(void)
{

    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT



    P2SEL0 &= ~(BIT0 | BIT1);
       P2SEL1 |= BIT0 | BIT1;                  // USCI_A0 UART operation
       PM5CTL0 &= ~LOCKLPM5;
       CSCTL0_H = CSKEY_H;                     // Unlock CS registers
       CSCTL1 = DCOFSEL_3 | DCORSEL;           // Set DCO to 8MHz
       CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
       CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers
       CSCTL0_H = 0;                           // Lock CS registers
       UCA0CTLW0 = UCSWRST;                    // Put eUSCI in reset
       UCA0CTLW0 |= UCSSEL__SMCLK;             // CLK = SMCLK
       UCA0BRW = 52;                           // 8000000/16/9600
       UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
       UCA0CTLW0 &= ~UCSWRST;                  // Initialize eUSCI
       UCA0IE |= UCRXIE;CSCTL0_H = CSKEY_H;
    CSCTL1 = DCOFSEL_3 | DCORSEL;
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;
    CSCTL0_H = 0;
    UCA0CTLW0 = UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK;
    UCA0BRW = 52;
    UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA0CTLW0 &= ~UCSWRST;
    UCA0IE |= UCRXIE;

    P1DIR |= BIT0;       // LED out
    P1OUT &= ~BIT0;       // Clear LED

    P1DIR |= BIT1;       // LED out
    P1OUT &= ~BIT1;       // Clear LED

    P1DIR |= BIT3;       // P1.3 to output
    // P1SEL |= BIT3;       // P1.3 to TA0.1
    P1DIR |= BIT4;     // P1.4 to output
    // P1SEL |= BIT4;     // P1.4 to TA0.2
    P1DIR |= BIT5;      // P1.5 to output
    // P1SEL |= BIT5;      // P1.5 to TA0.3

    TA0CCTL0 = (CCIE);
    TA0CCR0 = 0x0001;
    TA0CTL = TASSEL_2 + MC_1 + ID_3;

    redPWM = 1;
    greenPWM = 1;
    bluePWM = 1;

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
    __no_operation();                         // For debugger

}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
{

    if (TimerCount == 255)
    {
        P1OUT |= BIT3
        ; //turns on BIT3 led
        P1OUT |= BIT4
        ; //turns on BIT4 led
        P1OUT |= BIT5
        ; //turns on BIT5 led
        TimerCount = 0;
    }
    if (TimerCount == redPWM)
    {
        P1OUT &= ~BIT3
        ; //turns off BIT3 led
    }
    if (TimerCount == greenPWM)
    {
        P1OUT &= ~BIT4
        ; //turns off BIT4 led
    }
    if (TimerCount == bluePWM)
    {
        P1OUT &= ~BIT5
        ; //turns off BIT5 led
    }

    TimerCount++;
    TA0CCTL0 &= ~BIT0;  //clears BIT0
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
    {
    case USCI_NONE:
        break;
    case USCI_UART_UCRXIFG:
        if (byteCount == 0)
        {
            P1OUT |= BIT0
            ;
            numOfBytes = UCA0RXBUF;
            byteCount++;
        }
        else if ((byteCount > 0) && (byteCount < 4))
        {
            switch (byteCount)
            {
            case 1:
                redPWM = UCA0RXBUF;
                break;
            case 2:
                greenPWM = UCA0RXBUF;
                break;
            case 3:
                bluePWM = UCA0RXBUF;
                break;

            }
            byteCount++;
        }
        else if ((byteCount > 3) && (byteCount < numOfBytes))
        {
            Message[byteCount + 1] = UCA0RXBUF;
            UCA0TXBUF = UCA0RXBUF;
            byteCount++;
        }
        else if (byteCount >= numOfBytes)
        {
            byteCount = 0;
            P1OUT &= ~BIT0
            ;
        }
        __no_operation();
        break;
    case USCI_UART_UCTXIFG:
        break;
    case USCI_UART_UCSTTIFG:
        break;
    case USCI_UART_UCTXCPTIFG:
        break;
    }
}
