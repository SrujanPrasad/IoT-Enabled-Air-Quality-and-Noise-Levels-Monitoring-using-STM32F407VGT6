#include "esp01.h"
#include <stdio.h>
#include <string.h>

/* Buffer for AT command responses */
static char esp_response[200];

/* Function to initialize the ESP-01 module */
void ESP_Init(UART_HandleTypeDef *huart, const char *ssid, const char *password) {
    char command[100];

    // Reset the module
    HAL_UART_Transmit(huart, (uint8_t *)"AT+RST\r\n", strlen("AT+RST\r\n"), HAL_MAX_DELAY);
    HAL_Delay(2000);

    // Set Wi-Fi mode to Station
    HAL_UART_Transmit(huart, (uint8_t *)"AT+CWMODE=1\r\n", strlen("AT+CWMODE=1\r\n"), HAL_MAX_DELAY);
    HAL_Delay(1000);

    // Connect to Wi-Fi
    snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);
    HAL_UART_Transmit(huart, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);
    HAL_Delay(5000); // Allow time to connect
}

/* Function to send data to ThingSpeak */
void ESP_SendData(UART_HandleTypeDef *huart, const char *api_key, int aqi, int noise_level) {
    char command[100];
    char http_request[200];

    // Start TCP connection to ThingSpeak
    snprintf(command, sizeof(command), "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");
    HAL_UART_Transmit(huart, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);
    HAL_Delay(2000);

    // Prepare HTTP GET request
    snprintf(http_request, sizeof(http_request),
             "GET /update?api_key=%s&field1=%d&field2=%d\r\n",
             api_key, aqi, noise_level);

    // Send HTTP GET request length
    snprintf(command, sizeof(command), "AT+CIPSEND=%d\r\n", (int)strlen(http_request) + 2);
    HAL_UART_Transmit(huart, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);
    HAL_Delay(1000);

    // Send the HTTP GET request
    HAL_UART_Transmit(huart, (uint8_t *)http_request, strlen(http_request), HAL_MAX_DELAY);
    HAL_Delay(1000);

    // Close the TCP connection
    HAL_UART_Transmit(huart, (uint8_t *)"AT+CIPCLOSE\r\n", strlen("AT+CIPCLOSE\r\n"), HAL_MAX_DELAY);
    HAL_Delay(500);
}
