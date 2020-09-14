#ifndef LED_CONTROL_GUARD
#define LED_CONTROL_GUARD

#include "./avr.h"
#include "../tetris.h"

/* LED Matrix 0 Shift Registers + */
#define DATA_0 0
#define LATCH_0 1
#define CLK_0 2
#define LED_GND_0 PORTD
#define END_LED_1 10

/* LED Matrix 1 Shift Registers + */
#define DATA_1 1
#define LATCH_1 2
#define CLK_1 3
#define LED_GND_1 PORTC
#define END_LED_2 18

/* Macro Functions */

/* Set PORTD and PORTC to 0xFF since you ground them to turn on LED */
#define INIT_DISPLAY ({\
 	DDRB |= (1<<DATA_0) | (1<<LATCH_0) | (1<<CLK_0);\
	DDRA |= (1<<DATA_1) | (1<<LATCH_1) | (1<<CLK_1);\
	DDRD = 0xFF;\
	DDRC = 0xFF;\
        PORTD = 0xFF;\
	PORTC = 0xFF;\
		})

#define SHIFT(p, i) ({\
	SET_BIT(p, i);\
	CLR_BIT(p, i);\
		})
#define LATCH(p, i) SET_BIT(p, i)
#define UNLATCH(p, i) CLR_BIT(p, i)

#define LATCH_M1 LATCH(PORTB, LATCH_0)
#define UNLATCH_M1 UNLATCH(PORTB, LATCH_0)
#define SHIFT_M1 SHIFT(PORTB, CLK_0);

#define LATCH_M2 LATCH(PORTA, LATCH_1)
#define UNLATCH_M2 UNLATCH(PORTA, LATCH_1)
#define SHIFT_M2 SHIFT(PORTA, CLK_1);

#define RESET_ROWS(p) (p = 0xFF)
#define SET_ROW(p,i) CLR_BIT(p, i)

/* Functions */
void display(void);

#endif
