#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>
#include <Fonts/FreeSans9pt7b.h>

#include <timer.h>
#include <sensors.h>
#include <storage.h>
#include <settings.h>
#include <wifi_connection.h>
#include <date_time.h>
#include <mqtt.h>
#include <number_set.h>
#include <data_logger.h>
#include <http_handlers.h>

// Color definitions
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define GRAY 0x8410
#define DARK_GRAY 0x4A69

#define BACKGROUND WHITE
#define FORE_COLOR BLACK

#define TFT_CS 2 // TFT display cable select pin
#define TFT_DC 4 // TFT display command pin

#define DHTPIN 17
#define DHTTYPE DHT11

#define SD_CS 5  // SD card module cable select pin

TFT_ILI9163C* tft;

Sensors *sensors;

NumberSet *tempRep, *presHumiRep, *ddmmRep, *yearRep, *timeRep;
Timer *tmrRefreshTime, *tmrShowPres;

Storage *storage;
Settings *settings;
WiFiConnection *wifi;
DateTime *dateTime;
MQTT *mqtt;
DataLogger *dataLogger;
HttpHandlers *httpHandlers;

// Starts showing humidity
bool flgShowPres = false;

void drawScreen();
void drawPresHumi();
void printDateTime();
void printValues();
void switchHumiPres();
void messageReceived(char* topic, uint8_t* payload, unsigned int length);
void downloadLogs();
void deleteLogs();
void getSettings();

void printDateTime() {
    if(!dateTime->refresh()) {
        Serial.println("Failed to refresh time");
        return;
    }

    timeRep->setValue(dateTime->hour() + ":" + dateTime->minutes());
    ddmmRep->setValue(dateTime->day() + "." + dateTime->month());
    yearRep->setValue("." + dateTime->year());
}

void printValues() {
    tempRep->setValue(sensors->temp());
    if (flgShowPres)
        presHumiRep->setValue(sensors->pres());
    else
        presHumiRep->setValue(sensors->humi());
}

void switchHumiPres() {
    flgShowPres = !flgShowPres;
}

void drawPresHumi() {
    // First blank space where word will be written
    tft->fillRect(64+1, 0, tft->width(), 40, BACKGROUND);

    tft->setCursor(64 + 10, 16);
    if (flgShowPres)
        tft->print("Pres");
    else
        tft->print("Hum");
}

void drawScreen() {
    tft->fillScreen(BACKGROUND);
    tft->setTextColor(FORE_COLOR);
    tft->setCursor(10, 16);
    tft->print("Temp");
    drawPresHumi();

    // Upper horizontal double line
    tft->drawRect(0, tempRep->y() + tempRep->height() + 3, tft->width(), 2, FORE_COLOR);
    // Upper vertical line
    tft->drawFastVLine(64, 0, tempRep->y() + tempRep->height() + 3, FORE_COLOR);
    // Lower horizontal single line
    tft->drawRect(0, ddmmRep->y() - 3, tft->width(), 1, FORE_COLOR);

    printDateTime();
}

void messageReceived(char* topic, uint8_t* payload, unsigned int length) {
    mqtt->processReceivedMessage(topic, payload, length);
}

/////////// HTTP Handlers
void downloadLogs() {
    httpHandlers->handleDownloadLogs();
}

void deleteLogs() {
    if (httpHandlers->handleDeleteLogs())
        dataLogger->logData();
}

void getSettings() {
    httpHandlers->handleGetSettings();
}

void setup(void) {
    Serial.begin(9600);
    Serial.println("begin setup - " + String(ESP.getFreeHeap()));

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

    storage = new Storage(SD_CS, tft);
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

    wifi = new WiFiConnection(settings, tft);
    wifi->begin();

    httpHandlers = new HttpHandlers(wifi, storage, settings, tft,
                                    downloadLogs, deleteLogs, getSettings);
    if (!httpHandlers->begin()) {
        Serial.println("Could not start http server");
        return;
    }

    tempRep = new NumberSet(tft, 3, 20, NS_SFLT, 2, BACKGROUND, FORE_COLOR);
    presHumiRep = new NumberSet(tft, 64 + 3, 20, NS_SFLT, 2, BACKGROUND, FORE_COLOR);
    timeRep = new NumberSet(tft, (tft->width() - NS_HHMM_3_W) / 2, tempRep->y() + tempRep->height() + 15, NS_HHMM, 3, BACKGROUND, FORE_COLOR);
    ddmmRep = new NumberSet(tft, tft->width() - (NS_UFLT_1_W * 2) - 3, tft->height() - NS_SIZE_1_H - 3, NS_UFLT, 1, BACKGROUND, FORE_COLOR);
    yearRep = new NumberSet(tft, ddmmRep->x() + ddmmRep->width(), ddmmRep->y(), NS_UFLT, 1, BACKGROUND, FORE_COLOR);

    // Get current time
    dateTime = new DateTime(
        settings->getSettings().dateTime.gmtOffset,
        settings->getSettings().dateTime.daylightOffset,
        settings->getSettings().dateTime.server.c_str());

    dataLogger = new DataLogger(sensors, dateTime, storage,
                                settings->getSettings().storage.outputPath,
                                settings->getSettings().storage.writePeriod);
    dataLogger->logData();

    mqtt = new MQTT(wifi, sensors, settings, tft, dataLogger, messageReceived);
    if (!mqtt->begin()) {
        Serial.println("MQTT is not connected");
        return;
    }

    drawScreen();
    printDateTime();
    
    tmrRefreshTime = new Timer(1000);
    tmrShowPres = new Timer(10 * 1000);

    tmrRefreshTime->start();
    tmrShowPres->start();

    Serial.println("end setup - " + String(ESP.getFreeHeap()));
}

void loop() {
    if (mqtt->isConnected()) {
        if (tmrRefreshTime->isTime()) {
            printDateTime();
        }

        if (tmrShowPres->isTime()) {
            switchHumiPres();
            drawPresHumi();
            printValues();
        }

        sensors->loop();
        if (sensors->updated()) {
            printValues();
        }

        tempRep->refresh();
        presHumiRep->refresh();
        timeRep->refresh();
        ddmmRep->refresh();
        yearRep->refresh();

        dataLogger->loop();
        httpHandlers->loop();
    }
    mqtt->loop();
}
