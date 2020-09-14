#include "avr.h"

void avr_wait(unsigned short msec)
{
	TCCR0 = 3; 		/* Timer counter control register */
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.001); /* Timer counter register: stores the "time" to start from */
		SET_BIT(TIFR, TOV0); /* Timer interrupt flag, Timer counter overflow... TOV0 is cleared by writing 1 */
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}
