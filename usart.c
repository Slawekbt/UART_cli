/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "ring_buffer.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_rcc.h"
#include "core.h"


/* Private definitions -------------------------------------------------------*/

/* Definition for USARTx clock resources */
#define USARTx                           USART3
#define USARTx_CLK_ENABLE()              __USART3_CLK_ENABLE();
#define USARTx_RX_GPIO_CLK_ENABLE()      __GPIOD_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __GPIOD_CLK_ENABLE() 

#define USARTx_FORCE_RESET()             __USART3_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __USART3_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_8
#define USARTx_TX_GPIO_PORT              GPIOD  
#define USARTx_TX_AF                     GPIO_AF7_USART3

#define USARTx_RX_PIN                    GPIO_PIN_9
#define USARTx_RX_GPIO_PORT              GPIOD 
#define USARTx_RX_AF                     GPIO_AF7_USART3

/* Definition for USARTx's NVIC */
#define USARTx_IRQn                      USART3_IRQn
#define USARTx_IRQHandler                USART3_IRQHandler

/* Private variables ---------------------------------------------------------*/


// UART transmit buffer descriptor
static RingBuffer USART_RingBuffer_Tx;
// UART transmit buffer memory pool
static char RingBufferData_Tx[1024];



// UART receive buffer descriptor
static RingBuffer USART_RingBuffer_Rx;
// UART receive buffer memory pool
static char RingBufferData_Rx[1024];

/// UART descriptor
static UART_HandleTypeDef UartHandle;

char static znak_tx;

bool USART_PutChar(char c){
		
	 bool result;
   CORE_EnterCriticalSection();
   
   result	= RingBuffer_PutChar(&USART_RingBuffer_Tx, c);
   CORE_ExitCriticalSection();
	
  __USART_ENABLE_IT(&UartHandle, USART_IT_TXE);

	
	return result;
}


size_t USART_WriteData(const void *data, size_t dataSize){
	size_t i = 0;
  
  while(i < dataSize)
  {
    
    USART_PutChar( ((char *)data)[i] );
    i++;

  }

	return i;
}


size_t USART_WriteString(const char *string){
	
  size_t length = strlen(string) - 1;
  USART_WriteData(string, length);
  
	return length;
}


bool USART_GetChar(char *c){
	
  bool result;
	
  CORE_EnterCriticalSection();
	
  result =  RingBuffer_GetChar(&USART_RingBuffer_Rx, c);	// returns char put from USART to Buffer during interrupt
	
  CORE_ExitCriticalSection();
	
  return result;
}


size_t USART_ReadData(char *data, size_t maxSize){
  
  size_t i = 0; 
 
  while( USART_GetChar(&data[i]) & (i < maxSize) ) i++;
//  for(i=0;i < maxSize; ){
//    if(USART_GetChar(&data[i]))
//      i++;
//    else
//      continue;
//  }
  
	return i;
}


// USART Interrupt Service Routine (ISR)
void USARTx_IRQHandler(void){


	if (__HAL_USART_GET_FLAG(&UartHandle, USART_FLAG_RXNE)) {
     
		if (__HAL_USART_GET_IT_SOURCE(&UartHandle, USART_IT_RXNE)) {
			// the RXNE interrupt is enabled
       
			    RingBuffer_PutChar(&USART_RingBuffer_Rx, USARTx->DR);

		
	}
}
	if (__HAL_USART_GET_FLAG(&UartHandle, USART_FLAG_TXE)) {
    
    
    if (__HAL_USART_GET_IT_SOURCE(&UartHandle, USART_IT_TXE)) {
	    // the TXE interrupt is enabled
          
      if (RingBuffer_GetChar(&USART_RingBuffer_Tx, &znak_tx)) {
          USARTx->DR = znak_tx;
      } 
      else {
        __USART_DISABLE_IT(&UartHandle, USART_IT_TXE);
      }
          
	  }
  }

}




/**
  * This function configures the hardware resources used in this example: 
  *  - Peripheral's clock enable
  *	 - Peripheral's GPIO Configuration  
  *  - NVIC configuration for UART interrupt request enable
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();
  /* Enable USART1 clock */
  USARTx_CLK_ENABLE(); 
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;
  
  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);
    
  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;
    
  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
    
  /*##-3- Configure the NVIC for UART ########################################*/
  /* NVIC for USART1 */
  HAL_NVIC_SetPriority(USARTx_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USARTx_IRQn);
}

/**
  * This function frees the hardware resources used in this example:
  * 	- Disable the Peripheral's clock
  *   - Revert GPIO and NVIC configuration to their default state  
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  /*##-1- Reset peripherals ##################################################*/
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks ################################*/
  /* Configure UART Tx as alternate function */
  HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
  /* Configure UART Rx as alternate function */
  HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);
  
  /*##-3- Disable the NVIC for UART ##########################################*/
  HAL_NVIC_DisableIRQ(USARTx_IRQn);
}

/**
  * This function initialize ring buffers, USART device and enable receive interrupt.  
  */
bool USART_Init(void){
	// initialize ring buffers
	RingBuffer_Init(&USART_RingBuffer_Tx, RingBufferData_Tx, sizeof(RingBufferData_Tx));
	RingBuffer_Init(&USART_RingBuffer_Rx, RingBufferData_Rx, sizeof(RingBufferData_Rx));
	
	// configure USART
  UartHandle.Instance          = USARTx;
  
  UartHandle.Init.BaudRate     = 115200;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
	HAL_UART_MspInit(&UartHandle);
	
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
		// printf("ERROR in USART_Init");
    return false;
  }
	/* Enable the USART Data Register not empty Interrupt */
  __USART_ENABLE_IT(&UartHandle, USART_IT_RXNE); 
	
	return true;
}


