#ifndef wifi_connection_h
#define wifi_connection_h

#include <TFT_ILI9163C.h>
#include <WiFiMulti.h>
#include <settings.h>

class WiFiConnection {
    private:
        const char *SSID_AP = "meteo_station";

        WiFiMulti *m_wifiMulti;
        Settings *m_settings;
        TFT_ILI9163C *m_tft;

    public:
        WiFiConnection(Settings *settings, TFT_ILI9163C *tft);

        bool begin();
        bool connect();
        bool beginAP();
        bool isConnected();
        bool isModeAP();
        String getIP();
};

#endif
