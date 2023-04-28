/***************************************************************************//**
 * @file
 * @brief FreeRTOS Blink Demo for Energy Micro EFM32GG_STK3700 Starter Kit
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"

#include "em_chip.h"
#include "bsp.h"
#include "bsp_trace.h"

#include "sleep.h"
#include "I2C.h"

#define STACK_SIZE_FOR_TASK    (configMINIMAL_STACK_SIZE + 10)
#define TASK_PRIORITY          (tskIDLE_PRIORITY + 1)

//Adre�a APSD-9960 sparkfun --> 0x39
//https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor

/* Structure with parameters for LedBlink */
typedef struct {
  /* Delay between blink of led */
  portTickType delay;
  /* Number of led */
  int          ledNo;
} TaskParams_t;

/***************************************************************************//**
 * @brief Simple task which is blinking led
 * @param *pParameters pointer to parameters passed to the function
 ******************************************************************************/

static void LedBlink(void *pParameters)
{
  TaskParams_t     * pData = (TaskParams_t*) pParameters;
  const portTickType delay = pData->delay;
  printf("Hola\n");
  //bool status = false;
  //status = I2C_Test();
  //printf("Estate: %d\n", status);
  for (;; ) {
    BSP_LedToggle(pData->ledNo);
    vTaskDelay(delay);
  }
}

int _write(int file, const char *ptr, int len) {
    int x;
    for (x = 0; x < len; x++) {
       ITM_SendChar (*ptr++);
    }
    return (len);
}

void printStatus(void *pParameters)
{
	while(1) {
	  bool status = false;
	  status = I2C_Test();
	  printf("State: %d\n", status);
	  vTaskDelay(500);

	}
}

void detectarColor(void *pParameters)
{
	uint8_t data = 3;

	I2C_WriteRegister(0x80, data);
	uint16_t infoRed, infoGreen, infoBlue;
	uint8_t redLow, redHigh, greenLow, greenHigh, blueLow, blueHigh;
	I2C_ReadRegister(0x96, &redLow);
	I2C_ReadRegister(0x97, &redHigh);

	I2C_ReadRegister(0x98, &greenLow);
	I2C_ReadRegister(0x99, &greenHigh);

	I2C_ReadRegister(0x9A, &blueLow);
	I2C_ReadRegister(0x9B, &blueHigh);

	infoRed = ((uint16_t)redHigh << 8) | redLow;
	infoGreen = ((uint16_t)greenHigh << 8) | greenLow;
	infoBlue = ((uint16_t)blueHigh << 8) | blueLow;



	printf("I2C: %02X\n", data);



}
/***************************************************************************//**
 * @brief  Main function
 ******************************************************************************/

int main(void)
{
  /* Chip errata */
  CHIP_Init();
  /* If first word of user data page is non-zero, enable Energy Profiler trace */
  BSP_TraceProfilerSetup();

  BSP_I2C_Init(0x39 << 1);
  /* Initialize LED driver */
  BSP_LedsInit();
  /* Setting state of leds*/
  BSP_LedSet(0);
  BSP_LedSet(1);

  /* Initialize SLEEP driver, no calbacks are used */
  SLEEP_Init(NULL, NULL);
#if (configSLEEP_MODE < 3)
  /* do not let to sleep deeper than define */
  SLEEP_SleepBlockBegin((SLEEP_EnergyMode_t)(configSLEEP_MODE + 1));
#endif

  /* Parameters value for taks*/
  static TaskParams_t parametersToTask1 = { pdMS_TO_TICKS(1000), 0 };
  static TaskParams_t parametersToTask2 = { pdMS_TO_TICKS(500), 1 };

  /*Create two task for blinking leds*/
  xTaskCreate(LedBlink, (const char *) "LedBlink1", STACK_SIZE_FOR_TASK, &parametersToTask1, TASK_PRIORITY, NULL);
  xTaskCreate(LedBlink, (const char *) "LedBlink2", STACK_SIZE_FOR_TASK, &parametersToTask2, TASK_PRIORITY, NULL);
  xTaskCreate(printStatus, (const char *) "printSatus", STACK_SIZE_FOR_TASK, NULL, TASK_PRIORITY, NULL);

  /*Start FreeRTOS Scheduler*/

  vTaskStartScheduler();

  return 0;
}
