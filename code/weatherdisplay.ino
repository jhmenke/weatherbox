#include "WS2801/WS2801.h"
#include "Grove_OLED_Display_96X96/Grove_OLED_Display_96X96.h"
#include "ledInclude.h"
#include "weatherInclude.h"
#include "application.h"

// system settings
#define CYCLE_MS 100
#define TO_S 1000.0/CYCLE_MS
#define PIR_ON_TIME TO_S*30
#define PUMP_PIN D6
#define FOGGER_PIN A1
bool systemActive = true;
unsigned int onTime = 0;
bool weatherChange = false;

// multicolor LED variables
#define ROTARYPIN A0
float hue = 0.0;
int buffer = 0;

// weather variables
char weatherStringBuffer[64];
char *weatherToday = NULL;
char *temperatureToday = NULL;
char *weatherTomorrow = NULL;
char *temperatureTomorrow = NULL;
char weatherKnob = 0;

// PIR sensor
unsigned int lastActiveCycle = 0;
#define PIR_SENSOR_PORT D4

void setup()
{
    // Default weather
    currentWeather = error;
    // Multicolor LED strip (A3 for clock, A5 for data)
    ledStrip.begin();
    // Webhook
    // Particle.subscribe("weatherUpdate", weatherHandler, MY_DEVICES); // IFTTT Weather
    Particle.subscribe("hook-response/weatherUpdate", weatherHandler, MY_DEVICES); // DarkSkyAPI
    // OLED Display
    Wire.begin();
    SeeedGrayOled.init();     
    SeeedGrayOled.clearDisplay();
    SeeedGrayOled.setNormalDisplay();
    SeeedGrayOled.setVerticalMode(); 
    putLine(6, "Starting"); 
    // pump
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(FOGGER_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(FOGGER_PIN, LOW);
}

void weatherHandler(const char *event, const char *data)
{
    // flash green color to indicate internet update
    setStripColorHSB(0.35, 1.0, 0.5);
    // Parse weather from Darksky API
    String str = String(data);
    str.toCharArray(weatherStringBuffer, 64); 
    // Relevant variables
    weatherToday = strtok(weatherStringBuffer, "~");
    temperatureToday = strtok(NULL, "~");
    weatherTomorrow = strtok(NULL, "~");
    temperatureTomorrow = strtok(NULL, "~");
    // set weather
    updateWeather();
}

void updateWeather() {
    weatherChange = true;
    char weatherString[24];
    char temperature[8];
    String timeString;
    // choose correct data, copy just to be safe
    if (weatherKnob == 1) {
        strcpy(weatherString, weatherToday);
        strcpy(temperature, temperatureToday);
        timeString = String("today");
    } else if (weatherKnob == 0) {
        strcpy(weatherString, weatherTomorrow);
        strcpy(temperature, temperatureTomorrow);
        timeString = String("tomorrow");
    } else {
        strcpy(weatherString, "test");
        strcpy(temperature, "99.99");
        timeString = String("debug");
    }
    // update currentWeather object
    if (strlen(weatherString) == 0) {
        currentWeather = error;
    } else if (strcmp(weatherString, "rain") == 0 ||
               strcmp(weatherString, "hail") == 0) {
        currentWeather = rain;
    } else if(strcmp(weatherString, "clear-day") == 0) {
        currentWeather = sunshine;
    } else if (strcmp(weatherString, "thunderstorm") == 0 ||
               strcmp(weatherString, "tornado") == 0) {
        currentWeather = thunderstorm;
    } else if (strcmp(weatherString, "fog") == 0 ||
               strcmp(weatherString, "cloudy") == 0 ||
               strcmp(weatherString, "partly-cloudy-day") == 0 ||
               strcmp(weatherString, "partly-cloudy-night") == 0) {
        currentWeather = fog;
    } else if (strcmp(weatherString, "snow") == 0 ||
        strcmp(weatherString, "sleet") == 0) {
        currentWeather = snow;
    } else if(strcmp(weatherString, "clear-night") == 0) {
        currentWeather = night;
    } else if(strcmp(weatherString, "wind") == 0) {
        currentWeather = wind;
    } else if(strcmp(weatherString, "test") == 0) {
        currentWeather = rain;
    } else {
        currentWeather = error; // this should not happen
    }
    // write to OLED Display
    // make sure to use full 12 char lines to avoid clearing the display (buggy function)
    String weatherLine = String(weatherString);
    String temperatureLine = String(temperature) + " C";
    putLine(2, "Weather");
    putLine(4, timeString); 
    putLine(6, weatherLine); 
    putLine(8, "Max. temp.: "); 
    putLine(10, temperatureLine);
}

void putLine(int line, String s) {
    // line must be in [0, 2, 4, 6, 8, 10, 12]
    SeeedGrayOled.setTextXY(line, 0);  
    for(int i = 0; i < 12; i++) {
        if(i < s.length()) {
            SeeedGrayOled.putChar(s.charAt(i));
        } else {
            SeeedGrayOled.putChar(' ');
        }
    }
}


void loop()
{
    // check overflow of onTime
    if (lastActiveCycle > onTime) {
        lastActiveCycle = onTime;
    }
    // check PIR Sensor
    if (digitalRead(PIR_SENSOR_PORT) == HIGH) {
        if (!systemActive) {
            updateWeather();
            systemActive = true;
        }
        lastActiveCycle = onTime;
    } else {
        if (systemActive && (onTime - lastActiveCycle) >= PIR_ON_TIME) {
            systemActive = false;
            currentWeather = noWeather;
            weatherChange = true;
        }
    }
    // weather today or tomorrow?
    char weatherKnobBackup = weatherKnob;
    int knobState = analogRead(ROTARYPIN);
    if (knobState >= 3000) {
        weatherKnob = 2;
    } else if (knobState >= 1600) {
        weatherKnob = 1;
    } else {
        weatherKnob = 0;
    }
    if (weatherKnob != weatherKnobBackup) {
        updateWeather();
    }
    // check if weather has changed since last cycle, if it has: set weather
    if (weatherChange) {
        // led
        setStripColorHSB(currentWeather.hueLed, currentWeather.saturationLed, currentWeather.brightnessLed);
        // water pump
        digitalWrite(PUMP_PIN, currentWeather.dcPump > 0.0 ? HIGH : LOW);
        // fogger
        digitalWrite(FOGGER_PIN, currentWeather.dcFogger > 0.0 ? HIGH : LOW);
        // analogWrite(A1, int(255 * currentWeather.dcFogger)); // alternative: real duty cycle
        weatherChange = false;
    }
    // check weather online (every 5 minutes)
    if ((onTime % (300*1000/CYCLE_MS) == 0) && (systemActive)) {
        // blue LED indicates request
        setStripColorHSB(0.5, 1.0, 0.5);
        // if the connection was successful, publish the request
        if (WiFi.ready()) {
            Particle.publish("weatherUpdate");    
        }
    }
    // next loop()
    onTime++;
    delay(CYCLE_MS);    
}
