/*
* Servo_controller.c
*
* Created: 2013-07-17 17:31:43
*  Author: mlodedrwale
*/


#include <avr/io.h>
#include <avr/interrupt.h>

//usart definitions
#define USART_BAUDRATE 9600
#define UART_BAUD_SELECT (((F_CPU) + 8UL * (USART_BAUDRATE)) / (16UL * (USART_BAUDRATE)) -1UL)

// loop counters
volatile uint8_t loop_main;
volatile uint8_t counter_mini_loop;
volatile uint8_t whereiam = 1;
volatile uint8_t current_function = 1;

// uart data register
volatile uint8_t uart_count = 0;
volatile uint8_t uart_data[3];
volatile uint8_t start_byte = 0;
volatile uint8_t uart_shift = 0;
volatile uint8_t servo[12];


//naked functions definitions
ISR(TIMER0_OVF_vect)  __attribute__ ((naked));

void mini_loop_1(void) __attribute__ ((naked));
void mini_loop_2(void) __attribute__ ((naked));
void mini_loop_3(void) __attribute__ ((naked));
void mini_loop_4(void) __attribute__ ((naked));

typedef void (* mini_loop_pointer) (void);
mini_loop_pointer current_mini_loop;


//PC5 - servo1
//PC4 - servo2
//PC3 - servo3
void mini_loop_1(void)
{
	if (servo[0] == counter_mini_loop)
	{
		PORTC &= ~(1<<PC5);
	}
	if (servo[1] == counter_mini_loop)
	{
		PORTC &= ~(1<<PC4);
	}
	if (servo[2] == counter_mini_loop)
	{
		PORTC &= ~(1<<PC3);
	}
	asm volatile ("ret");
}

//PC2 - servo4
//PC1 - servo5
//PC0 - servo6
void mini_loop_2(void)
{
	if (servo[3] == counter_mini_loop)
	{
		PORTC &= ~(1<<PC2);
	}
	if (servo[4] == counter_mini_loop)
	{
		PORTC &= ~(1<<PC1);
	}
	if (servo[5] == counter_mini_loop)
	{
		PORTC &= ~(1<<PC0);
	}
	asm volatile ("ret");
}

//PB5 - servo7
//PB4 - servo8
//PB3 - servo9
void mini_loop_3(void)
{
	if (servo[6] == counter_mini_loop)
	{
		PORTB &= ~(1<<PB5);
	}
	if (servo[7] == counter_mini_loop)
	{
		PORTB &= ~(1<<PB4);
	}
	if (servo[8] == counter_mini_loop)
	{
		PORTB &= ~(1<<PB3);
	}
	asm volatile ("ret");
}

//PB2 - servo10
//PB1 - servo11
//PB0 - servo12
void mini_loop_4(void)
{
	if (servo[9] == counter_mini_loop)
	{
		PORTB &= ~(1<<PB2);
	}
	if (servo[10] == counter_mini_loop)
	{
		PORTB &= ~(1<<PB1);
	}
	if (servo[11] == counter_mini_loop)
	{
		PORTB &= ~(1<<PB0);
	}
	asm volatile ("ret");
}

//80us
void lo_loop()
{
	if(counter_mini_loop < 32){
		if ((UCSRA & (1<<RXC)))
		{
			if(!(uart_count == 0 && UDR != 170))
			{
				uart_data[uart_count] = UDR;
				++uart_count;

				if (uart_count > 2)
				{
					if(uart_data[1] >= 'A' && uart_data[1] < 'A' + 12)
					{
						servo[uart_data[1] - 'A'] = uart_data[2];
					}
					uart_count = 0;
				}
			}
		}
		counter_mini_loop += 1;
	}
	else {
		counter_mini_loop = 0;
		whereiam = 2;
		current_function +=1;
		if (current_function == 1)
		{
			current_mini_loop = mini_loop_1;
			PORTC |= (1<<PC5) | (1<<PC4) |(1<<PC3);
		}
		if (current_function == 2)
		{
			current_mini_loop = mini_loop_2;
			PORTC |= (1<<PC2) | (1<<PC1) |(1<<PC0);
		}
		if (current_function == 3)
		{
			current_mini_loop = mini_loop_3;
			PORTB |= (1<<PB5) | (1<<PB4) |(1<<PB3);
		}
		if (current_function == 4)
		{
			current_mini_loop = mini_loop_4;
			PORTB |= (1<<PB2) | (1<<PB1) |(1<<PB0);
			current_function = 0;
		}
	}
}

// turns servo lines high
void hi_loop()
{
	counter_mini_loop +=1;
	if(counter_mini_loop >7)
	{
		TIMSK &= ~(1<<TOIE2);	//disable timer2 overflow interrupt
		counter_mini_loop = 0;
		whereiam = 3;
		TCNT0 = 0xF9;
		TIMSK = 1<<TOIE0;		//enable timer0 overflow interrupt
	}
}

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 0xF9;				// 8us

	current_mini_loop();
	counter_mini_loop += 1;
	
	if (counter_mini_loop == 0) //255 + 1 == 0 !!
	{
		TIMSK &= ~(1<<TOIE0);	//disable timer0 overflow interrupt
		whereiam = 1;
		TCNT2 = 0xB5;			// 80ms
		TIMSK |= (1 << TOIE2);	//enable timer2 overflow interrupt
	}
	asm volatile ("reti");
}

ISR(TIMER2_OVF_vect)
{
	TCNT2 = 0xB5;		// 80ms
	if (whereiam == 1)
	{
		lo_loop();
	}
	if (whereiam == 2)
	{
		hi_loop();
	}
}

void init_servo_values(){
	for (uint8_t i = 0; i<12; i++){
		servo[i] = 128;
	}
}

int main(void)
{
	DDRC = 0xFF;
	PORTC = 0;
	
	DDRB = 0xFF;
	PORTB = 0;
	
	init_servo_values();

	//UART
	UCSRB |= (1 << RXEN);									// Turn on the transmission and reception circuitry
	UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);	// Use 8-bit character sizes
	UBRRH = (UART_BAUD_SELECT >> 8);						// Load upper 8-bits of the baud rate value into the high byte of the UBRR register
	UBRRL = UART_BAUD_SELECT;								// Load lower 8-bits of the baud rate value into the low byte of the UBRR register
	
	//Timer 0
	TCCR0 = (1<<CS01);										//prescaler = 8

	//Timer 2
	TCCR2 |= (1 << CS21);									//prescaler = 8
	TIFR = 1 << TOV2;										//Clear pending interrupts
	TIMSK = 1<<TOIE2;										//enable timer2 overflow interrupt
	TCNT2 = 0xFE;											//do it right now!

	sei();

	while(1)
	{
		//simple ;)
	}
}
