#include <http_handlers.h>

HttpHandlers *httpHandlers = nullptr;

//////////////////// HTTP Handlers
void downloadLogs(void) {
    httpHandlers->handleDownloadLogs();
}

void deleteLogs(void) {
    if (httpHandlers->handleDeleteLogs())
        dataLogger->logData();
}

void getSettings() {
    httpHandlers->handleGetSettings();
}

void getBootstrapCSS() {
    httpHandlers->handleGetBootstrapCSS();
}

void getBootstrapJS() {
    httpHandlers->handleGetBootstrapJS();
}

void getNotFound() {
    httpHandlers->handleGetNotFound();
}

void getSettingsWiFi() {
    httpHandlers->handleGetSettingsWiFi();
}
void addSettingsWiFi(){
    httpHandlers->handleAddSettingsWiFi();
}
void updSettingsWiFi(){
    httpHandlers->handleUpdSettingsWiFi();
}
void delSettingsWiFi(){
    httpHandlers->handleDelSettingsWiFi();
}


//////////////////// Constructor
HttpHandlers::HttpHandlers(WiFiConnection *wifi, Storage *storage, Settings *settings, TFT_ILI9163C *tft) {
    m_wifi = wifi;
    m_storage = storage;
    m_settings = settings;
    m_tft = tft;
}

//////////////////// Public methods implementation
bool HttpHandlers::begin() {
    if (!m_settings->isSettingsOK())
        return false;

    m_server = new WebServer(METEO_HTTP_PORT);

    defineRoutes();

    m_server->begin();

    return true;
}

void HttpHandlers::loop() {
    m_server->handleClient();
}

/////////// HTTP Handlers
void HttpHandlers::handleDownloadLogs() {
    Serial.println("Starting logs download");
    if (!m_storage->exists(m_settings->getSettings().storage.outputPath.c_str())) {
        m_server->send(404, "text/plain", "not found");
        return;
    }

    Serial.println("File exists");

    File file = m_storage->open(m_settings->getSettings().storage.outputPath.c_str());
    if (!file) {
        m_server->send(500, "text/plain", "fail to open log file");
        return;
    }

    Serial.println("File opened - size: " + String(file.size()));
    String dataType = "application/octet-stream";

    Serial.println("Sending header");
    m_server->sendHeader("Content-Disposition", "inline; filename=meteo_logs.txt");

    Serial.println("Start file streaming");
    if (m_server->streamFile(file, dataType) != file.size())
        Serial.println("Sent different data length than expected");
    
    Serial.println("File close");
    file.close();
}

bool HttpHandlers::handleDeleteLogs() {
    Serial.println("Starting logs delete");
    if (!m_storage->exists(m_settings->getSettings().storage.outputPath.c_str())) {
        m_server->send(404, "text/plain", "not found");
        return false;
    }

    Serial.println("File exists");
    bool flgOK = m_storage->remove(m_settings->getSettings().storage.outputPath.c_str());
    if (flgOK)
        m_server->send(204);
    else
        m_server->send(500, "text/plain", "could not delete file");
    
    return flgOK;
}

void HttpHandlers::handleGetSettings() {
    if (!m_storage->exists(SETTINGS_FILE)) {
        m_server->send(404, "text/plain", "not found");
        return;
    }

    String content = m_storage->readAll(SETTINGS_FILE);
    m_server->send(200, "application/json", content);
}

void HttpHandlers::handleGetBootstrapCSS() {
    File file = m_storage->open("/wwwroot/bootstrap.min.css.gz");
    if (!file) {
        m_server->send(404);
        return;
    }
    m_server->streamFile(file, "text/css");
    file.close();
}

void HttpHandlers::handleGetBootstrapJS() {
    File file = LittleFS.open("/wwwroot/bootstrap.min.js.gz");
    if (!file) {
        m_server->send(404);
        return;
    }
    m_server->streamFile(file, "text/js");
    file.close();
}

void HttpHandlers::handleGetNotFound() {
    Serial.println("not found?");
    String html = m_storage->readAll("/wwwroot/error.html");
    html.replace("{error_description}", "Resource not found");
    Serial.println(html);
    m_server->send(404, "text/html", html);
}

