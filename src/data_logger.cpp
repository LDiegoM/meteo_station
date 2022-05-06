#include <data_logger.h>

//////////////////// Constructor
DataLogger::DataLogger(Sensors *sensors, DateTime *dateTime, Storage *storage, String filePath, uint16_t logPeriod_sec) {
    m_sensors = sensors;
    m_dateTime = dateTime;
    m_storage = storage;
    m_filePath = filePath;
    m_tmrLog = new Timer(logPeriod_sec * 1000);
    m_lastLogTime = "0000-00-00 00:00:00";
}

//////////////////// Public methods implementation
void DataLogger::start() {
    m_tmrLog->start();
}

void DataLogger::loop() {
    if (!m_tmrLog->isRunning())
        m_tmrLog->start();
    
    if (m_tmrLog->isTime())
        writeData();
}

void DataLogger::logData() {
    writeData();
}

String DataLogger::getLastLogTime() {
    return m_lastLogTime;
}

float DataLogger::logSize() {
    return ((float) m_storage->fileSize(m_filePath.c_str())) / 1024;
}

//////////////////// Private methods implementation
String DataLogger::getLogData() {
    String ret = String(m_sensors->temp()) + "\t";
    ret += String(m_sensors->pres()) + "\t";
    ret += String(m_sensors->humi()) + "\n";

    return ret;
}

bool DataLogger::writeData() {
    String logTime = "";
    if (m_dateTime->refresh()) {
        logTime = m_dateTime->toString() + "\t";
    } else {
        return false;
    }

    String fullData = logTime +  getLogData();
    Serial.print("log: " + fullData);

    bool res = m_storage->writeFile(m_filePath.c_str(), fullData.c_str());
    if (res)
        m_lastLogTime = logTime;
    
    return res;
}
