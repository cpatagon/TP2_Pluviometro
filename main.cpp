/** Proyecto plubiometro 2**/

#include "mbed.h"
#include "arm_book_lib.h"
//#include "weather_station.h"



#define TIME_INI  1593561600  // 1 de julio de 2020, 00:00:00
#define BAUD_RATE 9600
#define DELAY_BETWEEN_TICK 500 // 500 ms
#define SWITCH_TICK_RAIN BUTTON1
#define RAINFALL_CHECK_INTERVAL 1 // in minute
#define MM_PER_TICK 0.2f // 0.2 mm de agua por tick
#define RAINFALL_COUNT_INIT 0
#define LAST_MINUTE_VALUE -1
#define MINUTES_PASSED_INIT 0

// Sensores
void initializeSensors();
bool isRaining();

// Análisis de Datos
void analyzeRainfall();
void accumulateRainfall();
bool hasTimePassedMinutesRTC(int minutes);

// Actuación
void actOnRainfall();
void reportRainfall();
void printRain(const char* buffer);
const char* DateTimeNow(void);
void printAccumulatedRainfall();


BufferedSerial pc(USBTX, USBRX, BAUD_RATE);

DigitalOut alarmLed(LED1);
DigitalIn tickRain(SWITCH_TICK_RAIN);
DigitalOut tickLed(LED2);

int rainfallCount = RAINFALL_COUNT_INIT;
int lastMinute = LAST_MINUTE_VALUE;

int main()
{
    initializeSensors();

    while (true) {
      if (isRaining()) {
             actOnRainfall();
        } else {
            alarmLed = OFF;
            tickLed = OFF;
        }
       if (hasTimePassedMinutesRTC(RAINFALL_CHECK_INTERVAL)) {
            reportRainfall();
        }
    }
}

// Sensores
void initializeSensors() {
    tickRain.mode(PullDown);
    alarmLed = OFF;
    tickLed = OFF;
    set_time(TIME_INI); // Configurar la fecha y hora inicial
}

bool isRaining() {
    return (tickRain == ON);
}


void printAccumulatedRainfall() {
    time_t seconds = time(NULL);
    struct tm* timeinfo = localtime(&seconds);
    char dateTime[80];
    strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M", timeinfo);
    
    int rainfallInteger = (int)(rainfallCount * MM_PER_TICK);
    int rainfallDecimal = (int)((rainfallCount * MM_PER_TICK - rainfallInteger) * 100);
    
    char buffer[100];
    int len = sprintf(buffer, "%s - Accumulated rainfall: %d.%02d mm\n", 
                      dateTime, rainfallInteger, rainfallDecimal);
    
     pc.write(buffer, len);
}

void accumulateRainfall() {
    time_t seconds = time(NULL);
    struct tm* timeinfo = localtime(&seconds);
    int currentMinute = timeinfo->tm_min;
    
    rainfallCount++;
    
    if (currentMinute != lastMinute) {
        if (lastMinute != LAST_MINUTE_VALUE) {  // No imprimir en la primera iteración
            printf("Rainfall in the last minute: %d ticks\n", rainfallCount);
        }
        rainfallCount = RAINFALL_COUNT_INIT;
        lastMinute = currentMinute;
    }
}

// funcion que entrega la fecha y hora actual 
const char* DateTimeNow() {
    time_t seconds = time(NULL);
    static char bufferTime[80];
    strftime(bufferTime, sizeof(bufferTime), "%Y-%m-%d %H:%M:%S", localtime(&seconds));
    return bufferTime;
}

// funcion que imprime cuando deteta una precipitacion 
void printRain(const char* buffer) {
    // Write the buffer content
    pc.write(buffer, strlen(buffer));
    // Write the rain detected message
    const char* message = " - Rain detected\r\n";
    pc.write(message, strlen(message));
}


bool hasTimePassedMinutesRTC(int minutes) {
    static int lastMinute = LAST_MINUTE_VALUE;
    static int minutesPassed = MINUTES_PASSED_INIT;

    time_t seconds = time(NULL);
    struct tm* timeinfo = localtime(&seconds);
    int currentMinute = timeinfo->tm_min;

    if (lastMinute == LAST_MINUTE_VALUE) {
        lastMinute = currentMinute;
        return false;
    }

    if (currentMinute != lastMinute) {
        minutesPassed++;
        lastMinute = currentMinute;

        if (minutesPassed >= minutes) {
            minutesPassed = MINUTES_PASSED_INIT;
            return true;
        }
    }

    return false;
}
