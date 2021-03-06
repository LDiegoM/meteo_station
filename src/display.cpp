#include <display.h>

//////////////////// Constructor
Display::Display(TFT_ILI9163C *tft, uint16_t backgroundColor, uint16_t foreColor, Sensors *sensors,
                 WiFiConnection *wifi, DateTime *dateTime) {
    m_tft = tft;
    m_backgroundColor = backgroundColor;
    m_foreColor = foreColor;
    m_sensors = sensors;
    m_wifi = wifi;
    m_dateTime = dateTime;

    m_apMode = m_wifi->isModeAP();

    if (!m_apMode) {
        m_tempRep = new NumberSet(m_tft, 3, 20, NS_SFLT, 2, m_backgroundColor, m_foreColor);
        m_presHumiRep = new NumberSet(m_tft, 64 + 3, 20, NS_SFLT, 2, m_backgroundColor, m_foreColor);
        m_timeRep = new NumberSet(m_tft, (m_tft->width() - NS_HHMM_3_W) / 2, m_tempRep->y() + m_tempRep->height() + 15, NS_HHMM, 3, m_backgroundColor, m_foreColor);
        m_ddmmRep = new NumberSet(m_tft, m_tft->width() - (NS_UFLT_1_W * 2) - 3, m_tft->height() - NS_SIZE_1_H - 3, NS_UFLT, 1, m_backgroundColor, m_foreColor);
        m_yearRep = new NumberSet(m_tft, m_ddmmRep->x() + m_ddmmRep->width(), m_ddmmRep->y(), NS_UFLT, 1, m_backgroundColor, m_foreColor);
    } else {
        m_humiRep = new NumberSet(m_tft, 3, 20, NS_SFLT, 2, m_backgroundColor, m_foreColor);
        m_presRep = new NumberSet(m_tft, 64 + 3, 20, NS_SFLT, 2, m_backgroundColor, m_foreColor);
        m_tempRep = new NumberSet(m_tft, (m_tft->width() - NS_SFLT_3_W) / 2, m_humiRep->y() + m_humiRep->height() + 23, NS_SFLT, 3, m_backgroundColor, m_foreColor);
    }

    m_showPres = false;
    m_tmrRefreshTime = new Timer(1000);
    m_tmrShowPres = new Timer(10 * 1000);
}

//////////////////// Public methods implementation
bool Display::begin() {
    drawScreen();
    printDateTime();

    if (!m_apMode) {
        m_tmrRefreshTime->start();
        m_tmrShowPres->start();
    }
    return true;
}

void Display::loop() {
    if (m_tmrRefreshTime->isTime())
        printDateTime();

    if (m_tmrShowPres->isTime()) {
        switchPresHumi();
        drawPresHumi();
        printValues();
    }

    if (m_sensors->updated()) {
        printValues();
    }

    m_tempRep->refresh();
    if (!m_apMode) {
        m_presHumiRep->refresh();
        m_timeRep->refresh();
        m_ddmmRep->refresh();
        m_yearRep->refresh();
    } else {
        m_presRep->refresh();
        m_humiRep->refresh();
    }
}

//////////////////// Private methods implementation
void Display::drawScreen() {
    m_tft->fillScreen(m_backgroundColor);
    m_tft->setTextColor(m_foreColor);
    m_tft->setCursor(10, 16);

    if (!m_apMode) {
        m_tft->print("Temp");
        drawPresHumi();

        // Upper horizontal double line
        m_tft->drawRect(0, m_tempRep->y() + m_tempRep->height() + 3, m_tft->width(), 2, m_foreColor);
        // Upper vertical line
        m_tft->drawFastVLine(64, 0, m_tempRep->y() + m_tempRep->height() + 3, m_foreColor);
    } else {
        m_tft->print("Hum");
        m_tft->setCursor(64 + 10, 16);
        m_tft->print("Pres");
        m_tft->setCursor(10, m_tempRep->y() - 3);
        m_tft->print("Temp");

        // Upper horizontal double line
        m_tft->drawRect(0, m_humiRep->y() + m_humiRep->height() + 3, m_tft->width(), 2, m_foreColor);
        // Upper vertical line
        m_tft->drawFastVLine(64, 0, m_humiRep->y() + m_humiRep->height() + 3, m_foreColor);

        m_tft->setCursor(5, m_tft->height() - 3);
        m_tft->print("IP:" + m_wifi->getIP());
    }

    // Lower horizontal single line
    m_tft->drawRect(0, m_tft->height() - NS_SIZE_1_H - 3 - 3, m_tft->width(), 1, m_foreColor);
}

void Display::switchPresHumi() {
    m_showPres = !m_showPres;
}

void Display::drawPresHumi() {
    // First blank space where word will be written
    m_tft->fillRect(64+1, 0, m_tft->width(), 40, m_backgroundColor);

    m_tft->setCursor(64 + 10, 16);
    if (m_showPres)
        m_tft->print("Pres");
    else
        m_tft->print("Hum");
}

void Display::printDateTime() {
    if (!m_wifi->isConnected() || m_apMode)
        return;

    if(!m_dateTime->refresh()) {
        Serial.println("Failed to refresh time");
        return;
    }

    m_timeRep->setValue(m_dateTime->hour() + ":" + m_dateTime->minutes());
    m_ddmmRep->setValue(m_dateTime->day() + "." + m_dateTime->month());
    m_yearRep->setValue("." + m_dateTime->year());
}

void Display::printValues() {
    m_tempRep->setValue(m_sensors->temp());
    if (m_apMode) {
        m_presRep->setValue(m_sensors->pres());
        m_humiRep->setValue(m_sensors->humi());
    } else {
        if (m_showPres)
            m_presHumiRep->setValue(m_sensors->pres());
        else
            m_presHumiRep->setValue(m_sensors->humi());
    }
}
