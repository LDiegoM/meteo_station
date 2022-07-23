#include <sensors.h>

//////////////////// Constructor
Sensors::Sensors(uint8_t dhtPin, uint8_t dhtType, TFT_ILI9163C *tft) {
    m_dhtPin = dhtPin;
    m_dhtType = dhtType;
    m_updated = false;
    m_tft = tft;

    m_tempValue = 17.5;
    m_tempValue2 = 19.4;
    m_humiValue = 45.4;
    m_presValue = 1013;

    m_dhtSensor = nullptr; // new DHT(m_dhtPin, m_dhtType);
    m_bmpSensor = nullptr; // new Adafruit_BMP085();

    m_tmrRefreshValues = new Timer(REFRESH_TIME_MILLIS);
}

//////////////////// Public methods implementation
/*bool Sensors::begin() {
    Timer *timeOut = new Timer(10 * 1000);
    timeOut->start();

    m_tft->setCursor(2, m_tft->getCursorY() + 20);
    m_tft->print("Start sensor");

    m_dhtSensor->begin();
    while (!m_bmpSensor->begin()) {
        if (timeOut->isTime()) {
            m_tft->setCursor(2, m_tft->getCursorY() + 20);
            m_tft->print("Sensor error");
            timeOut->stop();
            free(timeOut);
            delay(1000);
            return false;
        }
    }

    m_tft->setCursor(2, m_tft->getCursorY() + 20);
    m_tft->print("Sensor OK");
    timeOut->stop();
    free(timeOut);
    delay(1000);

    // Do a first value read
    readValues();

    startCycle();

    return true;
}
*/

bool Sensors::begin() {
    m_tft->setCursor(2, m_tft->getCursorY() + 20);
    m_tft->print("Start sensor");

    m_tft->setCursor(2, m_tft->getCursorY() + 20);
    m_tft->print("Sensor OK");
    delay(1000);

    // Do a first value read
    // readValues();
    m_updated = true;

    startCycle();

    return true;
}

void Sensors::startCycle() {
    m_tmrRefreshValues->start();
}

void Sensors::stopCycle() {
    m_tmrRefreshValues->stop();
}

void Sensors::loop() {
    if (!m_tmrRefreshValues->isRunning()) {
        return;
    }

    if (m_tmrRefreshValues->isTime()) {
        //readValues();
        m_updated = true;
    }
}

bool Sensors::updated() {
    return m_updated;
}

float Sensors::temp() {
    m_updated = false;
    return m_tempValue;
}

float Sensors::temp2() {
    m_updated = false;
    return m_tempValue2;
}

float Sensors::pres() {
    m_updated = false;
    return m_presValue;
}

float Sensors::humi() {
    m_updated = false;
    return m_humiValue;
}

//////////////////// Private methods implementation
void Sensors::readValues() {
    m_tempValue = m_dhtSensor->readTemperature();
    if (isnan(m_tempValue))
        m_tempValue = 0;

    m_tempValue2 = m_bmpSensor->readTemperature();
    m_presValue = (float)m_bmpSensor->readPressure() / 100;
    
    m_humiValue = m_dhtSensor->readHumidity();
    if (isnan(m_humiValue))
        m_humiValue = 0;

    m_updated = true;
}
