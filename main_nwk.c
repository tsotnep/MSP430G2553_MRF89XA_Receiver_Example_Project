/*
 * main.h
 *
 *  Created  on: 06.02.2015
 *  Author: Mairo Leier, Maksim Gorev
 *
 *  Version: 0.3		15.10.2015
 */
/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/

#include "system.h"
#include "network.h"
#include "general.h"

// Drivers
#include "uart.h"
#include "spi.h"
#include "radio.h"

// Network
#include "nwk_security.h"
#include "nwk_radio.h"

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/
/*
 * -pwmLED pins are OVERLAPPING with (Wireless module's & LED1_TOGGLE()) -s pins,
 * HIGH probably its better to (change Wireless pins & remove LED1_TOGGLE),
 * BUT before that, for testing, you can simply comment some our pwmLEDs.
 *
 *
 */
//port 1
#define pwmLED1	BIT5
#define pwmLED2	BIT6

//port 2
#define pwmLED3	BIT3
#define pwmLED4	BIT2
#define pwmLED5	BIT4

//timer setup
#define timerZeroStartValue		0b1111111110011011 // -100 => period is 100 counts
#define timerOneStartValue		0b1111111110011011 // -100 => period is 100 counts

/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/
void _init_timer0();
void _init_timer1();
void Print_Error(uint8 error_code);
/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/
uint8 exit_code = 0;		// Exit code number that is set after function exits
uint8 payload_length;

/***************************************************************************************************
 *         Main section                                                                            *
 ***************************************************************************************************/
void main(void) {
	uint8 cntr, len, var;
	uint8 rssi_env, rssi_rx;

	// Initialize system
	Print_Error(System_Init());

	while (1) {

		// Clear received packet buffer
		for (var = RF_BUFFER_SIZE - 1; var > 0; var--)
			RxPacket[var] = 0;

		// Set radio into RX mode
		Radio_Set_Mode(RADIO_RX);

		// Enter to LPM0 w/ interrupts enabled
		// Module stays in low power mode until interrupt is received from radio and wakes up
		__bis_SR_register(LPM0_bits + GIE);

		// Receive data, wait until 100ms until timeout and continue with other tasks
		if (exit_code = Radio_Receive_Data(RxPacket, &len, 100, &rssi_rx)) {
			Print_Error(exit_code);
		} else {

			// Toggle LED1 when data is received
			LED1_TOGGLE()
			;

			UART_Send_Data("\r\nPacket length: ");
			UART0_Send_ByteToChar(&(RxPacket[0]));

			UART_Send_Data("\r\nDestination address: ");
			UART0_Send_ByteToChar(&(RxPacket[1]));

			UART_Send_Data("\r\nSource address: ");
			UART0_Send_ByteToChar(&(RxPacket[2]));

			UART_Send_Data("\r\nMessage: ");
			for (var = 5; var < len; ++var) {
				/*
				 * here we start writing RxPacket[var] -s into our array,
				 * that we will process later - in if-else statement
				 *
				 *
				 *
				 *
				 */
				UART0_Send_ByteToChar(&(RxPacket[var]));
			}

			// Print the RECEIVED_SIGNAL_SRENGTH
			UART_Send_Data("\r\nSignal dtrength:");
			UART0_Send_ByteToChar(&(rssi_rx));

			// Get environemnt RSSI value (noise level)
			rssi_env = Radio_Get_RSSI();
			// Print the NOISE LEVEL
			UART_Send_Data("\r\nEnvironment noise level:");
			UART0_Send_ByteToChar(&(rssi_env));

			UART_Send_Data("\r\n");		// Insert new line to separate packets
		}

		/*
		 * 	here should be if-else statements,
		 * 	and some logic,
		 * 	so that highest-valued-pwm will be written in CCR0 (c) tsotne
		 *
		 *
		 *
		 *
		 */

	}
}

/******************************************************************
 ********************* ROUTINE T I M E R 0 ************************/
//ccr0 - interrupt routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer00_A(void) {
	P2OUT |= (pwmLED1);			// CCR0
}

//ccr1, OverFlow - interrupt routine
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer01_A(void) {
	switch (TA0IV) {
	case TA0IV_TACCR1:			// CCR1
		P2OUT |= (pwmLED2);
		break;
	case TA1IV_TAIFG:			// OverFlow
		P2OUT &= ~(pwmLED1 | pwmLED2);
		TA0R = timerZeroStartValue;
		break;
	}
}

/******************************************************************
 ********************* ROUTINE T I M E R 1 ************************/
//ccr0 - interrupt routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer10_A(void) {
	P2OUT |= (pwmLED3);			// CCR0
}

//ccr1, ccr2, OverFlow - interrupt routine
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer11_A(void) {
	switch (TA1IV) {
	case TA1IV_TACCR1:			// CCR1
		P2OUT |= (pwmLED4);
		break;
	case TA1IV_TACCR2:			// CCR2
		P2OUT |= (pwmLED5);
		break;
	case TA1IV_TAIFG:			// OverFlow
		P2OUT &= ~(pwmLED3 | pwmLED4 | pwmLED5);
		TA1R = timerOneStartValue;
		break;
	}
}

/******************************************************************
 ******************* INITIALIZE T I M E R 0 ***********************/
void _init_timer0() {
	/*
	 * ccr0, 		ccr1, 		ccr2
	 * p1.5-pwmLED4, 	p1.6-pwmLED5,  p1.4//ccr2 is only on port3 that do not exists
	 */
	//Timer0
	P1DIR |= (pwmLED1 | pwmLED2);
	P1OUT &= ~(pwmLED1 | pwmLED2);
	TA0CCR0 = 0B1111111111111100; 	//p1.5
	TA0CCR1 = 0b1111111111001110; 	//p1.6
	TA1R = timerZeroStartValue;
	TA0CCTL0 = CCIE;
	TA0CCTL1 = CCIE;
	TA1CTL = TASSEL_2 | MC_2 | ID_3 | TAIE;
}

/******************************************************************
 ******************* INITIALIZE T I M E R 1 ***********************/
void _init_timer1() {
	/*
	 * ccr0, 		ccr1, 		ccr2
	 * p2.3-pwmLED1,	p2.2-pwmLED2, 	p2.4-pwmLED3
	 */
	//Timer1
	P2DIR |= (pwmLED3 | pwmLED4 | pwmLED5);
	P2OUT &= ~(pwmLED3 | pwmLED4 | pwmLED5);
	TA1CCR2 = 0b1111111111001110; 	//p2.3
	TA1CCR1 = 0b1111111111100111; 	//p2.2
	TA1CCR0 = 0B1111111111111100; 	//p2.4
	TA1R = timerOneStartValue;
	TA1CCTL0 = CCIE;
	TA1CCTL1 = CCIE;
	TA1CCTL2 = CCIE;
	TA1CTL = TASSEL_2 | MC_2 | ID_3 | TAIE;
}

/******************************************************************
 ******************* PRINTING ERROR MESSAGES **********************/
void Print_Error(uint8 error_code) {

	// Print out error code only if it is not 0
	if (error_code) {
		UART_Send_Data("Error code: ");
		UART0_Send_ByteToChar(&error_code);
		UART_Send_Data("\r\n");
		error_code = 0;
	}

	// If packet size has wrong length then reset radio module
	if (error_code == ERR_RX_WRONG_LENGTH) {
		Radio_Init(RF_DATA_RATE, TX_POWER, RF_CHANNEL);
	}

	exit_code = 0;
}

