/*
 * Task_Flash.c
 *
 */


/*
 * main.c
 */
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_hibernate.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/hibernate.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/uartstdio.h"
#include "utils/cmdline.h"
/*#include <rgb.h>
#include <buttons.h>
#include <rgb_commands.h>
#include <qs-rgb.h>*/
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include <stdarg.h>
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"
#include "driverlib/adc.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "priorities.h"
#include "flashtask.h"
#include "temptask.h"

#include "serialtask.h"


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//INICIA TASK DA GRAVAÇÃO DE VALORES NA FLASH

/**************************************************************************/
//SETA O TAMANHO DA PILHA PARA A TASK ARMAZENAMENTO DE TEMPERATURAS
/**************************************************************************/
#define FLASHTASKSTACKSIZE        128         //TAMANHO DA PILHA EM PALAVRAS

//*****************************************************************************
//
// DEFINE TAMANHO DOS ITENS DA TASK FLASH E DA FILA ASSOCIADA A ELA
//
//*****************************************************************************
#define FLASH_ITEM_SIZE           sizeof(uint8_t)
#define FLASH_QUEUE_SIZE          5

//*****************************************************************************



/**************************************************************************************/
//INSTANCIA O SEMÁFORO E A FILA QUE ARMAZENA OS DADOS ENVIADOS DA TASK DE TEMPERATURA
/*************************************************************************************/

xQueueHandle man_queue_flash;
//extern xQueueHandle man_queue_flash;
extern xSemaphoreHandle g_pUARTSemaphore;
//extern xQueueHandle man_queue_serial;
//extern uint32_t ui32TempValueC;


/***************************************************************************
 	 	 	 	 	 DEFINE A TASK DE ARMAZENAMENTO NA FLASH
****************************************************************************/

static void FlashTask(void *pvParameters)
{
    portTickType ui32arm_ult_tempo_flash;
    uint32_t ui32FlashDelay = 200;
   // uint8_t ui8CurButtonState, ui8PrevButtonState;
    uint32_t ui32Mensagem_Flash;
//   uint8_t ui8PinData;
    uint8_t indice_Flash_Buffer = 0;
    uint32_t ui32TempLida[20];
    //
    // Get the current tick count.
    //
    ui32arm_ult_tempo_flash = xTaskGetTickCount();
    uint8_t ui8PinData=8;
    //
    // Loop forever.
    //

    while(1)
   {
    	//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,ui8PinData);
    	xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    	//UARTprintf("Task FLASH acordou:\n" );
    	xSemaphoreGive(g_pUARTSemaphore);
    	if(xQueueReceive(man_queue_flash, &ui32Mensagem_Flash, 0) == pdPASS)
    	{
    		ui32TempLida[indice_Flash_Buffer]=ui32Mensagem_Flash;
    		indice_Flash_Buffer++;

    		if(indice_Flash_Buffer==19)
    		{
    			xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    			UARTprintf("FLASH" );
                UARTprintf("\n= %4d\n" ,indice_Flash_Buffer);
                xSemaphoreGive(g_pUARTSemaphore);
                indice_Flash_Buffer=0;
    		 }

    	}
        uint8_t ui8PinData=0;
    	//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,ui8PinData);
    	//vTaskDelayUntil(&ui32arm_ult_tempo_flash, ui32FlashDelay / portTICK_RATE_MS);
    }
}
/***************************************************************************
 	 	 	 	 	 INICIALIZA A TASK DE GRAVAÇÃO NA FLASH
****************************************************************************/


uint32_t Flash_Task_Ini(void)

{
/***************************************************************************
	 	 	 	 	 	 CRIA A FILA DA TASK DE GRAVAÇÃO NA FLASH
****************************************************************************/

	man_queue_flash = xQueueCreate(FLASH_QUEUE_SIZE, FLASH_ITEM_SIZE);


	ROM_FPULazyStackingEnable();
	if(xTaskCreate(FlashTask, (const portCHAR *)"Flash",FLASHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY + PRIORITY_FLASH_TASK, NULL) != pdTRUE)
         {
             return(1);
         }

    return(0);
}









