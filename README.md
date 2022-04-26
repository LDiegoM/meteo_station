# About

Meteo station is an experimental weather measuring station built with ESP32.

It was developed using PaltformIO with Visual Studio Code.

In [this document](./doc/README.md) you'll find the electronic details to assemble this project.

# Features

Meteo station uses 2 sensors (DHT11 and BMP180) to measure temperature, atmospheric pressure and humidity.

It also connects to an MQTT server to publish those masures to different topics.

Topics list:
- `topic-meteo-temp`: temperature value from DHT11.
- `topic-meteo-temp2`: temperature value from BMP180.
- `topic-meteo-pres`: pressure value from BMP180.
- `topic-meteo-humi`: humidity value from DHT11.

Meteo station logs measures in the microSD card, in the file `/meteo_data.txt` (defined in settings file).

Meteo station shows information in a 128x128 pixels display, in this format:

![Screen](./doc/Screen_design.png)

# Application settings

To configure Meteo station write a json file in a microSD card. Below is an example of settings file.

Name: `/meteo_settings.json`
```json
{
    "mqtt":{
        "server": "your_mqtt_server_dns_address",
        "port": 1234,
        "username": "your_awesome_mqtt_service_username",
        "password": "your_awesome_mqtt_service_password",
        "crt_path": "/mqtt_ca_root.crt",
        "send_period_seconds": 3600
    },
    "wifi":[
        {
            "ssid": "wifi_ap1_ssid",
            "password": "wifi_ap1_password"
        },
        {
            "ssid": "wifi_ap2_ssid",
            "password": "wifi_ap2_password"
        }
    ],
    "storage":{
        "output_path": "/meteo_data.txt",
        "write_period_seconds": 1800
    },
    "date_time":{
        "server": "pool.ntp.org",
        "gmt_offset": -3,
        "daylight_offset": 0
    }
}
```

Meteo station listens from the topic `topic-meteo-cmd` to receive different commands using the following json format:
```json
{
    "cmd": "command-type",
    "value": "some-value"
}
```

Theese are the available commands:
- `RESEND`: when this command is received (value fueld doesn't matters), the aplication will send current measures to MQTT topics.
- `GET_IP`: value field doesn't matters. The application will publish current local IP to topic `topic-meteo-res-ip`.
- `SET_AP_SSID`: value field should be the ssid of the wifi AP to add to settings. The application will publish `OK` or `ERROR: message` to the topic `topic-meteo-res-ap-ssid`. SSID should not be empty nor currently present in settings-
- `SET_AP_PASS`: value field should be the password of the wifi AP to add to settings. The application will publish `OK` or `ERROR: message` to the topic `topic-meteo-res-ap-pass`. Password could be empty.
- `SET_AP_SAVE`: value field doesn't matters. The application will save the new wifi AP with given ssid and password using `SET_AP_SSID` and `SET_AP_PASS` commands, and publish `OK` or `ERROR: message` to the topic `topic-meteo-res-ap-save`.

# MQTT service

I choose [Hime MQ](https://www.hivemq.com/) MQTT service for testing porpuses. [Here](./doc/mqtt_ca_root.crt) you'll find the root ca certificate sor a secure connection to this service.

# MQTT monitoring

For monitoring porpuses I configured a complet IoT dashboard using [Iot MQTT Panel](https://play.google.com/store/apps/details?id=snr.lab.iotmqttpanel.prod) application for Android.

[Here](./doc/IoTMQTTPanel.json) I let a JSON file to import in IoT MQTT Panel application to built the default dashboards.

# Author

- Main idea, development and functional prototype by Diego M. Lopez (ldiegom@gmail.com)

# Changelog

## Unreleased

- Log weather measures to SD card.

## 0.0.4 - 2022-04-15

- Add commands to return local IP and create new wifi AP.

## 0.0.3 - 2022-04-14

- Refactor mqtt and wifi abstractions.

## 0.0.2 - 2022-04-14

- Add feature to create new wifi AP in settings and save it.

## 0.0.1 - 2022-04-11

- First code revision of the application.
