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
    struct storage {
        String outputPath;
        uint16_t writePeriod;
    } storage;
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

    public:
        Settings(Storage *storage, TFT_ILI9163C *tft);

        bool begin();
        bool isSettingsOK();
        settings_t getSettings();
        bool saveSettings();
        void addWifiAP(const char* ssid, const char* password);
        bool ssidExists(String ssid);
};

#endif
