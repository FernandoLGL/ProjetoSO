/*
 * temperatura_task.c
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
#include "drivers/rgb.h"
#include "switch_task.h"
#include "led_task.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "driverlib/adc.h"
//ENDIncludes

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

#define TEMPTASKSTACKSIZE        128

extern xQueueHandle queue1; //Grava Task
extern xQueueHandle queue2; // Serial Task
extern xSemaphoreHandle g_pUARTSemaphore;

static uint32_t g_pui32Colors[3];

TaskHandle_t TemperaturaTask_handler;

static void TemperaturaTask(void *pvParameters)
{

    uint32_t ui32TempDelay = 1000;
    uint32_t ui32ADC0Value[4];
    uint32_t ui32TempValueC;
    volatile uint32_t ui32TempAvg;
    portTickType ui16arm_ult_tempo;

    ui16arm_ult_tempo = xTaskGetTickCount();

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ROM_ADCHardwareOversampleConfigure(ADC0_BASE, 64);

    ROM_ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
    ROM_ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
    ROM_ADCSequenceEnable(ADC0_BASE, 1);

    while(1)
    {

        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("task TEMPERATURA ACORDOU \n");
        xSemaphoreGive(g_pUARTSemaphore);

        g_pui32Colors[RED] = 0x8000;
        RGBColorSet(g_pui32Colors);

        ROM_ADCIntClear(ADC0_BASE, 1);
        ROM_ADCProcessorTrigger(ADC0_BASE, 1);

        while(!ADCIntStatus(ADC0_BASE, 1, false))   { }

        ROM_ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);

        ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
        ui32TempValueC = (1475 - (2475 * ui32TempAvg) / 4096)/10;

        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf(" %3d*C\n" ,ui32TempValueC);
        xSemaphoreGive(g_pUARTSemaphore);
        if(xQueueSend(queue1, &ui32TempValueC, portMAX_DELAY) != pdPASS)
        {
            UARTprintf("\nQueue full. This should never happen.\n");
            while(1)
            {
            }
        }
        if(xQueueSend(queue2, &ui32TempValueC, portMAX_DELAY) != pdPASS)
        {
            UARTprintf("\nQueue full. This should never happen.\n");
            while(1)
            {
            }
        }
        vTaskDelayUntil(&ui16arm_ult_tempo, ui32TempDelay / portTICK_RATE_MS);
    }
}

uint32_t TemperaturaTaskInit(void)
{
    if(xTaskCreate(TemperaturaTask, (const portCHAR *)"Temperatura",TEMPTASKSTACKSIZE, NULL, tskIDLE_PRIORITY + PRIORITY_TEMPERATURA_TASK, &TemperaturaTask_handler) != pdTRUE)
    {
        return(1);
    }

    return(0);
}
