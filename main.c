/*
 * prelab6.c
 *
 * Created: 19/04/2024 07:12:41
 * Author : asrol
 */ 
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

	// Mostrar los primeros 6 bits en PORTB
	PORTB = bufferRX & 0b00111111; // Utiliza una máscara para obtener los 6 bits menos significativos

	// Mostrar los bits 7 y 8 en PD2 y PD3 de PORTD
	uint8_t upper_bits = (bufferRX >> 6) & 0b11; // Obtiene los bits 7 y 8
	PORTD = (PORTD & ~0b00001100) | (upper_bits << 2); // Limpia y establece los bits PD2 y PD3

	// Responde enviando el carácter recibido de vuelta a la computadora
	writeUart(bufferRX);
	
	// Aquí puedes decidir no hacer nada después de encender las LEDs
	// Permitiendo que las LEDs permanezcan encendidas hasta que se reciba un nuevo carácter
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
