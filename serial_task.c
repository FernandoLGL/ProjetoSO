/*
 * serial_task.c
 *
 *  Created on: 26 de nov de 2019
 *      Author: renan
 */

//Includes
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
#include "switch_task.h"
#include "led_task.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//ENDIncludes

#include "driverlib/adc.h"

#define SERIALTASKSTACKSIZE        128

#define SERIAL_ITEM_SIZE           sizeof(uint8_t)
#define SERIAL_QUEUE_SIZE          5

extern xSemaphoreHandle g_pUARTSemaphore;

xQueueHandle queue2;

static void SerialTask(void *pvParameters)
{

    //portTickType ui32arm_ult_tempo_serial;
    //uint32_t ui32Serial_Delay = 200;
    uint32_t ui32Serial_Msg;
    uint32_t index_serial_Buffer = 0;
    uint32_t ui32TempValue[10];
    //ui32arm_ult_tempo_serial = xTaskGetTickCount();

    while(1)
    {
        if(xQueueReceive(queue2, &ui32Serial_Msg, 0) == pdPASS)
        {
            ui32TempValue[index_serial_Buffer]=ui32Serial_Msg;
            index_serial_Buffer++;

            if(index_serial_Buffer==10)
            {
                xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                UARTprintf("Task SERIAL acordou:\n" );
                UARTprintf("Task SERIAL enviou os seguintes valores: \n");
                int i = 0;
                while(i<=index_serial_Buffer)
                {
                    UARTprintf("%d ,\n",  ui32TempValue[i]);
                    i++;
                }
                xSemaphoreGive(g_pUARTSemaphore);
                index_serial_Buffer=0;
            }

        }
        //vTaskDelayUntil(&ui32arm_ult_tempo_serial, ui32Serial_Delay / portTICK_RATE_MS);
    }

}

uint32_t SerialTaskInit(void)
{
    queue2 = xQueueCreate(SERIAL_QUEUE_SIZE, SERIAL_ITEM_SIZE);

    ROM_FPULazyStackingEnable();
    //TempTask(xTaskGetTickCount);
    if(xTaskCreate(SerialTask, (const portCHAR *)"Serial", SERIALTASKSTACKSIZE, NULL, tskIDLE_PRIORITY + PRIORITY_LED_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    return(0);
}