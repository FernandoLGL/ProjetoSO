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

#include "temptask.h"
#include "flashtask.h"
#include "serialtask.h"

/*
void UARTIntHandler(void)
{
uint32_t ui32Status;
ui32Status = UARTIntStatus(UART0_BASE, true);
UARTIntClear(UART0_BASE, ui32Status);
while(UARTCharsAvail(UART0_BASE))
{
UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART0_BASE));
//echo character
GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
SysCtlDelay(SysCtlClockGet() / (1000 * 3));
GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
}
}
*/


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif


/*
void
ConfigureUART(void)
{

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);


    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);


    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);


    UARTStdioConfig(0, 115200, 16000000);
}
//INICIA TASK DA LEITURA DE TEMPERATURA
*/
/**************************************************************************/
//SETA O TAMANHO DA PILHA PARA A TASK ARMAZENAMENTO DE TEMPERATURAS
/**************************************************************************/
#define TEMPTASKSTACKSIZE        128         //TAMANHO DA PILHA EM PALAVRAS


/**************************************************************************/
//ENVIA DADOS AO SEMÁFORO E ÀS FILAS 1 E 2
/**************************************************************************/


extern xQueueHandle man_queue_flash;
extern xQueueHandle man_queue_serial;
extern xSemaphoreHandle g_pUARTSemaphore;

static void TempTask(void *pvParameters)
{
    man_queue_flash = xQueueCreate(20 , sizeof(int));
    man_queue_serial = xQueueCreate(20 , sizeof(int));

    portTickType ui16arm_ult_tempo;
    uint32_t ui32TempDelay = 200;
    int seq_arm_temp=0;
    uint32_t ui32ADC0Value[4];
    volatile uint32_t ui32TempAvg;
    uint32_t ui32TempValueC;
    volatile uint32_t ui32TempValueF;
    uint8_t ui8PinData = 2;
    ui16arm_ult_tempo = xTaskGetTickCount();
    seq_arm_temp=0;

    while(1)
    {
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
        ROM_ADCHardwareOversampleConfigure(ADC0_BASE, 64);
        ROM_ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
        ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
        ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
        ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
        ROM_ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
        ROM_ADCSequenceEnable(ADC0_BASE, 1);
        ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
        ROM_ADCIntClear(ADC0_BASE, 1);
        ROM_ADCProcessorTrigger(ADC0_BASE, 1);
        ROM_ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
        ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
        ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
        ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;

            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,ui8PinData);
            UARTprintf("*************************************************************\n");
            UARTprintf("\n task TEMPERATURA ACORDOU \n");
            //UARTprintf("\nTEMPERATURA EM GRAUS CELSIUS:\n");
            UARTprintf("\n= %3d*C\n" ,ui32TempValueC);
            xSemaphoreGive(g_pUARTSemaphore);

         if(xQueueSend(man_queue_flash, &ui32TempValueC, portMAX_DELAY) != pdPASS)
         {
             UARTprintf("\nQueue cheia. Isso nunca deveria acontecer.\n");
             while(1)
             {
             }
         }
         if(xQueueSend(man_queue_serial, &ui32TempValueC, portMAX_DELAY) != pdPASS)
         {
             UARTprintf("\nQueue cheia. Isso nunca deveria acontecer.\n");
             while(1)
             {
             }
         }
         uint8_t ui8PinData=0;
         //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,ui8PinData);
         vTaskDelayUntil(&ui16arm_ult_tempo, ui32TempDelay / portTICK_RATE_MS);
    }
}

//}




/***************************************************************************
                     INICIALIZA A TASK DE TEMPERATURA
****************************************************************************/


uint32_t Temp_Task_Ini(void)

{
    ROM_FPULazyStackingEnable();
//TempTask(xTaskGetTickCount);
    if(xTaskCreate(TempTask, (const portCHAR *)"Temperatura",TEMPTASKSTACKSIZE, NULL, tskIDLE_PRIORITY + PRIORITY_LED_TASK, NULL) != pdTRUE)
         {
             return(1);
         }

    return(0);
}





