#ifndef mqtt_h
#define mqtt_h

#include <TFT_ILI9163C.h>
#include <wifi_connection.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <timer.h>
#include <sensors.h>
#include <settings.h>

class MQTT {
    private:
        WiFiConnection *m_wifi;
        Sensors *m_sensors;
        Settings *m_settings;
        TFT_ILI9163C *m_tft;

        WiFiClientSecure *m_secureClient;

        PubSubClient *m_mqttClient;

        bool m_connected;
        Timer *m_tmrSendValuesToMQTT, *m_tmrConnectMQTT;
        const char* MQTT_TEMP_TOPIC = "topic-meteo-temp";
        const char* MQTT_TEMP_TOPIC2 = "topic-meteo-temp2";
        const char* MQTT_PRES_TOPIC = "topic-meteo-pres";
        const char* MQTT_HUMI_TOPIC = "topic-meteo-humi";
        const char* MQTT_CMD_TOPIC = "topic-meteo-cmd";

        MQTT_CALLBACK_SIGNATURE;

        void sendValuesToMQTT();

    public:
        MQTT(WiFiConnection *wifi, Sensors *sensors, Settings *settings, TFT_ILI9163C *tft, MQTT_CALLBACK_SIGNATURE);
        
        bool begin();
        bool connect();
        bool isConnected();
        void loop();
        void processReceivedMessage(char* topic, uint8_t* payload, unsigned int length);
};

#endif
