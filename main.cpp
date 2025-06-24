#include "mbed.h"
#include "DHT.h"
#include "DS3231.h"
#include "N5110.h"
#include "Joystick.h"
#include "PlantThresholds.h"
#include <cstdio>
#include <string>

///////////// objects ///////////////////
I2C i2c(PB_9, PB_8);
DS3231 rtc(i2c);
BufferedSerial pc(USBTX, USBRX, 9600);
N5110 lcd(PC_7, PA_9, PB_10, PB_5, PB_3, PA_10);
Joystick joystick(PA_0, PA_1);
DigitalIn button1(PC_10);
DigitalOut led(LED1);
AnalogIn LDR(PA_5);
AnalogIn moisture(A0);
DHT dhtSensor(D5, DHT22);
DigitalInOut pump_gate(PC_6);

// Ticker for low power periodic wake-up
LowPowerTicker readTicker; //Set Ticker
EventFlags flags; // Set flags
#define FLAG_READ_SENSORS 0x01

// Timing intervals (in seconds)
const int DHT_INTERVAL = 10800; // 3 hours
const int LIGHT_INTERVAL = 7200; // 2 hours
const int SOIL_INTERVAL = 10800; // 3 hours
const int WATERING_DURATION_MS = 4000; // Watering time: 4 seconds

// Evaluation times based on RTC
const int LIGHT_EVAL_HOUR1 = 12; // Midday light check
const int LIGHT_EVAL_HOUR2 = 19;  // Evening light check
const int SOIL_EVAL_HOUR = 17; // Soil check daily


PlantType selectedPlant = SPIDER_PLANT; // Set as default plant
PlantThreshold activeThreshold = PlantDatabase[selectedPlant]; // Load threshold values

// Store Sensor value 
float temp = 0, humidity = 0, light = 0, soilMoisture = 0;
float light_sum_morning = 0;
float light_sum_afternoon = 0;
int light_reads_morning = 0;
int light_reads_afternoon = 0;
float soil_sum = 0;
int soil_reads = 0;
// Soil calibration ADC values
const float DRY_ADC = 0.68f;    // ADC value when dry
const float WET_ADC = 0.30f;    // ADC value when saturated

// Time tracking for events
time_t last_temp_hum_warning = 0;
time_t last_soil_eval = 0;
time_t last_light_eval = 0;
time_t last_light_read = 0;
time_t last_temp_hum_read = 0;
time_t last_soil_read = 0;
time_t last_watering_time = 0;
time_t system_start_time = 0;

///////////// prototypes ///////////////
void init();
void showError(const char* msg);
void showWarning(const char* label, const char* level);
const char* evaluate(float value, float min, float max);
void display_all(const char* timeStr);
void pump_on();
void pump_off();
void readTickerCallback();
void readSensors(time_t now);
void plant_selection_menu();
////////////////////////////////////////

int main() {
init(); // Initialize Sysytem
pump_off(); // Make sure pump is off
lcd.printString("Smart Planter", 0, 1);
lcd.printString("Initializing...", 0, 3);
lcd.refresh();
ThisThread::sleep_for(2000ms);
plant_selection_menu(); // Call the main menu function

struct tm timeinfo;
if (!rtc.readTime(&timeinfo)) showError("RTC Fail"); // RTC error handling

system_start_time = time(NULL);
flags.set(FLAG_READ_SENSORS);
readTicker.attach(callback(readTickerCallback), 3600); // 1 hour interval

char timeStr[17];
while (true) {
    rtc.readTime(&timeinfo);
    time_t now = time(NULL);
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

    if (flags.get() & FLAG_READ_SENSORS) {
        flags.clear(FLAG_READ_SENSORS);
        readSensors(now);
    }

    display_all(timeStr);
    led = !led;
    ThisThread::sleep_for(500ms);
}
}

// System initialization
void init() {
lcd.init(LPH7366_1);
lcd.setContrast(0.5);
joystick.init();
}

// Display error on LCD
void showError(const char* msg) {
lcd.clear();
lcd.printString("Sensor Error", 0, 1);
lcd.printString(msg, 0, 3);
lcd.refresh();
ThisThread::sleep_for(3000ms);
}

// Display warning message
void showWarning(const char* label, const char* level) {
lcd.clear();
lcd.printString(label, 0, 1);
lcd.printString(level, 0, 3);
lcd.refresh();
ThisThread::sleep_for(10000ms);
}

// Compare sensor reading against thresholds
const char* evaluate(float value, float min, float max) {
if (value < min) return "Too Low";
else if (value > max) return "Too High";
else return "OK";
}

// Show all sensor statuses on LCD
void display_all(const char* timeStr) {
lcd.clear();
char line0[17], line1[17], line2[17], line3[17], line4[17], line5[17];
snprintf(line0, sizeof(line0), "%s", PlantNames[selectedPlant]);
snprintf(line1, sizeof(line1), "Temp: %s", evaluate(temp, activeThreshold.min_temp, activeThreshold.max_temp));
snprintf(line2, sizeof(line2), "Hum: %s", evaluate(humidity, activeThreshold.min_humidity, activeThreshold.max_humidity));
snprintf(line3, sizeof(line3), "Light: %s", evaluate(light, activeThreshold.min_light, activeThreshold.max_light));
snprintf(line4, sizeof(line4), "Soil: %s", evaluate(soilMoisture, activeThreshold.min_soil, activeThreshold.max_soil));
snprintf(line5, sizeof(line5), "Time: %s", timeStr);
lcd.printString(line0, 0, 0);
lcd.printString(line1, 0, 1);
lcd.printString(line2, 0, 2);
lcd.printString(line3, 0, 3);
lcd.printString(line4, 0, 4);
lcd.printString(line5, 0, 5);
lcd.refresh();
}

