/*
 * prelab6.c
 *
 * Created: 19/04/2024 07:12:41
 * Author : asrol
 */ 
/*
 * prelab6.c
 *
 * Created: 19/04/2024 07:12:41
 * Author : asrol
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

// Funciones para UART
void writeUart(char caracter);
void initUart(void);
void cadena(char* Texto);

// Funciones para el ADC
void initADC(void);
uint16_t ADC_Read(uint8_t channel);

// Funciones para las opciones
void leerPotenciometro(void);
void enviarASCII(void);
void mostrarMenu(void);

// Variable global para el buffer de recepci�n
volatile uint8_t bufferRX = 0;

// Bandera para salir de la opci�n actual y volver al men�
volatile int salir_opcion = 0;

void initUart(void) {
	// Configura UART a 9600 baudios
	UBRR0 = 103; // Configura velocidad de transmisi�n de 9600 baudios
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bits de datos, sin paridad, 1 bit de parada
}

void writeUart(char caracter) {
	// Espera hasta que el registro de datos est� vac�o
	while (!(UCSR0A & (1 << UDRE0))) {}
	// Env�a el car�cter
	UDR0 = caracter;
}

void cadena(char* Texto) {
	while (*Texto != '\0') {
		writeUart(*Texto);
		Texto++;
	}
}

ISR(USART_RX_vect) {
	// Recibe el car�cter desde UART y lo guarda en bufferRX
	bufferRX = UDR0;
}

void initADC(void) {
	// Configurar el ADC
	ADMUX = (1 << REFS0); // Referencia AVcc (5V) y seleccionar canal ADC0
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Habilitar ADC, prescaler de 128
}

uint16_t ADC_Read(uint8_t channel) {
	// Seleccionar el canal del ADC
	ADMUX &= 0xF0; // Limpiar los 4 bits menos significativos
	ADMUX |= channel;
	
	// Iniciar la conversi�n
	ADCSRA |= (1 << ADSC);
	
	// Esperar a que la conversi�n termine
	while (ADCSRA & (1 << ADSC));
	
	// Retornar el resultado de la conversi�n
	return ADCH;
}

void leerPotenciometro(void) {
	salir_opcion = 0; // Reiniciar la bandera
	
	cadena("Men� de Potenci�metro:\n");
	cadena("1. Ajustar Potenci�metro\n");
	cadena("2. Enviar valor del potenci�metro a la computadora\n");
	
	// Esperar a que el usuario seleccione una opci�n
	while (bufferRX == 0) {}
	char opcion = bufferRX;
	bufferRX = 0;
	
	if (opcion == '1') {
		// Ajustar el potenci�metro
		cadena("Variando el potenci�metro, presione 'b' para volver al men�\n");
		while (!salir_opcion) {
			// Leer el valor del potenci�metro desde el canal ADC0
			uint16_t valorPot = ADC_Read(0);
			
			// Mostrar el valor en los LEDS
			PORTB = (uint8_t)(valorPot & 0x3F);
			uint8_t upper_bits = (valorPot >> 6) & 0b11;
			PORTD = (PORTD & ~0b00001100) | (upper_bits << 2);
			
			// Verificar si se presion� 'b' para salir
			if (bufferRX == 'b') {
				salir_opcion = 1;
				bufferRX = 0;
			}
		}
		} else if (opcion == '2') {
		// Enviar el valor del potenci�metro a la computadora
		cadena("Enviando valor del potenci�metro a la computadora...\n");
		
		uint16_t valorPot = ADC_Read(0);
		
		// Convertir el valor ADC a voltaje real
		float voltaje = (float)valorPot * 5.0 / 1023.0;
		
		// Convertir el voltaje a cadena de texto
		char buffer[10];
		snprintf(buffer, sizeof(buffer), "%.2fV\n", voltaje);
		
		// Enviar el voltaje a la computadora
		cadena(buffer);
		
		cadena("Valor enviado.\n");
		
		// Salir de la opci�n
		salir_opcion = 1;
		bufferRX = 0;
	}
	
	mostrarMenu();
}

void enviarASCII(void) {
	salir_opcion = 0;
	
	cadena("Ingrese un car�cter, presione 'b' para volver al men�\n");
	
	while (!salir_opcion) {
		// Esperar a que se reciba un car�cter desde UART
		while (bufferRX == 0) {}
		
		if (bufferRX == 'b') {
			salir_opcion = 1;
			bufferRX = 0;
			continue;
		}
		
		// Mostrar los 6 bits menos significativos en PORTB
		PORTB = bufferRX & 0b00111111;
		
		// Mostrar los 2 bits m�s significativos en PORTD
		uint8_t upper_bits = (bufferRX >> 6) & 0b11;
		PORTD = (PORTD & ~0b00001100) | (upper_bits << 2);
		
		// Limpiar bufferRX
		bufferRX = 0;
	}
}

void mostrarMenu() {
	cadena("1. Leer Potenci�metro\n");
	cadena("2. Enviar ASCII\n");
}

int main(void) {
	// Habilitar interrupciones globales
	sei();

	// Inicializar UART y ADC
	initUart();
	initADC();

	// Configurar PORTB y PORTD como puertos de salida
	DDRB = 0x3F; // PB0-PB5 como salidas (6 pines)
	DDRD |= 0b00001100; // PD2 y PD3 como salidas (2 pines)

	// Bucle infinito
	while (1) {
		mostrarMenu();
		
		// Esperar hasta que se reciba una selecci�n de men�
		while (bufferRX == 0) {}
		
		if (bufferRX == '1') {
			leerPotenciometro();
			} else if (bufferRX == '2') {
			enviarASCII();
		}
		
		// Limpiar bufferRX despu�s de procesar la selecci�n de men�
		bufferRX = 0;
	}
}
