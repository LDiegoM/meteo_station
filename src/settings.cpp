#include <settings.h>

//////////////////// Constructor
Settings::Settings(Storage *storage, TFT_ILI9163C *tft) {
    m_storage = storage;
    m_settingsOK = false;
    m_tft = tft;
}

//////////////////// Public methods implementation
bool Settings::begin() {
    m_tft->setCursor(2, m_tft->getCursorY() + 20);
    m_tft->print("Read settings");

    m_settingsOK = readSettings();
    if (!m_settingsOK)
        return false;
    
    m_tft->setCursor(2, m_tft->getCursorY() + 20);
    m_tft->print("Settings OK");
    delay(1000);

    return true;
}

bool Settings::isSettingsOK() {
    return m_settingsOK;
}

Settings::settings Settings::getSettings() {
    return m_settings;
}

//////////////////// Private methods implementation
bool Settings::readSettings() {
    StaticJsonDocument<1024> configs;
    char *settingsJson = m_storage->readAll(SETTINGS_FILE);
    DeserializationError error = deserializeJson(configs, settingsJson);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        m_tft->setCursor(2, m_tft->getCursorY() + 20);
        m_tft->print("Bad JSON");
        return false;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    m_settings.mqtt.server = jsonObj["mqtt"]["server"].as<String>();
    m_settings.mqtt.port = jsonObj["mqtt"]["port"].as<uint16_t>();
    m_settings.mqtt.username = jsonObj["mqtt"]["username"].as<String>();
    m_settings.mqtt.password = jsonObj["mqtt"]["password"].as<String>();
    m_settings.mqtt.caCertPath = jsonObj["mqtt"]["crt_path"].as<String>();
    m_settings.mqtt.sendPeriod = jsonObj["mqtt"]["send_period_seconds"].as<uint16_t>();
    m_settings.mqtt.ca_cert = m_storage->readAll(m_settings.mqtt.caCertPath.c_str());

    m_settings.wifiAPs.clear();
    for (int i = 0; i < jsonObj["wifi"].size(); i++) {
        wifiAP wifi;
        wifi.ssid = jsonObj["wifi"][i]["ssid"].as<String>();
        wifi.password = jsonObj["wifi"][i]["password"].as<String>();

        m_settings.wifiAPs.push_back(wifi);
    }

    m_settings.storage.outputPath = jsonObj["storage"]["output_path"].as<String>();
    m_settings.storage.writePeriod = jsonObj["storage"]["write_period_seconds"].as<uint16_t>();

    m_settings.dateTime.server = jsonObj["date_time"]["server"].as<String>();
    m_settings.dateTime.gmtOffset = jsonObj["date_time"]["gmt_offset"].as<long>();
    m_settings.dateTime.daylightOffset = jsonObj["date_time"]["daylight_offset"].as<int>();

    // TODO: Validate parameters
    return true;
}
