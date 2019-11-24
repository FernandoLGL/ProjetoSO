/*
 * Serial_Task.c
 *
 */



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
#define SERIALTASKSTACKSIZE        128         //TAMANHO DA PILHA EM PALAVRAS

//*****************************************************************************
//
// DEFINE TAMANHO DOS ITENS DA TASK FLASH E DA FILA ASSOCIADA A ELA
//
//*****************************************************************************
#define SERIAL_ITEM_SIZE           sizeof(uint8_t)
#define SERIAL_QUEUE_SIZE          5

//*****************************************************************************



/**************************************************************************************/
//INSTANCIA O SEMÁFORO E A FILA QUE ARMAZENA OS DADOS ENVIADOS DA TASK DE TEMPERATURA
/*************************************************************************************/
xQueueHandle man_queue_serial;
extern xSemaphoreHandle g_pUARTSemaphore;

/***************************************************************************
 	 	 	 	 	 DEFINE A TASK DE ARMAZENAMENTO NA FLASH
****************************************************************************/

static void SerialTask(void *pvParameters)
{
    portTickType ui32arm_ult_tempo_serial;
    uint32_t ui32SerialDelay = 200;
    uint32_t ui32Mensagem_Serial;
    uint8_t indice_serial_Buffer = 0;
    uint32_t ui32TempLida[20];
    ui32arm_ult_tempo_serial = xTaskGetTickCount();
    uint8_t ui8PinData=8;
    while(1)
    {

    	//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,ui8PinData);
    	xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    	//UARTprintf("Task SERIAL acordou:\n" );
    	xSemaphoreGive(g_pUARTSemaphore);
		if(xQueueReceive(man_queue_serial, &ui32Mensagem_Serial, 0) == pdPASS)
    	{
    		ui32TempLida[indice_serial_Buffer]=ui32Mensagem_Serial;
    		indice_serial_Buffer++;
    		if(indice_serial_Buffer==19)
    		{
    			xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    			UARTprintf("Task SERIAL enviou os seguintes valores:\n" );
                UARTprintf("Valor %d \n",  ui32TempLida[0]);
                UARTprintf("Valor %d \n",  ui32TempLida[1]);
                UARTprintf("Valor %d \n",  ui32TempLida[2]);
                UARTprintf("Valor %d \n",  ui32TempLida[3]);
                UARTprintf("Valor %d \n",  ui32TempLida[4]);
                UARTprintf("Valor %d \n",  ui32TempLida[5]);
                UARTprintf("Valor %d \n",  ui32TempLida[6]);
                UARTprintf("Valor %d \n",  ui32TempLida[7]);
                UARTprintf("Valor %d \n",  ui32TempLida[8]);
                UARTprintf("Valor %d \n",  ui32TempLida[9]);
                UARTprintf("Valor %d \n",  ui32TempLida[10]);
                UARTprintf("Valor %d \n",  ui32TempLida[11]);
                UARTprintf("Valor %d \n",  ui32TempLida[12]);
                UARTprintf("Valor %d \n",  ui32TempLida[13]);
                UARTprintf("Valor %d \n",  ui32TempLida[14]);
                UARTprintf("Valor %d \n",  ui32TempLida[15]);
                UARTprintf("Valor %d \n",  ui32TempLida[16]);
                UARTprintf("Valor %d \n",  ui32TempLida[17]);
                UARTprintf("Valor %d \n",  ui32TempLida[18]);
                UARTprintf("Valor %d \n",  ui32TempLida[19]);
                xSemaphoreGive(g_pUARTSemaphore);
                indice_serial_Buffer=0;
    		 }

    	  }
    	  ui8PinData=0;
    	  //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,ui8PinData);
    	  //vTaskDelayUntil(&ui32arm_ult_tempo_serial, ui32SerialDelay / portTICK_RATE_MS);
    	}

}
//}





/***************************************************************************
 	 	 	 	 	 INICIALIZA A TASK DE GRAVAÇÃO NA FLASH
****************************************************************************/


uint32_t Serial_Task_Ini(void)

{
/***************************************************************************
	 	 	 	 	 	 CRIA A FILA DA TASK DE GRAVAÇÃO NA FLASH
****************************************************************************/

	man_queue_serial = xQueueCreate(SERIAL_QUEUE_SIZE, SERIAL_ITEM_SIZE);


	ROM_FPULazyStackingEnable();
//TempTask(xTaskGetTickCount);
    if(xTaskCreate(SerialTask, (const portCHAR *)"Serial",SERIALTASKSTACKSIZE, NULL, tskIDLE_PRIORITY + PRIORITY_LED_TASK, NULL) != pdTRUE)
         {
             return(1);
         }

    return(0);
}










