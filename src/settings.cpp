#include <settings.h>

//////////////////// Constructor
Settings::Settings(Storage *storage) {
    m_storage = storage;
    m_settingsOK = false;
    readSettings();
}

//////////////////// Public methods implementation
bool Settings::isSettingsOK() {
    return m_settingsOK;
}

Settings::settings Settings::getSettings() {
    return m_settings;
}

//////////////////// Private methods implementation
void Settings::readSettings() {
    StaticJsonDocument<1024> configs;
    char *settingsJson = m_storage->readAll(SETTINGS_FILE);
    DeserializationError error = deserializeJson(configs, settingsJson);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
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
    m_settingsOK = true;
}
