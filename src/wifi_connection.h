#ifndef wifi_connection_h
#define wifi_connection_h

#include <TFT_ILI9163C.h>

#ifdef ESP8266
#include <ESP8266WiFiMulti.h>

#elif defined(ESP32)
#include <WiFiMulti.h>
#else
#error "Unsupported platform"
#endif

#include <settings.h>

class WiFiConnection {
    private:
        const char *SSID_AP = "meteo_station";

#ifdef ESP8266
        ESP8266WiFiMulti *m_wifiMulti;
#elif defined(ESP32)
        WiFiMulti *m_wifiMulti;
#endif
        Settings *m_settings;
        TFT_ILI9163C *m_tft;

    public:
        WiFiConnection(Settings *settings, TFT_ILI9163C *tft);

        bool begin();
        bool connect(bool verbose);
        bool beginAP();
        bool isConnected();
        bool isModeAP();
        String getIP();
        String getSSID();
};

#endif
