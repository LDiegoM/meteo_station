/*
    This is an implementation of MQTT queues for meteo station.

    Diego M. Lopez - 2021 (ldiegom@gmail.com)
*/

#ifndef mqtt_h
#define mqtt_h

#include <TFT_ILI9163C.h>
#include <wifi_connection.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <timer.h>
#include <sensors.h>
#include <settings.h>
#include <data_logger.h>

class MQTT {
    private:
        WiFiConnection *m_wifi;
        Sensors *m_sensors;
        Settings *m_settings;
        TFT_ILI9163C *m_tft;
        DataLogger *m_dataLogger;

        WiFiClientSecure *m_secureClient;

        PubSubClient *m_mqttClient;

        bool m_connected;
        Timer *m_tmrSendValuesToMQTT, *m_tmrConnectMQTT;
        const char* MQTT_TOPIC_TEMP = "topic-meteo-temp";
        const char* MQTT_TOPIC_TEMP2 = "topic-meteo-temp2";
        const char* MQTT_TOPIC_PRES = "topic-meteo-pres";
        const char* MQTT_TOPIC_HUMI = "topic-meteo-humi";
        const char* MQTT_TOPIC_CMD = "topic-meteo-cmd";
        const char* MQTT_TOPIC_RES_IP = "topic-meteo-res-ip";
        const char* MQTT_TOPIC_RES_LOG = "topic-meteo-res-log";
        const char* MQTT_TOPIC_RES_AP_SSID = "topic-meteo-res-ap-ssid";
        const char* MQTT_TOPIC_RES_AP_PASS = "topic-meteo-res-ap-pass";
        const char* MQTT_TOPIC_RES_AP_SAVE = "topic-meteo-res-ap-save";

        MQTT_CALLBACK_SIGNATURE;

        struct command {
            String cmd, value;
        } m_command;

        struct wifiAP {
            String ssid, password;
        } m_new_wifiAP;

        void sendValuesToMQTT();
        String commandToJSON();
        bool jsonToCommand(String json);

    public:
        MQTT(WiFiConnection *wifi, Sensors *sensors, Settings *settings, TFT_ILI9163C *tft,
             DataLogger *dataLogger, MQTT_CALLBACK_SIGNATURE);
        
        bool begin();
        bool connect();
        bool isConnected();
        void loop();
        void processReceivedMessage(char* topic, uint8_t* payload, unsigned int length);
};

#endif
