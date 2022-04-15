#include <wifi_connection.h>

//////////////////// Constructor
WiFiConnection::WiFiConnection(Settings *settings, TFT_ILI9163C *tft) {
    m_settings = settings;
    m_tft = tft;

    m_wifiMulti = new WiFiMulti();
}

//////////////////// Public methods implementation
bool WiFiConnection::begin() {
    if (!m_settings->isSettingsOK())
        return false;

    for (int i = 0; i < m_settings->getSettings().wifiAPs.size(); i++) {
        m_wifiMulti->addAP(m_settings->getSettings().wifiAPs[i].ssid.c_str(),
                           m_settings->getSettings().wifiAPs[i].password.c_str());
    }

    return connect();
}

bool WiFiConnection::connect() {
    m_tft->fillScreen(BLACK);
    m_tft->setTextColor(WHITE);
    m_tft->setCursor(2, 20);

    m_tft->print("WiFi Connect");
    m_tft->setCursor(2, m_tft->getCursorY() + 20);
    
    if (m_wifiMulti->run() != WL_CONNECTED) {
        m_tft->print("FAIL");
        return false;
    }
    m_tft->print("OK: " + WiFi.SSID());
    delay(1000);
    return true;
}

bool WiFiConnection::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiConnection::getIP() {
    if (!isConnected())
        return "";
    
    return WiFi.localIP().toString();
}