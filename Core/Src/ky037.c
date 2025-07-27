#include "ky037.h"
#include "math.h"

#define REFERENCE_VOLTAGE 5     // Reference voltage for the ADC
#define ADC_RESOLUTION 4095.0     // ADC resolution (12-bit ADC, max value 4095)
#define REFERENCE_VALUE 1.0       // Reference value (can be calibrated for your setup)
#define CALIBRATION_OFFSET 0.0    // Calibration offset (set based on calibration)
#define NUM_SAMPLES 100            // Number of samples for averaging

ADC_HandleTypeDef* hadc_global_ky037;

void KY037_Init(ADC_HandleTypeDef* hadc) {
    hadc_global_ky037 = hadc;
}

float KY037_ReadDecibels(void) {
    uint32_t adc_sum = 0;

    // Take NUM_SAMPLES ADC readings and calculate their sum
    for (int i = 0; i < NUM_SAMPLES; i++) {
        HAL_ADC_Start(hadc_global_ky037);
        HAL_ADC_PollForConversion(hadc_global_ky037, HAL_MAX_DELAY);
        uint32_t adc_value = HAL_ADC_GetValue(hadc_global_ky037);
        adc_sum += adc_value;
    }

    // Calculate the average ADC value
    uint32_t adc_avg_value = adc_sum / NUM_SAMPLES;

    // Convert ADC value to voltage
    float voltage = (adc_avg_value / ADC_RESOLUTION) * REFERENCE_VOLTAGE;

    // Calculate decibels using the formula
    float decibels = (20 * log10(voltage / REFERENCE_VALUE)) - CALIBRATION_OFFSET;

    return decibels;
}
