#ifndef ESP01_H
#define ESP01_H

#include "stm32f4xx_hal.h"

/* Function prototypes */
void ESP_Init(UART_HandleTypeDef *huart, const char *ssid, const char *password);
void ESP_SendData(UART_HandleTypeDef *huart, const char *api_key, int AIRQuality, int Noise_Level);

#endif // ESP01_H
