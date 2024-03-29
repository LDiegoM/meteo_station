#include <mqtt_handlers.h>

MqttHandlers *mqtt = nullptr;

//////////////////// MQTT Handlers
void mqttMessageReceived(char* topic, uint8_t* payload, unsigned int length) {
    mqtt->processReceivedMessage(topic, payload, length);
}

//////////////////// Constructor
MqttHandlers::MqttHandlers(WiFiConnection *wifi, Sensors *sensors, Settings *settings, TFT_ILI9163C *tft,
           DataLogger *dataLogger, Storage *storage) {
    m_wifi = wifi;
    m_sensors = sensors;
    m_settings = settings;
    m_tft = tft;
    m_dataLogger = dataLogger;
    m_storage = storage;
    m_connected = false;
}

//////////////////// Public methods implementation
bool MqttHandlers::begin() {
    if (!m_settings->isSettingsOK() || m_wifi->isModeAP())
        return false;

    m_secureClient = new WiFiClientSecure();
    m_secureClient->setCACert(m_settings->getSettings().mqtt.ca_cert);

    m_mqttClient = new PubSubClient(*m_secureClient);
    m_mqttClient->setCallback(mqttMessageReceived);

    m_tmrConnectMQTT = new Timer(5000);
    m_tmrSendValuesToMQTT = new Timer(m_settings->getSettings().mqtt.sendPeriod * 1000);
    m_tmrSendValuesToMQTT->start();

    return connect(true);
}

bool MqttHandlers::connect(bool verbose) {
    m_connected = false;
    if (!m_settings->isSettingsOK() || m_wifi->isModeAP())
        return false;

    if (m_mqttClient->connected()) {
        m_connected = true;
        return true;
    }
    
    // If wifi is not connected, try to connect
    if (!m_wifi->isConnected()) {
        if (!m_wifi->connect(verbose))
            return false;
    }

    if (verbose) {
        m_tft->fillScreen(BLACK);
        m_tft->setTextColor(WHITE);
        m_tft->setCursor(2, 20);

        m_tft->print("MQTT Connect");
        m_tft->setCursor(2, m_tft->getCursorY() + 20);
    }

    String clientID = "ESP32-device_" + WiFi.localIP().toString();
    m_mqttClient->setServer(m_settings->getSettings().mqtt.server.c_str(), m_settings->getSettings().mqtt.port);
    if (!m_mqttClient->connect(clientID.c_str(),
                               m_settings->getSettings().mqtt.username.c_str(),
                               m_settings->getSettings().mqtt.password.c_str())) {
        if (verbose) {
            m_tft->print("FAIL");
            delay(1000);
        }

        return false;
    }
    m_mqttClient->subscribe(MQTT_TOPIC_CMD);
    m_tmrConnectMQTT->stop();
    m_connected = true;

    if (verbose) {
        sendValuesToMQTT();
        m_tft->print("OK");
        delay(1000);
    }

    return true;
}

bool MqttHandlers::isConnected() {
    return m_connected;
}

void MqttHandlers::loop() {
    if (!m_settings->isSettingsOK() || m_wifi->isModeAP())
        return;

    if (!m_mqttClient->connected()) {
        m_connected = false;

        if (!m_tmrConnectMQTT->isRunning()) {
            m_tmrConnectMQTT->start();
        }

        if (m_tmrConnectMQTT->isTime()) {
            Serial.println("MQTT not connected. Reconnecting");
            if (!connect(false)) {
                return;
            }
        } else {
            return;
        }
    }

    if (m_tmrSendValuesToMQTT->isTime()) {
        sendValuesToMQTT();
    }

    m_mqttClient->loop();
}

