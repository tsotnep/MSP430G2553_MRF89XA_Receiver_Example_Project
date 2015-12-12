/*
 * main_nwk.c
 *
 *  Created  on: 06.02.2015
 *  Author: Mairo Leier, Maksim Gorev
 *	Editors: Tsotne Putkaradze, Nish Tahir
 *
 *  Version: 0.4		7.12.2015
 *
 *  communication: 	https://docs.google.com/spreadsheets/d/1EX6megqmACzNTX3JTMHogJLxSqM1P2WMeiZZyxYM3Lw/edit#gid=0
 *  packet format: 	http://ati.ttu.ee/embsys/images/thumb/a/ae/Variable_length_packet_format.png/600px-Variable_length_packet_format.png
 *  wireless chip: 	http://prntscr.com/9d8lgj
 *
 *  whole project: 	https://github.com/siavooshpayandehazad/TUT_EmbSys_SmartEnv
 *	this part:		https://github.com/tsotnep/MSP430G2553_MRF89XA_Receiver_Example_Project
 *
 *  Family Guide:	http://www.ti.com/lit/ug/slau144j/slau144j.pdf
 *  Datasheet:		http://www.ti.com/lit/ds/symlink/msp430g2553.pdf
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
//port 1
#define pwmLED1_P10_T00	BIT0	//1.0
#define pwmLED2_P11_T01	BIT1	//1.1
#define pwmLED3_P12_T02	BIT2	//1.2


//port 2
#define pwmLED4_P22_T10	BIT2	//2.2
#define pwmLED5_P23_T11	BIT3	//2.3
#define pwmLED6_P24_T12	BIT4	//2.4

//timer setup, TODO: use signed(-100) and similar things, instead of binary values
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
	uint8 len, var;
	uint8 rssi_env, rssi_rx;

	// Initialize system
	Print_Error(System_Init());

	P1DIR |= (pwmLED1_P10_T00 | pwmLED2_P11_T01 | pwmLED3_P12_T02);
	P1OUT &= ~(pwmLED1_P10_T00 | pwmLED2_P11_T01 | pwmLED3_P12_T02);

	P2DIR |= (pwmLED4_P22_T10 | pwmLED5_P23_T11 | pwmLED6_P24_T12);
	P2OUT &= ~(pwmLED4_P22_T10 | pwmLED5_P23_T11 | pwmLED6_P24_T12);

	_init_timer0();
	_init_timer1();

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

//			UART_Send_Data("\r\nPacket length: ");
//			UART0_Send_ByteToChar(&(RxPacket[0]));
//
//			UART_Send_Data("\r\nDestination address: ");
//			UART0_Send_ByteToChar(&(RxPacket[1]));
//
//			UART_Send_Data("\r\nSource address: ");
//			UART0_Send_ByteToChar(&(RxPacket[2]));
//
//			UART_Send_Data("\r\nMessage: ");
			for (var = 5; var < len; ++var) {
				/* TODO
				 * here we start writing RxPacket[var] -s into our array,
				 * that we will process later - in if-else statement
				 *
				 *
				 *
				 *
				 */
//				UART0_Send_ByteToChar(&(RxPacket[var]));
			}

			// Print the RECEIVED_SIGNAL_SRENGTH
//			UART_Send_Data("\r\nSignal dtrength:");
//			UART0_Send_ByteToChar(&(rssi_rx));

			// Get environemnt RSSI value (noise level)
			rssi_env = Radio_Get_RSSI();
			// Print the NOISE LEVEL
//			UART_Send_Data("\r\nEnvironment noise level:");
//			UART0_Send_ByteToChar(&(rssi_env));
//
//			UART_Send_Data("\r\n");		// Insert new line to separate packets
		}
	}
}

/********************* ROUTINE T I M E R 0 ************************/
//ccr0 - interrupt routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer00_A(void) {
	P1OUT |= (pwmLED1_P10_T00);			// CCR0
}

//ccr1, OverFlow - interrupt routine
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer01_A(void) {
	switch (TA0IV) {
	case TA0IV_TACCR1:			// CCR1
		P1OUT |= (pwmLED2_P11_T01);
		break;
	case TA0IV_TACCR2:			// CCR2
		P1OUT |= (pwmLED3_P12_T02);
		break;
	case TA0IV_TAIFG:			// OverFlow
		P1OUT &= ~(pwmLED1_P10_T00);
		P1OUT &= ~(pwmLED2_P11_T01);
		P1OUT &= ~(pwmLED3_P12_T02);
		TA0R = timerZeroStartValue;
		break;
	}
}

/********************* ROUTINE T I M E R 1 ************************/
//ccr0 - interrupt routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer10_A(void) {
	P2OUT |= (pwmLED4_P22_T10);			// CCR0
}

//ccr1, ccr2, OverFlow - interrupt routine
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer11_A(void) {
	switch (TA1IV) {
	case TA1IV_TACCR1:			// CCR1
		P2OUT |= (pwmLED5_P23_T11);
		break;
	case TA1IV_TACCR2:			// CCR2
		P2OUT |= (pwmLED6_P24_T12);
		break;
	case TA1IV_TAIFG:			// OverFlow
		P2OUT &= ~(pwmLED4_P22_T10);
		P2OUT &= ~(pwmLED5_P23_T11);
		P2OUT &= ~(pwmLED6_P24_T12);
		TA1R = timerOneStartValue;
		break;
	}
}

/******************************************************************
 ******************* INITIALIZE T I M E R 0 ***********************/
void _init_timer0() {
	//Timer0
	TA0CCR0 = 0B1111111111111100; 	//p1.0
	TA0CCR1 = 0b1111111111110000; 	//p1.1
	TA0CCR2 = 0b1111111111001110; 	//p1.2
	TA0R = timerZeroStartValue;		//0b1111111110011011
	TA0CCTL0 = CCIE;
	TA0CCTL1 = CCIE;
	TA0CCTL2 = CCIE;
	TA0CTL = TASSEL_2 | MC_2 | ID_3 | TAIE;
}

/******************************************************************
 ******************* INITIALIZE T I M E R 1 ***********************/
void _init_timer1() {
	//Timer1
	TA1CCR0 = 0b1111111111001110; 	//p2.2
	TA1CCR1 = 0b1111111111110000; 	//p2.3
	TA1CCR2 = 0B1111111111111100; 	//p2.4
	TA1R = timerOneStartValue;		//0b1111111110011011
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

