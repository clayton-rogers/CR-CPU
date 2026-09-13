// This file contains includes for all the functions contained directly in
// the OS, and thus accessible to all programs.

// Sets the 8 attached LEDs using the lower byte
//void _set_led(int LED);
int __set_led(int LED);

// Writes a character to the uart device, blocking while the device is full
//void _write_uart(int character);
int __write_uart(int character);

// Reads the number of characters in the uart queue
//int _read_uart_tx_size(void);
int __read_uart_tx_size();

// Reads the next character from the uart, blocking until there is one
//int _read_uart(void);
int __read_uart();

// Reads the number of characters in the rx uart queue
//int _read_uart_rx_size(void);
int __read_uart_rx_size();

// Sleeps for the given number of microseconds
//void usleep(int micros);
int usleep(int micros);

// Sleeps for the given number of milliseconds
//void msleep(int millis);
int msleep(int millis);
