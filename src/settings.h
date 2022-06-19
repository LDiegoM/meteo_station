#ifndef settings_h
#define settings_h

#include <vector>
#include <ArduinoJson.h>
#include <TFT_ILI9163C.h>
#include <storage.h>

struct wifiAP_t {
    String ssid, password;
};

struct settings_t {
    struct mqtt {
        String server, username, password, caCertPath;
        uint16_t port, sendPeriod;
        char* ca_cert;
    } mqtt;
    std::vector<wifiAP_t> wifiAPs;
    struct logger {
        String outputPath;
        uint16_t writePeriod;
    } logger;
    struct dateTime {
        String server;
        long gmtOffset;
        int daylightOffset;
    } dateTime;
};

extern const char* SETTINGS_FILE;

class Settings {
    private:
        Storage *m_storage;
        settings_t m_settings;
        bool m_settingsOK;
        TFT_ILI9163C *m_tft;

        bool readSettings();
        String createJson();
        void defaultSettings();

    public:
        Settings(Storage *storage, TFT_ILI9163C *tft);

        bool begin();
        bool isSettingsOK();
        settings_t getSettings();
        bool saveSettings();

        void addWifiAP(const char* ssid, const char* password);
        bool updWifiAP(const char* ssid, const char* password);
        bool delWifiAP(const char* ssid);
        bool ssidExists(String ssid);

        void setMQTTValues(String server, String username, String password, uint16_t port, uint16_t sendPeriod);
        void setMQTTValues(String server, String username, uint16_t port, uint16_t sendPeriod);
        bool setMQTTCertificate(String certData);

        void setLoggerValues(uint16_t writePeriod);

        void setDateValues(String server, long gmtOffset, int daylightOffset);
};

#endif
