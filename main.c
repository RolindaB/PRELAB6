/*
 * prelab6.c
 *
 * Created: 19/04/2024 07:12:41
 * Author : asrol
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void writeUart(char caracter);
void initUart(void);
uint8_t bufferRX;

void initUart(void) {
	// Configura UART a 9600 baudios
	UBRR0 = 103; // Configura velocidad de transmisión de 9600 baudios
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bits de datos, sin paridad, 1 bit de parada
}

void writeUart(char caracter) {
	// Espera hasta que el registro de datos esté vacío
	while (!(UCSR0A & (1 << UDRE0))) {}
	
	// Envía el carácter
	UDR0 = caracter;
}

ISR(USART_RX_vect) {
	// Recibe el carácter desde UART y lo guarda en bufferRX
	bufferRX = UDR0;

	// Dividir el carácter recibido en dos partes
	uint8_t lower_bits = bufferRX & 0b00111111; // Los 6 bits menos significativos
	uint8_t upper_bits = (bufferRX >> 6) & 0b11; // Los 2 bits más significativos

	// Mostrar los 6 bits menos significativos en PORTB
	PORTB = lower_bits;

	// Mostrar los 2 bits más significativos en los pines PD2 y PD3 de PORTD
	PORTD = (PORTD & ~0b00001100) | (upper_bits << 2);

	// Añade un retraso para mantener las LEDs encendidas
	_delay_ms(500);

	// Opcionalmente, apaga las LEDs después del retraso
	PORTB = 0; // Apaga todas las LEDs en PORTB
	PORTD = 0; // Apaga las LEDs en PD2 y PD3

	// Responde enviando el carácter recibido de vuelta a la computadora
	writeUart(bufferRX);
}

int main(void) {
	// Habilitar interrupciones globales
	sei();

	// Inicializar UART
	initUart();

	// Configurar PORTB y PORTD como puertos de salida
	DDRB = 0x3F; // Configura PB0-PB5 como salidas (6 pines)
	DDRD |= 0b00001100; // Configura PD2 y PD3 como salidas (2 pines)

	// Bucle infinito
	while (1) {
		// El código se ejecuta en el bucle
	}
}
