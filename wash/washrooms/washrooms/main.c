#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

#define LCD_PORT PORTC
#define LCD_DDR DDRC
#define LCD_PIN PINC
#define RS PC0
#define RW PC1
#define EN PC2

char permanent_code[5] = "1234"; // Default permanent passcode
char temporary_code[5]; // Temporary code for client access
uint8_t rent_paid = 1;  // Simulate rent payment status (1 = paid, 0 = unpaid)

void lcd_command(unsigned char cmd) {
	LCD_PORT = (LCD_PORT & 0x0F) | (cmd & 0xF0);
	LCD_PORT &= ~(1 << RS);
	LCD_PORT &= ~(1 << RW);
	LCD_PORT |= (1 << EN);
	_delay_us(1);
	LCD_PORT &= ~(1 << EN);
	_delay_us(200);

	LCD_PORT = (LCD_PORT & 0x0F) | ((cmd << 4) & 0xF0);
	LCD_PORT |= (1 << EN);
	_delay_us(1);
	LCD_PORT &= ~(1 << EN);
	_delay_us(200);
}

void lcd_data(unsigned char data) {
	LCD_PORT = (LCD_PORT & 0x0F) | (data & 0xF0);
	LCD_PORT |= (1 << RS);
	LCD_PORT &= ~(1 << RW);
	LCD_PORT |= (1 << EN);
	_delay_us(1);
	LCD_PORT &= ~(1 << EN);
	_delay_us(200);

	LCD_PORT = (LCD_PORT & 0x0F) | ((data << 4) & 0xF0);
	LCD_PORT |= (1 << EN);
	_delay_us(1);
	LCD_PORT &= ~(1 << EN);
	_delay_us(200);
}

void lcd_init(void) {
	LCD_DDR = 0xFF;
	_delay_ms(20);

	LCD_PORT = 0x30;
	lcd_command(0x03);
	_delay_ms(5);
	lcd_command(0x03);
	_delay_us(150);
	lcd_command(0x03);
	lcd_command(0x02);

	lcd_command(0x28);
	lcd_command(0x0C);
	lcd_command(0x06);
	lcd_command(0x01);
	_delay_ms(2);
}

void lcd_string(const char *str) {
	while (*str) {
		lcd_data(*str++);
		_delay_ms(1);
	}
}

void keypad_init(void) {
	DDRF = 0xF0;
	PORTF = 0x0F;
}

char keypad_scan(void) {
	static const char keymap[4][3] = {
		{'1', '2', '3'},
		{'4', '5', '6'},
		{'7', '8', '9'},
		{'*', '0', '#'}
	};

	for (uint8_t row = 0; row < 4; row++) {
		PORTF = 0x0F | ~(0x10 << row);
		_delay_ms(1);

		uint8_t cols = PINF & 0x07;
		if (cols != 0x07) {
			for (uint8_t col = 0; col < 3; col++) {
				if (!(cols & (1 << col))) {
					while (!(PINF & (1 << col))); // Wait for key release
					_delay_ms(20);
					return keymap[row][col];
				}
			}
		}
	}
	return 0;
}

void set_permanent_code() {
	lcd_command(0x01);
	lcd_string("Enter new code:");
	for (int i = 0; i < 4; i++) {
		permanent_code[i] = keypad_scan();
		lcd_data(permanent_code[i]);
		_delay_ms(2000);
	}
	permanent_code[4] = '\0';
	lcd_command(0x01);
	lcd_string("Code Updated");
	_delay_ms(2000);
}

void generate_temporary_code() {
	lcd_command(0x01);
	lcd_string("Temp Code: ");
	for (int i = 0; i < 4; i++) {
		temporary_code[i] = '0' + (rand() % 10);
		lcd_data(temporary_code[i]);
		_delay_ms(2000);
	}
	temporary_code[4] = '\0';
	_delay_ms(3000);
}

void access_washroom(char *entered_code) {
	if (strcmp(code, permanent_code) == 0 || strcmp(code, temporary_code) == 0) {
		lcd_command(0x01);
		lcd_string(" Access Granted");
		} else {
		lcd_command(0x01);
		lcd_string(" Access Denied");
	}
	_delay_ms(2000);
}

void check_rent_status() {
	lcd_command(0x01);
	if (rent_paid) {
		lcd_string(" Rent Paid");
		} else {
		lcd_string(" Rent Not Paid");
	}
	_delay_ms(2000);
}
///////////////////////////////////////////////////////////////////////////////
void main_menu() {
	lcd_command(0x01); // Clears the LCD screen
	lcd_command(0x80); // Sets the cursor to the beginning of the first line
	lcd_string(" *: Washroom #: Rent");
	lcd_command(0xC0); // Sets the cursor to the beginning of the second line
	lcd_string(" 1: Mgmt"); // Displays "1: Management" on the second line
	lcd_command(0x94); // Sets the cursor to the third line
	lcd_string(" 2: Tenant"); // Displays "2: Tenant" on the second line
	lcd_command(0xD4);
	lcd_string(" 3: Client");
}
///////////////////////////////////////////////////////////////////////////////
void management_menu() {
	lcd_command(0x01);
	lcd_string(" * Set Code");
	lcd_command(0xC0);
	lcd_string(" # Check Rent");
	_delay_ms(2000);    // Wait for 2 seconds to allow the user to read
	
	// Wait for user to press a key
	char key = keypad_scan();
	if (key == '*') {
		set_permanent_code();  // Call the function to set the new code
		} else if (key == '#') {
		check_rent_status();   // Check rent status if '#' is pressed
	}
	
	// Return to main menu after the action is completed
	main_menu();
}

void tenant_menu() {
	lcd_command(0x01);
	lcd_string(" * Gen Temp Code");
	lcd_command(0xC0);
	lcd_string("# Access Washroom");
}

void client_menu() {
	lcd_command(0x01);
	lcd_string(" Enter Code:");
}

int main(void) {
	lcd_init();
	keypad_init();
	main_menu();

	char key;
	while (1) {
		key = keypad_scan();
		if (key == '1') {
			management_menu();
			key = keypad_scan();
			if (key == '*') {
				set_permanent_code();
				} else if (key == '#') {
				check_rent_status();
			}
			main_menu();
			} else if (key == '2') {
			tenant_menu();
			key = keypad_scan();
			if (key == '*') {
				generate_temporary_code(); // Generate temporary code
				} else if (key == '#') {
				lcd_command(0x01);
				lcd_string(" Enter Code:");
				char entered_code[5] = {0};
				for (int i = 0; i < 4; i++) {
					entered_code[i] = keypad_scan();
					lcd_data(entered_code[i]);
					_delay_ms(2000);
				}
				access_washroom(entered_code);
			}
			main_menu();
			} else if (key == '3') {
			client_menu();
			char entered_code[5] = {0};
			for (int i = 0; i < 4; i++) {
				entered_code[i] = keypad_scan();
				lcd_data(entered_code[i]);
				_delay_ms(500);
			}
			access_washroom(entered_code);
			main_menu();
		}
	}
	return 0;
}
