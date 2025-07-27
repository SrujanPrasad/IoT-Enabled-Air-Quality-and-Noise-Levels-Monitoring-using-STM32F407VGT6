#include <ESP8266WiFi.h>
#include <math.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128  // OLED display width
#define SCREEN_HEIGHT 64  // OLED display height
#define OLED_RESET    -1  // Reset pin (not used)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// WiFi credentials
const char* ssid = "srujan";
const char* password = "srujan07";

// ThingSpeak API
const char* api_key = "2CM15K1Y3Y2TN0X8";
const char* server = "api.thingspeak.com";

// Pin assignments
#define MQ135_PIN A0          // Analog input for MQ135
#define KY037_PIN  5       // Digital pin for KY037
#define MQ135_POWER_PIN 4    // Digital pin to power MQ135

// Constants for the MQ135 Sensor
#define RL_VALUE 10.0  // Load resistance in kilo ohms
#define R0 10.0        // Base resistance for calibration
#define PPM_CO2_CONST 116.6020682
#define PPM_SMOKE_CONST 2.3
#define PPM_NOX_CONST 1.5
#define PPM_O3_CONST 0.5

// Exponent values for each gas (based on the MQ135 characteristics)
#define CO2_EXPONENT -2.769034857
#define SMOKE_EXPONENT -1.497
#define NOX_EXPONENT -0.8
#define O3_EXPONENT -1.15

// Constants for KY037 Sound Sensor
#define REFERENCE_VOLTAGE 5     // Reference voltage for the ADC
#define ADC_RESOLUTION 1024.0   // ADC resolution (10-bit ADC, max value 1023)
#define REFERENCE_VALUE 1.0     // Reference value (can be calibrated for your setup)
#define CALIBRATION_OFFSET 0.0  // Calibration offset

// Variables for smoothing
float smooth_value_mq135 = 0;  // For exponential smoothing for MQ135
float smooth_value_ky037 = 0;  // For exponential smoothing for KY037

// Constants
#define NUM_SAMPLES 500  // Number of samples for averaging
#define ALPHA 0.2        // Smoothing factor (0 < ALPHA <= 1)


// Breakpoints for AQI calculation (example values, adjust as needed)
const float co2_breakpoints[] = {0, 400, 1000, 2000};  // Example CO2 breakpoints
const float smoke_breakpoints[] = {0, 50, 150, 300};   // Example Smoke breakpoints
const float nox_breakpoints[] = {0, 0.1, 0.2, 1.0};    // Example NOx breakpoints
const float o3_breakpoints[] = {0, 0.05, 0.1, 0.2};    // Example O3 breakpoints


// Function to calculate resistance for MQ135
float calculateResistance(float adc_value) {
    float voltage = (adc_value / 4095.0) * 3.3;  // Convert ADC to voltage
    return ((3.3 - voltage) / voltage) * RL_VALUE;  // Calculate the resistance
}

// Function for exponential smoothing
float applyExponentialSmoothing(float new_value, float *smooth_value) {
    *smooth_value = (ALPHA * new_value) + ((1 - ALPHA) * (*smooth_value));
    return *smooth_value;
}

// Function to calculate PPM for a specific gas (MQ135)
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

// Function to read PPM from MQ135 and calculate AQI
float MQ135_ReadPPM() {
    uint32_t adc_sum = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        adc_sum += analogRead(MQ135_PIN);  // Read MQ135 analog
        delay(10);
    }

    uint32_t adc_value = adc_sum / NUM_SAMPLES;
    float resistance = calculateResistance(adc_value);
    resistance = applyExponentialSmoothing(resistance, &smooth_value_mq135);

    float ppm_co2 = calculatePPM(resistance, PPM_CO2_CONST, CO2_EXPONENT);
    float ppm_smoke = calculatePPM(resistance, PPM_SMOKE_CONST, SMOKE_EXPONENT);
    float ppm_nox = calculatePPM(resistance, PPM_NOX_CONST, NOX_EXPONENT);
    float ppm_o3 = calculatePPM(resistance, PPM_O3_CONST, O3_EXPONENT);

    float overall_aqi = fmax(fmax(calculateAQISubIndex(ppm_co2, co2_breakpoints, 4),
                                  calculateAQISubIndex(ppm_smoke, smoke_breakpoints, 4)),
                             fmax(calculateAQISubIndex(ppm_nox, nox_breakpoints, 4),
                                  calculateAQISubIndex(ppm_o3, o3_breakpoints, 4)));

    return overall_aqi;
}

// Function to read decibels from KY037
float KY037_ReadDecibels() {
    uint32_t adc_sum = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        adc_sum += analogRead(MQ135_PIN);  // Use the same pin to read KY037
        delay(10);
    }

    uint32_t adc_avg_value = adc_sum / NUM_SAMPLES;
    float voltage = (adc_avg_value / ADC_RESOLUTION) * REFERENCE_VOLTAGE;

    if (voltage < REFERENCE_VALUE) {
        voltage = REFERENCE_VALUE;
    }

    float decibels = (20 * log10(voltage / REFERENCE_VOLTAGE)) - CALIBRATION_OFFSET;
    decibels = abs(decibels); 

    return decibels;
}

// Function to send data to ThingSpeak
void sendToThingSpeak(float aqi, float decibels) {
    WiFiClient client;

    if (client.connect(server, 80)) {
        String url = "/update?api_key=";
        url += api_key;
        url += "&field1=";
        url += String(aqi);
        url += "&field2=";
        url += String(decibels);

        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: " + server + "\r\n" +
                     "Connection: close\r\n\r\n");

        client.stop();
    }
}

void setup() {
    Serial.begin(115200);
    analogRead(MQ135_PIN);

    pinMode(KY037_PIN, OUTPUT);      // KY037 pin as output to control power
    pinMode(MQ135_POWER_PIN, OUTPUT); // MQ135 power pin as output

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void loop() {
    // Read MQ135
    digitalWrite(MQ135_POWER_PIN, HIGH);   // Power on MQ135
    digitalWrite(KY037_PIN, LOW);          // Power off KY037
    delay(1000);  // Give time for MQ135 to stabilize
    float aqi = MQ135_ReadPPM();           // Read AQI from MQ135
    Serial.print("AQI: ");
    Serial.println(aqi);
    
    // Read KY037
    digitalWrite(MQ135_POWER_PIN, LOW);    // Power off MQ135
    digitalWrite(KY037_PIN, HIGH);        // Power on KY037
    delay(1000);  // Give time for KY037 to stabilize
    float decibels = KY037_ReadDecibels(); // Read decibels from KY037
    Serial.print("Noise Level: ");
    Serial.println(decibels);

    sendToThingSpeak(aqi, decibels);

    delay(100);  // Wait before switching again
}
