#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define LCD_PORT PORTC
#define LCD_DDR DDRC
#define LCD_PIN PINC
#define RS PC0
#define RW PC1
#define EN PC2

// LCD functions with corrected pin assignments
void lcd_command(unsigned char cmd) {
	// Upper nibble first
	LCD_PORT = (LCD_PORT & 0x0F) | (cmd & 0xF0);  // Keep lower bits, send upper nibble to PC7-PC4
	LCD_PORT &= ~(1 << RS);    // RS = 0 for command
	LCD_PORT &= ~(1 << RW);    // RW = 0 for write
	LCD_PORT |= (1 << EN);     // EN = 1
	_delay_us(1);
	LCD_PORT &= ~(1 << EN);    // EN = 0
	_delay_us(200);

	// Lower nibble
	LCD_PORT = (LCD_PORT & 0x0F) | ((cmd << 4) & 0xF0);  // Keep lower bits, send lower nibble to PC7-PC4
	LCD_PORT |= (1 << EN);     // EN = 1
	_delay_us(1);
	LCD_PORT &= ~(1 << EN);    // EN = 0
	_delay_us(200);
}

void lcd_data(unsigned char data) {
	// Upper nibble first
	LCD_PORT = (LCD_PORT & 0x0F) | (data & 0xF0);  // Keep lower bits, send upper nibble to PC7-PC4
	LCD_PORT |= (1 << RS);     // RS = 1 for data
	LCD_PORT &= ~(1 << RW);    // RW = 0 for write
	LCD_PORT |= (1 << EN);     // EN = 1
	_delay_us(1);
	LCD_PORT &= ~(1 << EN);    // EN = 0
	_delay_us(200);

	// Lower nibble
	LCD_PORT = (LCD_PORT & 0x0F) | ((data << 4) & 0xF0);  // Keep lower bits, send lower nibble to PC7-PC4
	LCD_PORT |= (1 << EN);     // EN = 1
	_delay_us(1);
	LCD_PORT &= ~(1 << EN);    // EN = 0
	_delay_us(200);
}

void lcd_init(void) {
	LCD_DDR = 0xFF;    // Set all pins as output
	_delay_ms(20);     // Wait for LCD to power up

	// Initialize in 4-bit mode
	LCD_PORT = 0x30;
	lcd_command(0x03);
	_delay_ms(5);
	lcd_command(0x03);
	_delay_us(150);
	lcd_command(0x03);
	lcd_command(0x02);    // Set to 4-bit mode
	
	lcd_command(0x28);    // 4-bit mode, 2 lines, 5x7 dots
	lcd_command(0x0C);    // Display ON, cursor OFF
	lcd_command(0x06);    // Increment cursor
	lcd_command(0x01);    // Clear display
	_delay_ms(2);
}

void lcd_string(const char *str) {
	while (*str) {
		lcd_data(*str++);
		_delay_ms(1);
	}
}

// Keypad setup and scanning
void keypad_init(void) {
	DDRF = 0xF0;     // PF7-PF4 outputs (rows), PF3-PF0 inputs (columns)
	PORTF = 0x0F;    // Enable pull-ups on inputs
}

char keypad_scan(void) {
	static const char keymap[4][4] = {
		{'1', '2', '3', ' '},
		{'4', '5', '6', ' '},
		{'7', '8', '9', ' '},
		{'*', '0', '#', ' '}
	};

	for (uint8_t row = 0; row < 4; row++) {
		PORTF = 0x0F | ~(0x10 << row);    // Set current row low, others high
		_delay_ms(1);                      // Allow signal to settle

		uint8_t cols = PINF & 0x0F;        // Read columns
		if (cols != 0x0F) {                // If any column is low
			for (uint8_t col = 0; col < 4; col++) {
				if (!(cols & (1 << col))) {
					while (!(PINF & (1 << col))); // Wait for key release
					_delay_ms(20);               // Debounce
					return keymap[row][col];
				}
			}
		}
	}
	return 0; // No key pressed
}

int main(void) {
	// Initialize LCD and keypad
	lcd_init();
	keypad_init();

	// Display initial menu
	lcd_command(0x80);    // Move to first line
	lcd_string("* Washroom");
	lcd_command(0xC0);    // Move to second line
	lcd_string("# Rent Menu");

	char key;
	while (1) {
		key = keypad_scan();
		if (key == '*') {
			lcd_command(0x01);    // Clear display
			lcd_string("Washroom Menu");
			_delay_ms(500);
			
			// Return to main menu
			lcd_command(0x01);
			lcd_command(0x80);
			lcd_string("* Washroom");
			lcd_command(0xC0);
			lcd_string("# Rent Menu");
		}
		else if (key == '#') {
			lcd_command(0x01);    // Clear display
			lcd_string("Rent Menu");
			_delay_ms(500);
			
			// Return to main menu
			lcd_command(0x01);
			lcd_command(0x80);
			lcd_string("* Washroom");
			lcd_command(0xC0);
			lcd_string("# Rent Menu");
		}
	}
	return 0;
}