void pump_on() { pump_gate.input(); }
void pump_off() { pump_gate.output(); pump_gate = 0; }

// Ticker callback to wake the system
void readTickerCallback() {
flags.set(FLAG_READ_SENSORS);
}

void readSensors(time_t now) {
struct tm t;
rtc.readTime(&t);
if (now - last_temp_hum_read >= DHT_INTERVAL || last_temp_hum_read == 0) {
    if (dhtSensor.readData() == ERROR_NONE) {
        temp = dhtSensor.ReadTemperature(CELCIUS);
        humidity = dhtSensor.ReadHumidity();
        last_temp_hum_read = now;

        if (now - last_temp_hum_warning >= DHT_INTERVAL) {
            if (strcmp(evaluate(temp, activeThreshold.min_temp, activeThreshold.max_temp), "OK") != 0)
                showWarning("Temp", evaluate(temp, activeThreshold.min_temp, activeThreshold.max_temp));
            if (strcmp(evaluate(humidity, activeThreshold.min_humidity, activeThreshold.max_humidity), "OK") != 0)
                showWarning("Humidity", evaluate(humidity, activeThreshold.min_humidity, activeThreshold.max_humidity));
            last_temp_hum_warning = now;
        }
    } else {
        showError("DHT Fail");
    }
}

if (now - last_light_read >= LIGHT_INTERVAL || last_light_read == 0) {
    light = LDR.read() * 100.0f;
    if (t.tm_hour >= 19 || t.tm_hour < 12) {
        light_sum_morning += light;
        light_reads_morning++;
    } else {
        light_sum_afternoon += light;
        light_reads_afternoon++;
    }
    last_light_read = now;
}

if (t.tm_hour == LIGHT_EVAL_HOUR1 && t.tm_min == 0 && now - last_light_eval >= 3600) {
    float morning_avg = light_reads_morning ? light_sum_morning / light_reads_morning : 0;
    if (morning_avg < activeThreshold.min_light)
        showWarning("Light 7pm-12", "Too Low");
    light_sum_morning = 0; light_reads_morning = 0;
    last_light_eval = now;
}

if (t.tm_hour == LIGHT_EVAL_HOUR2 && t.tm_min == 0 && now - last_light_eval >= 3600) {
    float afternoon_avg = light_reads_afternoon ? light_sum_afternoon / light_reads_afternoon : 0;
    float total_avg = (light_reads_morning + light_reads_afternoon) > 0
        ? (light_sum_morning + light_sum_afternoon) / (light_reads_morning + light_reads_afternoon)
        : 0;
    if (total_avg < activeThreshold.min_light)
        showWarning("Light Total", "Too Low");
    light_sum_afternoon = light_sum_morning = 0;
    light_reads_afternoon = light_reads_morning = 0;
    last_light_eval = now;
}

if (now - last_soil_read >= SOIL_INTERVAL || last_soil_read == 0) {
    float raw_adc = moisture.read();
    soilMoisture = (DRY_ADC - raw_adc) / (DRY_ADC - WET_ADC) * 100.0f;
    soilMoisture = soilMoisture < 0 ? 0 : (soilMoisture > 100 ? 100 : soilMoisture); // Clamp between 0–100%
    soil_sum += soilMoisture;
    soil_reads++;
    last_soil_read = now;
}

if (t.tm_hour == SOIL_EVAL_HOUR && t.tm_min == 0 && now - last_soil_eval >= 3600) {
    float avg_soil = soil_reads ? soil_sum / soil_reads : 0;
    if (avg_soil < activeThreshold.min_soil && now - last_watering_time >= SOIL_INTERVAL) {
        showWarning("Soil", "Too Dry");
        pump_on(); ThisThread::sleep_for(WATERING_DURATION_MS); pump_off();
        last_watering_time = now;
    } else if (avg_soil > activeThreshold.max_soil) {
        showWarning("Soil", "Too Moist");
    }
    soil_sum = 0; soil_reads = 0;
    last_soil_eval = now;
}
}

void plant_selection_menu() {
int index = 0;
joystick.init();
lcd.clear(); lcd.printString("Select Plant", 0, 0); lcd.refresh();
ThisThread::sleep_for(2000ms);
while (true) {
    lcd.clear();
    lcd.printString("Select Plant", 0, 0);
    for (int i = 0; i < 5; i++) {
        int idx = (index + i) % NUM_PLANTS;
        lcd.printString(PlantNames[idx], 1, i + 1);
    }
    lcd.printChar('<', 75, 1);
    lcd.refresh();

    Direction dir = joystick.get_direction();
    if (dir == S && index < NUM_PLANTS - 1) { index++; }
    else if (dir == N && index > 0) { index--; }
    else if (button1 == 1) {
        selectedPlant = static_cast<PlantType>(index);
        activeThreshold = PlantDatabase[selectedPlant];
        break;
    }
    ThisThread::sleep_for(250ms);
}
}