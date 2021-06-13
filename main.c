#include "stm32f4xx_hal.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

#define BACKSPACE 0x7F
#define ENTER 0x0D
#define LINE_FEED 0x0A
#define FORM_FEED 0x0C



char c;
static int timeMs = 0;

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct_1, GPIO_InitStruct_2;
	__GPIOB_CLK_ENABLE();
	
	// conf LED1
	GPIO_InitStruct_1.Pin = GPIO_PIN_0;
	GPIO_InitStruct_1.Mode = GPIO_MODE_OUTPUT_PP; // push-pull output
	GPIO_InitStruct_1.Pull = GPIO_NOPULL;					// no pull-up/down
	GPIO_InitStruct_1.Speed = GPIO_SPEED_LOW;			// analog pin bandwidth limited
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_1);
	
		// conf LED2
	GPIO_InitStruct_2.Pin = GPIO_PIN_7;
	GPIO_InitStruct_2.Mode = GPIO_MODE_OUTPUT_PP; // push-pull output
	GPIO_InitStruct_2.Pull = GPIO_NOPULL;					// no pull-up/down
	GPIO_InitStruct_2.Speed = GPIO_SPEED_LOW;			// analog pin bandwidth limited
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_2);
	
	
} /* LED_Init */

void delay(int delayMs)
{
	
	while(1)
	{
		if(timeMs > delayMs)
		{
			timeMs = 0;
			break;
		}
	}
	
} /* delay */

void SysTick_Handler(void)
{
	timeMs++;
}

/* To move to separate files  */

static char commandBuffer[256];

int USART_Echo(char* c)
{
	static int i;
	int length;
	
	if( i == 0 && *c == BACKSPACE )
	{
		// do nothing
	}
	
	else if( *c == BACKSPACE )
	{
		i--;
		USART_PutChar('\b');
		commandBuffer[i] = 0;
		USART_PutChar(commandBuffer[i]);
				
	}
	
	else if( *c == ENTER )
	{
		commandBuffer[i] = '\0';	// will be treated as EOL sign
		
		length = i;
		i = 0;
		
		return length;
	}
	
	else
	{
		commandBuffer[i] = *c;
		USART_PutChar(commandBuffer[i]);
		i++;
	}
	
	return 0;
}

void decryptCommand(char* input_command, int input_len)
{
	char debug[100];
	
	const char* led1_on = "led1 on";
	const char* led1_off = "led1 off";
	const char* led2_on = "led2 on";
	const char* led2_off = "led2 off";
	const char* help = "help";
	const char* clear = "clear";
	
	if( strcmp(led1_on, input_command) == 0)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		USART_WriteString(" Lighting on LED 1 \n\r\r");
	}
	
	else if( strcmp(led1_off, input_command) == 0)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		USART_WriteString(" Lighting off LED 1 \n\r\r");
		
	}
	
	else if( strcmp(led2_on, input_command) == 0)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
		USART_WriteString(" Lighting on LED 2 \n\r\r");
	}
	
	else if( strcmp(led2_off, input_command) == 0)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
		USART_WriteString(" Lighting off LED 2 \n\r\r");
		
	}
	
	else if( strcmp(clear, input_command) == 0)
	{
		USART_PutChar(FORM_FEED);
		USART_WriteString(">>");
		
	}
	
	else if( strcmp(help, input_command) == 0)
	{
		USART_WriteString(" Available commands  \n\r\r");
		USART_WriteString(" led1 on \n\r\r");
		USART_WriteString(" led1 off \n\r\r");
		USART_WriteString(" led2 on \n\r\r");
		USART_WriteString(" led2 off \n\r\r");
		USART_WriteString(" clear \n\r\r");
		USART_WriteString(" help \n\r\r");
		USART_WriteString(" \n\r\r");
	}

	
	else
	{
		USART_WriteString(" Command not recognised -- Try typing help \n\r\r");

	}
	
		sprintf(debug, "Input command length :: %i \n\r\r", input_len);
		USART_WriteString(debug);
	
}



int main(void)
{
	LED_Init();
	HAL_Init();	// 115200
	USART_Init();
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
	
	
	int command_length = 0;
	USART_WriteString("Ello there mate ;) \n\r\r >> ");
	
	while(1)
	{
		if(USART_GetChar(&c))
		{
			// USART_PutChar(c);
			command_length =  USART_Echo(&c);
			if( command_length > 0 )	// 		if ENTER was hit
			{
				USART_WriteString("\n\r\r");
				USART_WriteData(&commandBuffer, command_length);
				
				decryptCommand((char*)(&commandBuffer), command_length);
				
				USART_WriteString("\n\r\r>> ");
			}
			

		}
	}
	
}
