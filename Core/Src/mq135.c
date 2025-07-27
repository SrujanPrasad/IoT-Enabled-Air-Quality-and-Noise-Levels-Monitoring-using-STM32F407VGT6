#include "mq135.h"
#include "math.h"

#define RL_VALUE 10.0  // Load resistance in kilo ohms
#define R0 10.0        // Base resistance for calibration

// Constants for different gases (based on the MQ135 datasheet or calibration)
#define PPM_CO2_CONST 116.6020682
#define PPM_SMOKE_CONST 2.3
#define PPM_NOX_CONST 1.5
#define PPM_O3_CONST 0.5

// Exponent values for each gas (based on the MQ135 characteristics)
#define CO2_EXPONENT -2.769034857
#define SMOKE_EXPONENT -1.497
#define NOX_EXPONENT -0.8
#define O3_EXPONENT -1.15

// Sampling and filtering
#define NUM_SAMPLES 500  // Number of samples for averaging
#define ALPHA 0.2       // Smoothing factor (0 < ALPHA <= 1)

// Global Variables
ADC_HandleTypeDef* hadc_global;
float smooth_value = 0;  // For exponential smoothing

// Breakpoints for AQI calculation (example values, adjust as needed)
const float co2_breakpoints[] = {0, 400, 1000, 2000};
const float smoke_breakpoints[] = {0, 50, 150, 300};
const float nox_breakpoints[] = {0, 0.1, 0.2, 1.0};
const float o3_breakpoints[] = {0, 0.05, 0.1, 0.2};

// Function to initialize MQ135 sensor
void MQ135_Init(ADC_HandleTypeDef* hadc) {
    hadc_global = hadc;
}

// Function to calculate resistance
float calculateResistance(float adc_value) {
    float voltage = (adc_value / 4095.0) * 3.3;
    return ((3.3 - voltage) / voltage) * RL_VALUE;
}

// Function for exponential smoothing
float applyExponentialSmoothing(float new_value) {
    smooth_value = (ALPHA * new_value) + ((1 - ALPHA) * smooth_value);
    return smooth_value;
}

// Function to calculate PPM for a specific gas
float calculatePPM(float resistance, float ppm_const, float exponent) {
    return ppm_const * pow((resistance / R0), exponent);
}

// Function to calculate AQI subindex (linear interpolation)
float calculateAQISubIndex(float ppm, const float* breakpoints, int num_breakpoints) {
    for (int i = 0; i < num_breakpoints - 1; i++) {
        if (ppm >= breakpoints[i] && ppm <= breakpoints[i + 1]) {
            return ((ppm - breakpoints[i]) / (breakpoints[i + 1] - breakpoints[i])) *
                   (i + 1) * 50;
        }
    }
    return 0;
}

// Function to assume standard temperature and humidity
void getStandardTempHumidity(float* temperature, float* humidity) {
    *temperature = 25.0; // Assume 25°C as standard temperature
    *humidity = 50.0;    // Assume 50% humidity as standard value
}

// Function to compensate for temperature and humidity (using assumed values)
float compensateForTempHumidity(float ppm, float temperature, float humidity) {
    // Adjust the PPM value based on temperature and humidity.
    // Example formula (you can adjust based on calibration data):
    float tempFactor = 1 + (temperature - 25) * 0.02; // Assume 2% change per degree Celsius
    float humidityFactor = 1 + (humidity - 50) * 0.01; // Assume 1% change per 1% humidity
    return ppm * tempFactor * humidityFactor;
}

// Function to read PPM and calculate AQI
float MQ135_ReadPPM(void) {
    uint32_t adc_sum = 0;
    float temperature = 0, humidity = 0;

    // Collect multiple ADC samples
    for (int i = 0; i < NUM_SAMPLES; i++) {
        HAL_ADC_Start(hadc_global);
        HAL_ADC_PollForConversion(hadc_global, HAL_MAX_DELAY);
        adc_sum += HAL_ADC_GetValue(hadc_global);
    }

    // Calculate average ADC value
    uint32_t adc_value = adc_sum / NUM_SAMPLES;

    // Calculate resistance and smooth the value
    float resistance = calculateResistance(adc_value);
    resistance = applyExponentialSmoothing(resistance);

    // Calculate PPM values
    float ppm_co2 = calculatePPM(resistance, PPM_CO2_CONST, CO2_EXPONENT);
    float ppm_smoke = calculatePPM(resistance, PPM_SMOKE_CONST, SMOKE_EXPONENT);
    float ppm_nox = calculatePPM(resistance, PPM_NOX_CONST, NOX_EXPONENT);
    float ppm_o3 = calculatePPM(resistance, PPM_O3_CONST, O3_EXPONENT);

    // Get standard temperature and humidity (since no sensor is available)
    getStandardTempHumidity(&temperature, &humidity);

    // Compensate PPM values for temperature and humidity
    ppm_co2 = compensateForTempHumidity(ppm_co2, temperature, humidity);
    ppm_smoke = compensateForTempHumidity(ppm_smoke, temperature, humidity);
    ppm_nox = compensateForTempHumidity(ppm_nox, temperature, humidity);
    ppm_o3 = compensateForTempHumidity(ppm_o3, temperature, humidity);

    // Calculate AQI based on the highest subindex
    float overall_aqi = fmax(fmax(calculateAQISubIndex(ppm_co2, co2_breakpoints, 4),
                                  calculateAQISubIndex(ppm_smoke, smoke_breakpoints, 4)),
                             fmax(calculateAQISubIndex(ppm_nox, nox_breakpoints, 4),
                                  calculateAQISubIndex(ppm_o3, o3_breakpoints, 4)));

    return overall_aqi;
}
