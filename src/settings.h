#ifndef settings_h
#define settings_h

#include <vector>
#include <ArduinoJson.h>
#include <TFT_ILI9163C.h>
#include <storage.h>

class Settings {
    private:
        const char* SETTINGS_FILE = "/meteo_settings.json";

        struct wifiAP {
            String ssid, password;
        };

        struct settings {
            struct mqtt {
                String server, username, password, caCertPath;
                uint16_t port, sendPeriod;
                char* ca_cert;
            } mqtt;
            std::vector<wifiAP> wifiAPs;
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

        Storage *m_storage;
        settings m_settings;
        bool m_settingsOK;
        TFT_ILI9163C *m_tft;

        bool readSettings();

    public:
        Settings(Storage *storage, TFT_ILI9163C *tft);

        bool begin();
        bool isSettingsOK();
        settings getSettings();
};

#endif
