#ifndef sensors_h
#define sensors_h

#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <TFT_ILI9163C.h>
#include <timer.h>

class Sensors {
    private:
        uint8_t m_dhtPin, m_dhtType;
        bool m_updated;

        float m_tempValue, m_tempValue2, m_humiValue, m_presValue;
        DHT *m_dhtSensor;
        Adafruit_BMP085 *m_bmpSensor;
        TFT_ILI9163C *m_tft;
        Timer *m_tmrRefreshValues;

        const uint16_t REFRESH_TIME_MILLIS = 500;

        void readValues();

    public:
        Sensors(uint8_t dhtPin, uint8_t dhtType, TFT_ILI9163C *tft);

        bool begin();
        void startCycle();
        void stopCycle();
        void loop();
        bool updated();
        float temp();
        float temp2();
        float pres();
        float humi();
};

#endif
