#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>
#include <Fonts/FreeSans9pt7b.h>
#include <timer.h>
#include <number_set.h>
#include <storage.h>
#include <settings.h>
#include <sensors.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <PubSubClient.h>

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

// Wifi configuration
WiFiMulti wifiMulti;

Storage *storage;
Settings *settings;

// MQTT Broker settings
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);
Timer *tmrConnectMQTT, *tmrSendValuesToMQTT;
const char* mqttTempTopic = "topic-meteo-temp";
const char* mqttTempTopic2 = "topic-meteo-temp2";
const char* mqttPresTopic = "topic-meteo-pres";
const char* mqttHumiTopic = "topic-meteo-humi";
const char* mqttCmdTopic = "topic-meteo-cmd";

// Starts showing humidity
bool flgShowPres = false;

bool connectWifi();
bool connectMQTT();
void drawScreen();
void drawPresHumi();
void printDateTime();
void sendValuesToMQTT();
void printValues();
void switchHumiPres();
void messageReceived(char* topic, uint8_t* payload, unsigned int length);

bool connectMQTT() {
    if (mqttClient.connected())
        return true;
    
    if (!settings->isSettingsOK())
        return false;

    tmrRefreshTime->stop();
    tmrShowPres->stop();

    tft->fillScreen(BLACK);
    tft->setTextColor(WHITE);
    int line = 0;

    // If wifi is not connected, try to connect
    if (WiFi.status() != WL_CONNECTED) {
        tft->setCursor(2, 20 * ++line);
        tft->print("WiFi Connect");
        tft->setCursor(2, 20 * ++line);
        if (!connectWifi()) {
            tft->print("FAIL");
            return false;
        }
        tft->print("OK: " + WiFi.SSID());
        delay(1000);
    }

    // Get current time
    configTime(
        settings->getSettings().dateTime.gmtOffset * 60 * 60,
        settings->getSettings().dateTime.daylightOffset * 60 * 60,
        settings->getSettings().dateTime.server.c_str());

    line++;
    tft->setCursor(2, 20 * ++line);
    tft->print("MQTT Connect");
    tft->setCursor(2, 20 * ++line);

    String clientID = "ESP32-device_" + WiFi.localIP().toString();
    mqttClient.setServer(settings->getSettings().mqtt.server.c_str(), settings->getSettings().mqtt.port);
    if (!mqttClient.connect(clientID.c_str(),
                            settings->getSettings().mqtt.username.c_str(),
                            settings->getSettings().mqtt.password.c_str())) {
        tft->print("FAIL");
        return false;
    }
    mqttClient.subscribe(mqttCmdTopic);
    tft->print("OK");
    delay(1000);
    printValues();
    drawScreen();
    sendValuesToMQTT();

    tmrRefreshTime->start();
    tmrShowPres->start();
    tmrConnectMQTT->stop();

    return true;
}

bool connectWifi() {
    return wifiMulti.run() == WL_CONNECTED;
}

void printDateTime() {
    tm timeInfo;
    if(!getLocalTime(&timeInfo)){
        Serial.println("Failed to obtain time");
        return;
    }

    char year[4], month[3], day[3], hour[3], minutes[3];
    strftime(year, 5, "%Y", &timeInfo);
    strftime(month, 3, "%m", &timeInfo);
    strftime(day, 3, "%d", &timeInfo);
    strftime(hour, 3, "%H", &timeInfo);
    strftime(minutes, 3, "%M", &timeInfo);

    timeRep->setValue(String(hour) + ":" + String(minutes));
    ddmmRep->setValue(String(day) + "." + String(month));
    yearRep->setValue("." + String(year));
}

void sendValuesToMQTT() {
    if (!mqttClient.connected())
        return;
    
    mqttClient.publish(mqttTempTopic, String(sensors->temp()).c_str(), true);
    mqttClient.publish(mqttTempTopic2, String(sensors->temp2()).c_str(), true);
    mqttClient.publish(mqttPresTopic, String(sensors->pres()).c_str(), true);
    mqttClient.publish(mqttHumiTopic, String(sensors->humi()).c_str(), true);
}

void messageReceived(char* topic, uint8_t* payload, unsigned int length) {
    Serial.println("Message received from topic " + String(topic) + " - length: " + String(length));
    if (!String(topic).equals(mqttCmdTopic))
        return;
    
    String incomingMessage = "";
    for (int i = 0; i < length; i++)
        incomingMessage += (char)payload[i];
    
    Serial.println("incomingMessage: " + incomingMessage);
    if (incomingMessage.equals("RESEND")) {
        sendValuesToMQTT();
    }
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

void setup(void) {
    Serial.begin(9600);

    tft = new TFT_ILI9163C(TFT_CS, TFT_DC);
    tft->begin();
    tft->setFont(&FreeSans9pt7b);

    sensors = new Sensors(DHTPIN, DHTTYPE, tft);
    while (!sensors->begin()) {}

    storage = new Storage(SD_CS, tft);
    while (!storage->begin()) {}
    settings = new Settings(storage);
    if (!settings->isSettingsOK()) {
        Serial.println("Settings are not ok");
        return;
    }

    tempRep = new NumberSet(tft, 3, 20, NS_SFLT, 2, BACKGROUND, FORE_COLOR);
    presHumiRep = new NumberSet(tft, 64 + 3, 20, NS_SFLT, 2, BACKGROUND, FORE_COLOR);
    timeRep = new NumberSet(tft, (tft->width() - NS_HHMM_3_W) / 2, tempRep->y() + tempRep->height() + 15, NS_HHMM, 3, BACKGROUND, FORE_COLOR);
    ddmmRep = new NumberSet(tft, tft->width() - (NS_UFLT_1_W * 2) - 3, tft->height() - NS_SIZE_1_H - 3, NS_UFLT, 1, BACKGROUND, FORE_COLOR);
    yearRep = new NumberSet(tft, ddmmRep->x() + ddmmRep->width(), ddmmRep->y(), NS_UFLT, 1, BACKGROUND, FORE_COLOR);

    tmrConnectMQTT = new Timer(5000);
    tmrRefreshTime = new Timer(1000);
    tmrSendValuesToMQTT = new Timer(settings->getSettings().mqtt.sendPeriod * 1000);
    tmrShowPres = new Timer(10 * 1000);

    for (int i = 0; i < settings->getSettings().wifiAPs.size(); i++) {
        wifiMulti.addAP(settings->getSettings().wifiAPs[i].ssid.c_str(),
                        settings->getSettings().wifiAPs[i].password.c_str());
    }

    espClient.setCACert(settings->getSettings().mqtt.ca_cert);
    mqttClient.setCallback(messageReceived);
    connectMQTT();

    tmrSendValuesToMQTT->start();
}

void loop() {
    if (!mqttClient.connected()) {
        if (!tmrConnectMQTT->isRunning()) {
            tmrConnectMQTT->start();
        }

        if (tmrConnectMQTT->isTime()) {
            Serial.println("MQTT not connected. Reconnecting");
            if (!connectMQTT()) {
                return;
            }
        }
    } else {
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

        if (tmrSendValuesToMQTT->isTime()) {
            sendValuesToMQTT();
        }

        tempRep->refresh();
        presHumiRep->refresh();
        timeRep->refresh();
        ddmmRep->refresh();
        yearRep->refresh();

        mqttClient.loop();
    }
}
