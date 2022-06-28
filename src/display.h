#ifndef display_h
#define display_h

#include <TFT_ILI9163C.h>

#include <date_time.h>
#include <number_set.h>
#include <sensors.h>
#include <settings.h>
#include <timer.h>
#include <wifi_connection.h>

class Display {
    private:
        TFT_ILI9163C* m_tft;
        uint16_t m_backgroundColor;
        uint16_t m_foreColor;
        Sensors* m_sensors;
        WiFiConnection* m_wifi;
        DateTime* m_dateTime;

        NumberSet *m_tempRep, *m_presHumiRep, *m_ddmmRep, *m_yearRep, *m_timeRep;
        NumberSet *m_presRep, *m_humiRep;
        bool m_showPres, m_apMode;
        Timer *m_tmrRefreshTime, *m_tmrShowPres;

        void drawScreen();
        void switchPresHumi();
        void drawPresHumi();
        void printDateTime();
        void printValues();

    public:
        Display(TFT_ILI9163C *tft, uint16_t backgroundColor, uint16_t foreColor, Sensors *sensors,
                WiFiConnection *wifi, DateTime *dateTime);

        bool begin();
        void loop();
};

#endif
