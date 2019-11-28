/*
 * grava_task.c
 *
 *  Created on: 27 de nov de 2019
 *      Author: renan
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "drivers/buttons.h"
#include "utils/uartstdio.h"
#include "drivers/rgb.h"
#include "switch_task.h"
#include "led_task.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "driverlib/eeprom.h"

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

#define GRAVATASKSTACKSIZE        128         //TAMANHO DA PILHA EM PALAVRAS
#define GRAVA_ITEM_SIZE           sizeof(uint32_t)
#define GRAVA_QUEUE_SIZE          10

xQueueHandle queue1;
TaskHandle_t GravaTask_handler;

extern xSemaphoreHandle g_pUARTSemaphore;

static uint32_t g_pui32Colors[3];

static void GravaTask(void *pvParameters)
{
//    uint32_t ui32GravaDelay = 200;
    uint32_t ui32Mensagem_Grava;
    uint8_t Grava_Buffer_Index = 0;
    uint32_t ui32Grava_Buffer[10];

//    ui32arm_ult_tempo_grava = xTaskGetTickCount();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    EEPROMInit();

    while(1)
    {
        if(xQueueReceive(queue1, &ui32Mensagem_Grava, 0) == pdPASS)
        {
            ui32Grava_Buffer[Grava_Buffer_Index]=ui32Mensagem_Grava;
            Grava_Buffer_Index++;

            if(Grava_Buffer_Index==10)
            {
                uint32_t pui32Read[10];

                xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                UARTprintf("task GRAVA gravou dado \n");
                xSemaphoreGive(g_pUARTSemaphore);

                g_pui32Colors[GREEN] = 0x8000;
                RGBColorSet(g_pui32Colors);
                EEPROMMassErase();
                EEPROMProgram(ui32Grava_Buffer, 0x0, sizeof(ui32Grava_Buffer));
                EEPROMRead(pui32Read, 0x0, sizeof(pui32Read));

//                xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
//                UARTprintf("dado %d \n", pui32Read[0]);
//                UARTprintf("dado %d \n", pui32Read[1]);
//                UARTprintf("dado %d \n", pui32Read[2]);
//                UARTprintf("dado %d \n", pui32Read[3]);
//                UARTprintf("dado %d \n", pui32Read[4]);
//                UARTprintf("dado %d \n", pui32Read[5]);
//                UARTprintf("dado %d \n", pui32Read[6]);
//                UARTprintf("dado %d \n", pui32Read[7]);
//                UARTprintf("dado %d \n", pui32Read[8]);
//                UARTprintf("dado %d \n", pui32Read[9]);
//                xSemaphoreGive(g_pUARTSemaphore);

                Grava_Buffer_Index=0;
            }

        }
        //uint8_t ui8PinData=0;
        //vTaskDelayUntil(&ui32arm_ult_tempo_grava, ui32GravaDelay / portTICK_RATE_MS);
    }
}

uint32_t GravaTaskInit(void)

{
    queue1 = xQueueCreate(GRAVA_QUEUE_SIZE, GRAVA_ITEM_SIZE);

    if(xTaskCreate(GravaTask, (const portCHAR *)"Grava",GRAVATASKSTACKSIZE, NULL, tskIDLE_PRIORITY + PRIORITY_GRAVA_TASK, &GravaTask_handler) != pdTRUE)
    {
        return(1);
    }

    return(0);
}


