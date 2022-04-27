/*
    Example: https://github.com/zhouhan0126/WebServer-esp32/blob/master/examples/SDWebServer/SDWebServer.ino


*/
#ifndef http_handlers_h
#define http_handlers_h

#include <TFT_ILI9163C.h>
#include <WebServer.h>
#include <wifi_connection.h>
#include <settings.h>
#include <storage.h>

class HttpHandlers {
    private:
        const uint16_t METEO_HTTP_PORT = 80;

        WiFiConnection *m_wifi;
        Storage *m_storage;
        Settings *m_settings;
        TFT_ILI9163C *m_tft;
        void (*m_handlerDownloadLogs)();
        void (*m_handlerDeleteLogs)();
        void (*m_handlerGetSettings)();

        WebServer *m_server;

        void defineRoutes();

    public:
        HttpHandlers(WiFiConnection *wifi, Storage *storage, Settings *settings, TFT_ILI9163C *tft,
                     void (*handlerDownloadLogs)(), void (*handlerDeleteLogs)(),
                     void (*handlerGetSettings)());

        bool begin();
        void loop();

        // HTTP handlers
        void handleDownloadLogs();
        bool handleDeleteLogs();
        void handleGetSettings();
};

#endif
