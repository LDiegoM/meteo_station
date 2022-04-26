#include <data_logger.h>

//////////////////// Constructor
DataLogger::DataLogger(Sensors *sensors, DateTime *dateTime, Storage *storage, String filePath, uint16_t logPeriod_sec) {
    m_sensors = sensors;
    m_dateTime = dateTime;
    m_storage = storage;
    m_filePath = filePath;
    m_tmrLog = new Timer(logPeriod_sec * 1000);
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

//////////////////// Private methods implementation
String DataLogger::getLogData() {
    String ret = "";

    if (m_dateTime->refresh()) {
        ret += m_dateTime->toString() + "\t";
    }

    ret += String(m_sensors->temp()) + "\t";
    ret += String(m_sensors->pres()) + "\t";
    ret += String(m_sensors->humi()) + "\n";

    Serial.print("log: " + ret);

    return ret;
}

void DataLogger::writeData() {
    m_storage->writeFile(m_filePath.c_str(), getLogData().c_str());
}