void MqttHandlers::processReceivedMessage(char* topic, uint8_t* payload, unsigned int length) {
    Serial.println("Message received from topic " + String(topic) + " - length: " + String(length));
    if (!String(topic).equals(MQTT_TOPIC_CMD))
        return;
    
    String incomingMessage = "";
    for (int i = 0; i < length; i++)
        incomingMessage += (char)payload[i];
    
    Serial.println("incomingMessage: " + incomingMessage);
    if (!jsonToCommand(incomingMessage)) {
        return;
    }

    if (m_command.cmd.equals("RESEND")) {
        sendValuesToMQTT();
    } else if (m_command.cmd.equals("GET_IP")) {
        m_mqttClient->publish(MQTT_TOPIC_RES_IP, m_wifi->getIP().c_str(), false);
    } else if (m_command.cmd.equals("GET_LOG")) {
        m_mqttClient->publish(MQTT_TOPIC_RES_LOG, m_dataLogger->getLastLogTime().c_str(), false);
    } else if (m_command.cmd.equals("GET_LOG_SIZE")) {
        m_mqttClient->publish(MQTT_TOPIC_RES_LOGSIZE, String(m_dataLogger->logSize()).c_str(), false);
    } else if (m_command.cmd.equals("GET_STO_FREE")) {
        m_mqttClient->publish(MQTT_TOPIC_RES_FREESTO, (m_storage->getFree() + " of " + m_storage->getSize()).c_str(), false);
    } else if (m_command.cmd.equals("SET_AP_SSID")) {
        if (m_command.value.equals("")) {
            m_new_wifiAP.ssid = "";
            m_mqttClient->publish(MQTT_TOPIC_RES_AP_SSID, "ERROR: empty ssid", false);
        } else {
            if (m_settings->ssidExists(m_command.value)) {
                m_new_wifiAP.ssid = "";
                m_mqttClient->publish(MQTT_TOPIC_RES_AP_SSID, "ERROR: ssid exists", false);
            } else {
                m_new_wifiAP.ssid = m_command.value;
                m_mqttClient->publish(MQTT_TOPIC_RES_AP_SSID, "OK", false);
            }
        }
    } else if (m_command.cmd.equals("SET_AP_PASS")) {
        m_new_wifiAP.password = m_command.value;
        m_mqttClient->publish(MQTT_TOPIC_RES_AP_PASS, "OK", false);
    } else if (m_command.cmd.equals("SET_AP_SAVE")) {
        if (m_new_wifiAP.ssid.equals(""))
            m_mqttClient->publish(MQTT_TOPIC_RES_AP_SAVE, "ERROR: empty ssid", false);
        else {
            m_settings->addWifiAP(m_new_wifiAP.ssid.c_str(), m_new_wifiAP.password.c_str());
            if (m_settings->saveSettings())
                m_mqttClient->publish(MQTT_TOPIC_RES_AP_SAVE, "OK", false);
            else
                m_mqttClient->publish(MQTT_TOPIC_RES_AP_SAVE, "ERROR: fail to save", false);
            m_new_wifiAP.ssid = "";
            m_new_wifiAP.password = "";
        }
    }
}

//////////////////// Private methods implementation
void MqttHandlers::sendValuesToMQTT() {
    if (!m_settings->isSettingsOK())
        return;

    if (!m_mqttClient->connected()) {
        m_connected = false;
        return;
    }
    
    m_mqttClient->publish(MQTT_TOPIC_TEMP, String(m_sensors->temp()).c_str(), true);
    m_mqttClient->publish(MQTT_TOPIC_TEMP2, String(m_sensors->temp2()).c_str(), true);
    m_mqttClient->publish(MQTT_TOPIC_PRES, String(m_sensors->pres()).c_str(), true);
    m_mqttClient->publish(MQTT_TOPIC_HUMI, String(m_sensors->humi()).c_str(), true);
}

String MqttHandlers::commandToJSON() {
    StaticJsonDocument<200> doc;
    doc["cmd"] = m_command.cmd;
    doc["value"] = m_command.value;

    String json;
    serializeJsonPretty(doc, json);

    return json;
}

bool MqttHandlers::jsonToCommand(String json) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, json.c_str());
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return false;
    }

    JsonObject jsonObj = doc.as<JsonObject>();
    m_command.cmd = jsonObj["cmd"].as<String>();
    m_command.value = jsonObj["value"].as<String>();
    return true;
}
