#include <http_handlers.h>

//////////////////// Constructor
HttpHandlers::HttpHandlers(WiFiConnection *wifi, Storage *storage, Settings *settings, TFT_ILI9163C *tft,
                           void (*handlerDownloadLogs)(), void (*handlerDeleteLogs)(),
                           void (*handlerGetSettings)()) {
    m_wifi = wifi;
    m_storage = storage;
    m_settings = settings;
    m_tft = tft;
    m_handlerDownloadLogs = handlerDownloadLogs;
    m_handlerDeleteLogs = handlerDeleteLogs;
    m_handlerGetSettings = handlerGetSettings;
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

//////////////////// Private methods implementation
void HttpHandlers::defineRoutes() {
    m_server->on("/logs", HTTP_GET, m_handlerDownloadLogs);
    m_server->on("/logs", HTTP_DELETE, m_handlerDeleteLogs);
    m_server->on("/settings", HTTP_GET, m_handlerGetSettings);
}
