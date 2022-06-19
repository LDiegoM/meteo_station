/*
    Example: https://github.com/zhouhan0126/WebServer-esp32/blob/master/examples/SDWebServer/SDWebServer.ino

    To upload a file: https://tttapa.github.io/ESP8266/Chap12%20-%20Uploading%20to%20Server.html

*/
#ifndef http_handlers_h
#define http_handlers_h

#include <TFT_ILI9163C.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <wifi_connection.h>
#include <settings.h>
#include <storage.h>
#include <data_logger.h>

struct settings_mqtt_t {
    String server, username, password;
    uint16_t port, sendPeriod;
    String certData;
};

struct settings_date_t {
    String server;
    long gmtOffset;
    int daylightOffset;
};

/////////// HTTP Handlers
void downloadLogs();
void deleteLogs();
void getSettings();

void getBootstrapCSS();
void getBootstrapJS();
void getNotFound();

void getSettingsWiFi();
void addSettingsWiFi();
void updSettingsWiFi();
void delSettingsWiFi();

void getSettingsMQTT();
void updSettingsMQTT();
void getSettingsMQTTCert();

void getSettingsLogger();
void updSettingsLogger();

void getSettingsDate();
void updSettingsDate();

class HttpHandlers {
    private:
        const uint16_t METEO_HTTP_PORT = 80;

        const char* STATUS_DESCRIPTION = "This page contains current global status.";
        const char* SETTINGS_DESCRIPTION = "This page contains current configurations, and allows to modify them.";
        const char* ADMIN_DESCRIPTION = "This page allows to execute administrative commands.";

        const char* MSG_OK = "ok";
        const char* ERR_GENERIC = "Error saving settings. Please try again";
        const char* ERR_WIFI_AP_NOT_FOUND = "AP ssid not found";
        const char* ERR_WIFI_AP_IS_EMPTY = "AP ssid can't be empty";
        const char* ERR_WIFI_AP_EXISTS = "There's already an AP with the same SSID";

        const char* ERR_MQTT_IS_EMPTY = "MQTT parameters can't be empty";
        
        const char* ERR_LOGGER_IS_EMPTY = "Data Logger parameters can't be empty";

        const char* ERR_DATE_IS_EMPTY = "Date Time parameters can't be empty";

        WiFiConnection *m_wifi;
        Storage *m_storage;
        Settings *m_settings;
        TFT_ILI9163C *m_tft;
        WebServer *m_server;

        void defineRoutes();

        String getHeaderHTML(String section);
        String getFooterHTML(String page, String section);
        
        String getSettingsWiFiHTML();
        String getSettingsMQTTHTML();
        String getSettingsLoggerHTML();
        String getSettingsDateHTML();

        wifiAP_t parseWiFiBody(String body);
        std::vector<wifiAP_t> parseMultiWiFiBody(String body);

        settings_mqtt_t parseMQTTBody(String body);

        uint16_t parseLoggerBody(String body);

        settings_date_t parseDateBody(String body);

    public:
        HttpHandlers(WiFiConnection *wifi, Storage *storage, Settings *settings, TFT_ILI9163C *tft);

        bool begin();
        void loop();

        // HTTP handlers
        void handleDownloadLogs();
        bool handleDeleteLogs();
        void handleGetSettings();

        void handleGetBootstrapCSS();
        void handleGetBootstrapJS();
        void handleGetNotFound();

        void handleGetSettingsWiFi();
        void handleAddSettingsWiFi();
        void handleUpdSettingsWiFi();
        void handleDelSettingsWiFi();

        void handleGetSettingsMQTT();
        void handleUpdSettingsMQTT();
        void handleGetSettingsMQTTCert();

        void handleGetSettingsLogger();
        void handleUpdSettingsLogger();

        void handleGetSettingsDate();
        void handleUpdSettingsDate();
};

extern HttpHandlers *httpHandlers;

#endif
