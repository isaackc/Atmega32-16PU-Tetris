#include "led_control.h"

/* 
 * Note: You can simplify this to use only 8 pins instead of 16 pins since they don't run at the
 * same time, but this works fine.
 */
void display(void)
{
	int i;
	int j;

	for(i=2; i<END_LED_1; ++i) {
		RESET_ROWS(LED_GND_0);
		UNLATCH_M1;
		for(j=0; j<COLS; ++j) {
			if(board[i][j])
				SET_BIT(PORTB, DATA_0);
			else
				CLR_BIT(PORTB, DATA_0);
			SHIFT_M1;
		}
		LATCH_M1;
		SET_ROW(LED_GND_0, (i-2));
		avr_wait(1); 	/* Find if needed or not */
	}
	RESET_ROWS(LED_GND_0);
	
	for(i=10; i<END_LED_2; ++i) {
		RESET_ROWS(LED_GND_1);
		UNLATCH_M2;
		for(j=0; j<COLS; ++j) {
			if(board[i][j])
				SET_BIT(PORTA, DATA_1);
			else
				CLR_BIT(PORTA, DATA_1);
			SHIFT_M2;
		}
		LATCH_M2;
		SET_ROW(LED_GND_1, (i-10));
		avr_wait(1);	/* Find if needed or not */
	}
	RESET_ROWS(LED_GND_1);
}
