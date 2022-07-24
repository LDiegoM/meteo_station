#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>
#include <Fonts/FreeSans9pt7b.h>

#include <data_logger.h>
#include <date_time.h>
#include <display.h>
#include <http_handlers.h>
#include <mqtt_handlers.h>
#include <number_set.h>
#include <sensors.h>
#include <settings.h>
#include <storage.h>
#include <timer.h>
#include <wifi_connection.h>

// Color definitions
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define GRAY 0x8410
#define DARK_GRAY 0x4A69

#define BACKGROUND WHITE
#define FORE_COLOR BLACK

#define TFT_CS 2 // TFT display cable select pin
#define TFT_DC 4 // TFT display command pin

#define DHTPIN 17
#define DHTTYPE DHT11

TFT_ILI9163C* tft;

Sensors *sensors;
Storage *storage;
Settings *settings;
WiFiConnection *wifi;
DateTime *dateTime;
Display *display;

void setup(void) {
    Serial.begin(115200);

    tft = new TFT_ILI9163C(TFT_CS, TFT_DC);
    tft->begin();
    tft->setFont(&FreeSans9pt7b);
    tft->fillScreen(BLACK);
    tft->setTextColor(WHITE);
    tft->setCursor(2, 0);

    sensors = new Sensors(DHTPIN, DHTTYPE, tft);
    while (!sensors->begin()) {
        tft->fillScreen(BLACK);
        tft->setTextColor(WHITE);
        tft->setCursor(2, 0);
    }

    storage = new Storage(tft);
    while (!storage->begin()) {
        tft->fillScreen(BLACK);
        tft->setTextColor(WHITE);
        tft->setCursor(2, 0);
    }
    
    settings = new Settings(storage, tft);
    if (!settings->begin()) {
        Serial.println("Settings are not ok");
        return;
    }
    if (!settings->isSettingsOK())
        return;
    settings_t config = settings->getSettings();

    wifi = new WiFiConnection(settings, tft);
    wifi->begin();

    // Configure current time
    configTime(config.dateTime.gmtOffset * 60 * 60,
               config.dateTime.daylightOffset * 60 * 60,
               config.dateTime.server.c_str());
    dateTime = new DateTime();

    dataLogger = new DataLogger(sensors, dateTime, storage, config.logger.outputPath, config.logger.writePeriod);
    dataLogger->logData();

    mqtt = new MqttHandlers(wifi, sensors, settings, tft, dataLogger, storage);
    if (!mqtt->begin())
        Serial.println("MQTT is not connected");

    httpHandlers = new HttpHandlers(wifi, storage, settings, dataLogger, sensors, mqtt);
    if (!httpHandlers->begin()) {
        Serial.println("Could not start http server");
        return;
    }

    display = new Display(tft, BACKGROUND, FORE_COLOR, sensors, wifi, dateTime);
    display->begin();
}

void loop() {
    if (!settings->isSettingsOK())
        return;

    sensors->loop();
    dataLogger->loop();
    httpHandlers->loop();
    mqtt->loop();
    display->loop();
}
