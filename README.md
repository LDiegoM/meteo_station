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

WIP: Meteo station future version will log measures in the microSD card, in the file `/meteo_data.txt`.

Meteo staging listen from the topic `topic-meteo-cmd` to receive the following commands:
- `RESEND`: when this command is received, tha pplication will send current measures to MQTT topics.

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
        {"ssid": "wifi_ap1_ssid", "password": "wifi_ap1_password"},
        {"ssid": "wifi_ap2_ssid", "password": "wifi_ap2_password"}
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

# Authors

- Main idea, development and functional prototype by Diego M. Lopez (ldiegom@gmail.com)

# Changelog

## 0.0.1 - 2022-04-11

- First code revision of the application.
