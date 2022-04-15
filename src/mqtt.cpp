#include <mqtt.h>

//////////////////// Constructor
MQTT::MQTT(WiFiConnection *wifi, Sensors *sensors, Settings *settings, TFT_ILI9163C *tft, MQTT_CALLBACK_SIGNATURE) {
    m_wifi = wifi;
    m_sensors = sensors;
    m_settings = settings;
    m_tft = tft;
    this->callback = callback;
    m_connected = false;
}

//////////////////// Public methods implementation
bool MQTT::begin() {
    if (!m_settings->isSettingsOK())
        return false;

    m_secureClient = new WiFiClientSecure();
    m_secureClient->setCACert(m_settings->getSettings().mqtt.ca_cert);

    m_mqttClient = new PubSubClient(*m_secureClient);
    m_mqttClient->setCallback(callback);

    m_tmrConnectMQTT = new Timer(5000);
    m_tmrSendValuesToMQTT = new Timer(m_settings->getSettings().mqtt.sendPeriod * 1000);
    m_tmrSendValuesToMQTT->start();

    return connect();
}

bool MQTT::connect() {
    m_connected = false;
    if (!m_settings->isSettingsOK())
        return false;

    if (m_mqttClient->connected()) {
        m_connected = true;
        return true;
    }
    
    // If wifi is not connected, try to connect
    if (!m_wifi->isConnected()) {
        if (!m_wifi->connect())
            return false;
    }
        
    m_tft->fillScreen(BLACK);
    m_tft->setTextColor(WHITE);
    m_tft->setCursor(2, 20);

    m_tft->print("MQTT Connect");
    m_tft->setCursor(2, m_tft->getCursorY() + 20);

    String clientID = "ESP32-device_" + WiFi.localIP().toString();
    m_mqttClient->setServer(m_settings->getSettings().mqtt.server.c_str(), m_settings->getSettings().mqtt.port);
    if (!m_mqttClient->connect(clientID.c_str(),
                               m_settings->getSettings().mqtt.username.c_str(),
                               m_settings->getSettings().mqtt.password.c_str())) {
        m_tft->print("FAIL");
        return false;
    }
    m_mqttClient->subscribe(MQTT_CMD_TOPIC);
    m_tft->print("OK");
    m_tmrConnectMQTT->stop();
    m_connected = true;
    delay(1000);

    sendValuesToMQTT();

    return true;
}

bool MQTT::isConnected() {
    return m_connected;
}

void MQTT::loop() {
    if (!m_settings->isSettingsOK())
        return;

    if (!m_mqttClient->connected()) {
        if (!m_tmrConnectMQTT->isRunning()) {
            m_tmrConnectMQTT->start();
        }

        if (m_tmrConnectMQTT->isTime()) {
            Serial.println("MQTT not connected. Reconnecting");
            if (!connect()) {
                return;
            }
        }
    } else {
        if (m_tmrSendValuesToMQTT->isTime()) {
            sendValuesToMQTT();
        }

        m_mqttClient->loop();
    }
}

void MQTT::processReceivedMessage(char* topic, uint8_t* payload, unsigned int length) {
    Serial.println("Message received from topic " + String(topic) + " - length: " + String(length));
    if (!String(topic).equals(MQTT_CMD_TOPIC))
        return;
    
    String incomingMessage = "";
    for (int i = 0; i < length; i++)
        incomingMessage += (char)payload[i];
    
    Serial.println("incomingMessage: " + incomingMessage);
    if (incomingMessage.equals("RESEND")) {
        sendValuesToMQTT();
    }
}

//////////////////// Private methods implementation
void MQTT::sendValuesToMQTT() {
    if (!m_settings->isSettingsOK())
        return;

    if (!m_mqttClient->connected()) {
        m_connected = false;
        return;
    }
    
    m_mqttClient->publish(MQTT_TEMP_TOPIC, String(m_sensors->temp()).c_str(), true);
    m_mqttClient->publish(MQTT_TEMP_TOPIC2, String(m_sensors->temp2()).c_str(), true);
    m_mqttClient->publish(MQTT_PRES_TOPIC, String(m_sensors->pres()).c_str(), true);
    m_mqttClient->publish(MQTT_HUMI_TOPIC, String(m_sensors->humi()).c_str(), true);
}
