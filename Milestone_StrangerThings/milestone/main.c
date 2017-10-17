//Matthew Rodriguez, Austin Huang, Seamus Plunkett
#include <msp430.h>

#define red     BIT3;
#define green    BIT4;
#define blue     BIT5;
#define flag      BIT0;

int count = 0;
unsigned int byteCount = 0;
unsigned int numOfBytes = 0;
volatile unsigned int i = 0;
int redNum, greenNum, blueNum;
char Message[80];
//UART Message;
void uartSetUp(void)
{
    //This section initializes UART stuff and was inspired by example code found in code composer
    P2SEL0 &= ~(BIT0 | BIT1);               // Setting to GPIO
    P2SEL1 |= BIT0 | BIT1;                  // USCI_A0 UART operation

    PM5CTL0 &= ~LOCKLPM5;                   // High z

    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;           // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers
    CSCTL0_H = 0;                           // Lock CS registers

    UCA0CTLW0 = UCSWRST;                    // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;             // CLK = SMCLK
    UCA0BRW = 52;
    UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA0CTLW0 &= ~UCSWRST;                  // Initialize eUSCI
    UCA0IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt

}
void LEDSetUp(void)
{
    P1DIR |= flag
    ;       // LED out
    P1OUT &= ~flag
    ;       // Clear LED

    P1DIR |= BIT1;       // LED out
    P1OUT &= ~BIT1;       // Clear LED

    P1DIR |= red
    ;       // P1.3 to output
    P1DIR |= green
    ;     // P1.4 to output
    P1DIR |= blue
    ;      // P1.5 to output
}
void timerSetUp(void)
{
    //Timer A
    TA0CCTL0 = (CCIE);
    TA0CCR0 = 0x0001;
    TA0CTL = TASSEL_2 + MC_1 + ID_2;

}
int main(void)
{

    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

    uartSetUp();
    LEDSetUp();
    timerSetUp();

    //Starting LED duty cycle values.
    redNum = 0;
    greenNum = 0;
    blueNum = 0;

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
    __no_operation();                         // For debugger

}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
{

    if (count == 255)// max count value
    {
        if (redNum != 0)
            P1OUT |= red
        ; //turns on red led
        if (greenNum != 0)
            P1OUT |= green
        ; //turns on green led
        if (blueNum != 0)
            P1OUT |= blue
        ; //turns on blue led
        count = 0; // resets count
    }
    if (count == redNum)
    {
        P1OUT &= ~red
        ; //turns off red led
    }
    if (count == greenNum)
    {
        P1OUT &= ~green
        ; //turns off green led
    }
    if (count == blueNum)
    {
        P1OUT &= ~blue
        ; //turns off blue led
    }

    count++;
    TA0CCTL0 &= ~BIT0;  //clears flag
}


//UART interrupt
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
        break;//nothing happens
    case USCI_UART_UCRXIFG://Interrupt flag
        if (byteCount == 0)//First byte sent in
        {
            P1OUT |= flag
            ;//turn LED on UART starting
            numOfBytes = UCA0RXBUF;
            byteCount++;
        }
        else if ((byteCount > 0) && (byteCount < 4))
        {
            switch (byteCount)
            {
            case 1:
                redNum = UCA0RXBUF;//Duty cycle for red LED set
                break;
            case 2:
                greenNum = UCA0RXBUF;//Duty cycle for green LED set
                break;
            case 3:
                blueNum = UCA0RXBUF;//Duty cycle for blue LED set
                break;

            }
            byteCount++;
        }
        else if ((byteCount > 3) && (byteCount < numOfBytes))
        {
            Message[byteCount + 1] = UCA0RXBUF; //UART message being stored
            UCA0TXBUF = UCA0RXBUF; //UART message being transmitted through TX line
            byteCount++;
        }
        else if (byteCount >= numOfBytes)//Message complete.
        {
            byteCount = 0;
            P1OUT &= ~flag
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
