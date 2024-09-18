#define SW1 PD7 // Pin 13
#define SW2 PD2 // Pin 4
#define LED1 PB4 // Pin 18
#define LED2 PB5 // Pin 19

#include <avr/io.h>
#include <avr/interrupt.h>

int main(void)
{
	// Configure pins
	DDRD &= ~(1 << SW1); // Set PD7 (SW1) as input
	DDRD &= ~(1 << SW2); // Set PD2 (SW2) as input
	
	DDRB |= (1 << LED1) | (1 << LED2); // Set PB4 (LED1) and PB5 (LED2) as output

	// Initially turn LEDs off
	PORTB &= ~(1 << LED1);
	PORTB &= ~(1 << LED2);

	// External Interrupt on INT0 (PD2)
	EICRA |= (1 << ISC01) | (1 << ISC00); // INT0 on rising edge (SW2)
	EIMSK |= (1 << INT0); // Enable INT0

	sei(); // Enable global interrupts

	while (1)
	{
		// Poll SW1 (PD7) manually
		if ((PIND & (1 << SW1)) == 0) // Button pressed (active low)
		{
			PORTB |= (1 << LED1); // Turn on LED1
		}
		else
		{
			PORTB &= ~(1 << LED1); // Turn off LED1
		}
	}
}

ISR(INT0_vect) // Interrupt Service Routine for INT0 (SW2)
{
	PORTB ^= (1 << LED2); // Toggle LED2
}
