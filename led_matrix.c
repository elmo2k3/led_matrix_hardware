#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "led_matrix.h"

#define LED_SELECT_PORT PORTA
#define LED_SELECT_DDRD DDRA

#define LED_SELECT_1 PA0
#define LED_SELECT_2 PA1
#define LED_SELECT_3 PA2
#define LED_SELECT_4 PA3

#define LED_CONTROL_PORT PORTC
#define LED_CONTROL_DDRD DDRC

#define LED_RESET PC0
#define LED_BRIGHT PC1
#define LED_CLOCK PC2
#define LED_GREEN PC3
#define LED_RED PC4
#define LED_BRTWRT PC5
#define LED_BRTCLK PC6

uint16_t MODULE_RED[4][16];
uint16_t MODULE_GREEN[4][16];


void output_on_module(uint8_t module)
{
	uint8_t counter,counter2;
	LED_SELECT_PORT |= (1<<module);
//	LED_CONTROL_PORT |= (1<<LED_BRIGHT);

	for(counter2 = 0;counter2 < 16;counter2++)
	{
//		LED_CONTROL_PORT |=  (1<<LED_BRIGHT);
		for(counter = 0; counter < 16; counter++)
		{
			if((MODULE_RED[module][counter2]>>counter)&1)
				LED_CONTROL_PORT |= (1<<LED_RED);
			else
				LED_CONTROL_PORT &= ~(1<<LED_RED);
			if((MODULE_GREEN[module][counter2]>>counter)&1)
				LED_CONTROL_PORT |= (1<<LED_GREEN);
			else
				LED_CONTROL_PORT &= ~(1<<LED_GREEN);
			LED_CONTROL_PORT |= (1<<LED_CLOCK);
			LED_CONTROL_PORT &= ~(1<<LED_CLOCK);
		}
		LED_CONTROL_PORT &= ~(1<<LED_BRIGHT);
//		_delay_us(100);
	}
	LED_SELECT_PORT &= ~(1<<module);
}

void led_init(void)
{
	memset(&MODULE_RED,0,sizeof(MODULE_RED));

	LED_SELECT_DDRD |= (1<<LED_SELECT_1) | 
		(1<<LED_SELECT_2) | 
		(1<<LED_SELECT_3) | 
		(1<<LED_SELECT_4);

	LED_CONTROL_DDRD |= (1<<LED_RESET) | 
		(1<<LED_BRIGHT) |
		(1<<LED_CLOCK) |
		(1<<LED_GREEN) |
		(1<<LED_RED) ;
	//	(1<<LED_BRTWRT) | 
	//	(1<<LED_BRTCLK);
	
	LED_SELECT_PORT = 0;
	LED_CONTROL_PORT = 0;
}

void led_runner(void)
{
	uint8_t counter,counter2;


	/* reset */
	LED_CONTROL_PORT |= (1<<LED_RESET);
	LED_CONTROL_PORT &= ~(1<<LED_RESET);
	
//	for(counter=0;counter<16;counter++)
//	{
//		for(counter2=0;counter2<4;counter2++)
//		{
//			//MODULE_RED[counter2][counter]++;
//			MODULE_GREEN[counter2][counter]++;
//		}
//	}

	output_on_module(LED_SELECT_1);
	output_on_module(LED_SELECT_2);
	output_on_module(LED_SELECT_3);
	output_on_module(LED_SELECT_4);
	//LED_CONTROL_PORT |= (1<<LED_RESET);
//		LED_CONTROL_PORT &= ~(1<<LED_RESET);
//		}
//
}
