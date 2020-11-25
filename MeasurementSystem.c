/*
 * Measurement_System.c
 *
 * Created: 5/12/2020 5:17:18 PM
 *  Author: Mohab Mohamed Mostafa
 *	Section: 3
 */ 

#include "stdtypes.h"
#include "DIO_INTERFACE.h"
#include "LCD_Config.h"
#include "LCD_Interface.h"
#include <avr/interrupt.h>
#include "ADC_Config.h"
#define F_CPU 16000000UL
#include <util/delay.h>

// Prototypes
void readSw(void);


//Global Variables
volatile u8 conversionCompleted = 0;
u16 num;
double analogVolatage = 0;
double temp = 0;
u8 modeSelect = 0;						// 0 means voltage mode (blue led)  1 means temperature mode (Green led).
u8 voltStr[10];
u8 tempStr[10];
u8 voltStrF = 0;
u8 tempStrF = 0;
u8 swPrevState = 1;
u8 swCurrState = 1;

int main(void)
{
	conversionCompleted = 0;
	modeSelect = 0;
	swCurrState = 1;  // switch is connected to pullup resistor so it is a negative logic switch and 1 means it is not pressed.
	swPrevState = 1;
	voltStrF = 0;
	tempStrF = 0;
	DDRA = 0x00;
	DDRC = 0x0E;		// PIN0--> I/P - PIN 1~3--> O/P
	LCD_VoidInitialize_8bit();
	sei();	
	ADC_Init();
	ADC_Select(CHANNEL0);
	while(1)
    {
		readSw();
        if(conversionCompleted)
		{
			conversionCompleted = 0;
			if(modeSelect == 1) // Voltage Mode
			{
				ADC_Select(CHANNEL2);
				num = (ADC_ADCH << 2);
				analogVolatage = ((double) num / 1020);
				analogVolatage = analogVolatage * 5;
				analogVolatage = analogVolatage * 1000;
				
				//TempLCD_voidNumToStr(analogVolatage, voltStr);
				// voltstrF = 0 means that the device was in temperature mode and is being switched to voltage mode
				// so we have to change the text on the LCD.
				
				if(voltStrF == 0)		
				{
					voltStrF = 1; // device now on voltage mode.
					tempStrF = 0;
					PORTC |= 0x02;
					PORTC &= ~0x04;
					LCD_VoidSEND_CMD8BIT(CLEAR_DISPLAY);
					LCD_VoidPrint8BIT("Voltage Mode");
					LCD_VoidSEND_CMD8BIT(FORCE_CURSOR_SECOND_LINE);
					VoltLCD_VoidNumToStr2(analogVolatage);
					LCD_VoidPrint8BIT(" Volt");
				}
				//voltstrF = 1 means that the device is already measuring voltage.
				// so we only need to change the value of the voltage.
				else
				{
					LCD_VoidSEND_CMD8BIT(FORCE_CURSOR_SECOND_LINE);
					VoltLCD_VoidNumToStr2(analogVolatage);
				}
				
			}
			else if(modeSelect == 0)  //Temperature Mode
			{
				ADC_Select(CHANNEL1);
				num = (ADC_ADCH << 2);
				temp = ((double) num / 1020);   //find percentage of input reading
				temp = temp * 5;                     
				temp = temp - 0.5;                   
				temp = temp * 100 + 50;  
				
				
				// tempstrF = 0 means that the device was in voltage mode and is being switched to temperature mode
				// so we have to change text on the LCD.
				
				if(tempStrF == 0)		
				{
					tempStrF = 1;		// temperature mode is on.
					voltStrF = 0;		// voltage mode is off.
					PORTC |= 0x04;
					PORTC &= ~0x02;
					LCD_VoidSEND_CMD8BIT(CLEAR_DISPLAY);
					LCD_VoidPrint8BIT("Temperature Mode");
					LCD_VoidSEND_CMD8BIT(FORCE_CURSOR_SECOND_LINE);
					TempLCD_voidNumToStr2(temp);
					LCD_VoidPrint8BIT(" C");
				}
				// tempstrF = 1 means the device is already is in temperature mode.
				// so we only change the value of the temperature.
				else					
				{
					LCD_VoidSEND_CMD8BIT(FORCE_CURSOR_SECOND_LINE);
					TempLCD_voidNumToStr2(temp);
				}
				if(temp > 60)
				{
					PORTC |= 0x08;
				}
				else
				{
					PORTC &= ~0x08;
				}
			}
		}
		_delay_ms(100);
	}
}

ISR(ADC_vect)
{
	conversionCompleted = 1;
}


void readSw(void)
{
	swCurrState = PINC & 0x01;
	if((swPrevState == 1) && (swCurrState == 0))
	{
		modeSelect ^= 0x01;
	}
	swPrevState = swCurrState;		
}