//  MSP430FR5994 MileStone
//      Communication using UART
//
//   Description:
//
//   ACLK = 32.768kHz, MCLK = SMCLK = default DCO~1MHz
//
//                MSP430FR2311
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST              |
//            |                 |
//            |             P1.0|-->LED
//
//   Thai Nghiem
//   Rowan University
//   September 2017
//   Built with CCSv4 and IAR Embedded Workbench Version: 4.21
//******************************************************************************
#include <msp430.h>

int TimerCount = 0;
unsigned int byteCount = 0;
unsigned int numOfBytes = 0; //Maximum amount of Bytes
volatile unsigned int i = 0;
int redPWM, greenPWM, bluePWM;
char Message[80];  //UART Message;

void uartInitial(void);
void ledinitial (void);
void timerIntitial(void);

int main(void)
{

    WDTCTL = WDTPW + WDTHOLD;                 //Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                    // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    uartInitial();
    ledInitial();
    timerInitial();


    //Initialize PWM for each LEDs
    redPWM = 1;
    greenPWM = 1;
    bluePWM = 1;

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
    __no_operation();                         // For debugger

}

// USCI_A0 UART operation and intialization (inspired by William Goh in his/her code of
//                    "eUSCI_A0 UART echo at 9600 baud"
void uartInitial(void){
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
}

void ledInitial(void){
    P2SEL0 &= ~(BIT0 | BIT1);                // PWM output to LED Port 2
    P2SEL1 |= BIT0 | BIT1;

    P1DIR |= BIT0;        // Set P1.0 to output direction
    P1OUT &= ~BIT0;       // Switch LED off

    P1DIR |= BIT1;       //set Port 1.1 output ---LED
    P1OUT &= ~BIT1;       // Swich LED off

    P1DIR |= BIT3;       // P1.3 to output for Red LED
    P1DIR |= BIT4;     // P1.4 to output for Green LED
    P1DIR |= BIT5;      // P1.5 to output for Blue LED
}

void timerInitial(void){
    TA0CCTL0 = (CCIE); //Timer A capture/control Interrupt Enable
    TA0CCR0 = 0x0001; // Initialize CCRO
    TA0CTL = TASSEL_2 + MC_1 + ID_3; // SMCLK / Upmode / Divider of 8
}

//uart interrupt vector
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch (__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
    {
    case USCI_NONE:
        break;
    case USCI_UART_UCRXIFG:
        if (byteCount == 0) //First message define maximum amount of bytes
        {
            P1OUT |= BIT0; // Turn on the P1.0 LED
            numOfBytes = UCA0RXBUF; // Message length going to be recieved
            byteCount++; // Increment to get the next message
        }
        else if ((byteCount > 0) && (byteCount < 4))
        {
            switch (byteCount)
            {
            case 1:
                redPWM = UCA0RXBUF; //get the value for red PWM from Buffer
                break;
            case 2:
                greenPWM = UCA0RXBUF; //get the value for green PWM from Buffer
                break;
            case 3:
                bluePWM = UCA0RXBUF; //get the value for blue PWM from Buffer
                break;

            }
            byteCount++; // Increment to get the next message
        }
        else if ((byteCount > 3) && (byteCount < numOfBytes))
        {
            Message[byteCount + 1] = UCA0RXBUF; // Store the message
            UCA0TXBUF = UCA0RXBUF; // Transmit the message
            byteCount++; // Increment to get the next message
        }
        else if (byteCount >= numOfBytes)
        {
            byteCount = 0; // Reset to prepare for the next message
            P1OUT &= ~BIT0; // Turn off the P1.0 LED
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
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
{

    if (TimerCount == 255)
    {
        P1OUT |= BIT3; //turns on BIT3 led
        P1OUT |= BIT4; //turns on BIT4 led
        P1OUT |= BIT5; //turns on BIT5 led
        TimerCount = 0;
    }
    if (TimerCount == redPWM)
    {
        P1OUT &= ~BIT3; //turns off BIT3 led
    }
    if (TimerCount == greenPWM)
    {
        P1OUT &= ~BIT4; //turns off BIT4 led
    }
    if (TimerCount == bluePWM)
    {
        P1OUT &= ~BIT5; //turns off BIT5 led
    }

    TimerCount++;
    TA0CCTL0 &= ~BIT0;  //clears BIT0
}