void HttpHandlers::handleGetSettingsWiFi() {
    String html = getSettingsHeaderHTML("wifi");
    html += getSettingsWiFiHTML();
    html += getFooterHTML("settings", "wifi");
    m_server->send(200, "text/html", html);

}
void HttpHandlers::handleAddSettingsWiFi() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    wifiAP_t newWiFiAP = parseWiFiBody(body);
    if (newWiFiAP.ssid.equals("")) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    if (m_settings->ssidExists(newWiFiAP.ssid)) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_EXISTS);
        return;
    }

    m_settings->addWifiAP(newWiFiAP.ssid.c_str(), newWiFiAP.password.c_str());
    if (!m_settings->saveSettings()) {
        m_server->send(500, "text/plain", ERR_GENERIC);
        return;        
    }

    m_server->send(200, "text/plain", MSG_OK);
}
void HttpHandlers::handleUpdSettingsWiFi() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    std::vector<wifiAP_t> aps = parseMultiWiFiBody(body);
    if (aps.size() < 1) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    for (int i = 0; i < aps.size(); i++) {
        if (!m_settings->updWifiAP(aps[i].ssid.c_str(), aps[i].password.c_str())) {
            m_server->send(404, "text/plain", ERR_WIFI_AP_NOT_FOUND);
            return;
        }
    }

    if (!m_settings->saveSettings()) {
        m_server->send(500, "text/plain", ERR_GENERIC);
        return;        
    }

    m_server->send(200, "text/plain", MSG_OK);
}
void HttpHandlers::handleDelSettingsWiFi() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    wifiAP_t newWiFiAP = parseWiFiBody(body);
    if (newWiFiAP.ssid.equals("")) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    if (!m_settings->ssidExists(newWiFiAP.ssid)) {
        m_server->send(404, "text/plain", ERR_WIFI_AP_NOT_FOUND);
        return;
    }

    m_settings->delWifiAP(newWiFiAP.ssid.c_str());
    if (!m_settings->saveSettings()) {
        m_server->send(500, "text/plain", ERR_GENERIC);
        return;        
    }

    m_server->send(200, "text/plain", MSG_OK);
}

//////////////////// Private methods implementation
void HttpHandlers::defineRoutes() {
    m_server->on("/logs", HTTP_GET, downloadLogs);
    m_server->on("/logs", HTTP_DELETE, deleteLogs);

    // TODO: When wifi settings is fully implemented, redirect to wifi settings handler.
    m_server->on("/settings", HTTP_GET, getSettings);

    m_server->on("/bootstrap.min.css", HTTP_GET, getBootstrapCSS);
    m_server->on("/bootstrap.min.js", HTTP_GET, getBootstrapJS);

    m_server->on("/settings/wifi", HTTP_GET, getSettingsWiFi);
    m_server->on("/settings/wifi", HTTP_POST, addSettingsWiFi);
    m_server->on("/settings/wifi", HTTP_PUT, updSettingsWiFi);
    m_server->on("/settings/wifi", HTTP_DELETE, delSettingsWiFi);

    m_server->onNotFound(getNotFound);
}

String HttpHandlers::getSettingsHeaderHTML(String section) {
    String header = m_storage->readAll("/wwwroot/settings/header.html");
    Serial.println(header);

    header.replace("{active_wifi}", (section.equals("wifi") ? "active" : ""));
    header.replace("{active_mqtt}", (section.equals("mqtt") ? "active" : ""));
    header.replace("{active_logger}", (section.equals("logger") ? "active" : ""));
    header.replace("{active_date}", (section.equals("date") ? "active" : ""));

    return header;
}

String HttpHandlers::getFooterHTML(String page, String section) {
    String footer = m_storage->readAll("/wwwroot/footer.html");
    String js = "";
    if (!page.equals("")) {
        js += "<script>";
        js += m_storage->readAll((String("/wwwroot/") + page + "/utils.js").c_str());
        js += "\n";
        js += m_storage->readAll((String("/wwwroot/") + page + "/" + section + ".js").c_str());
        js += "</script>";
    }
    footer.replace("<!--{utils.js}-->", js);

    return footer;
}

String HttpHandlers::getSettingsWiFiHTML() {
    String htmlUpdate = "";
    for (int i = 0; i < m_settings->getSettings().wifiAPs.size(); i++) {
        String htmlAP = m_storage->readAll("/wwwroot/settings/wifi_update_ap.html");
        htmlAP.replace("{ap_name}", m_settings->getSettings().wifiAPs[i].ssid);
        htmlAP += "\n";
        htmlUpdate += htmlAP;
    }

    String html = m_storage->readAll("/wwwroot/settings/wifi.html");
    html.replace("<!--{wifi_update_ap.html}-->", htmlUpdate);
    return html;
}

wifiAP_t HttpHandlers::parseWiFiBody(String body) {
    wifiAP_t newWiFiAP;

    StaticJsonDocument<250> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return newWiFiAP;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    newWiFiAP.ssid = jsonObj["ap"].as<String>();
    newWiFiAP.password = jsonObj["pw"].as<String>();

    return newWiFiAP;
}

std::vector<wifiAP_t> HttpHandlers::parseMultiWiFiBody(String body) {
    std::vector<wifiAP_t> aps;

    StaticJsonDocument<250> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return aps;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    for (int i = 0; i < jsonObj["aps"].size(); i++) {
        wifiAP_t ap;
        ap.ssid = jsonObj["aps"][i]["ap"].as<String>();
        ap.password = jsonObj["aps"][i]["pw"].as<String>();

        aps.push_back(ap);
    }

    return aps;
}
