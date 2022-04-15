#ifndef wifi_connection_h
#define wifi_connection_h

#include <WiFiMulti.h>
#include <settings.h>

class WiFiConnection {
    private:
        WiFiMulti *m_wifiMulti;
        Settings *m_settings;
        TFT_ILI9163C *m_tft;

    public:
        WiFiConnection(Settings *settings, TFT_ILI9163C *tft);

        bool begin();
        bool connect();
        bool isConnected();
        String getIP();
};

#endif
