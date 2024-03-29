#include <data_logger.h>

DataLogger *dataLogger = nullptr;

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

String DataLogger::logSize() {
    return String(((float) m_storage->fileSize(m_filePath.c_str())) / 1024) + " kb";
}

//////////////////// Private methods implementation
String DataLogger::getLogData() {
    String ret = String(m_sensors->temp()) + "\t";
    ret += String(m_sensors->pres()) + "\t";
    ret += String(m_sensors->humi()) + "\n";

    return ret;
}

bool DataLogger::writeData() {
    if (!m_dateTime->refresh()) {
        return false;
    }

    String logTime = m_dateTime->toString() + "\t";
    String fullData = logTime + getLogData();
    Serial.print("log: " + fullData);

    bool res = m_storage->appendFile(m_filePath.c_str(), fullData.c_str());
    if (res)
        m_lastLogTime = logTime;
    
    return res;
}
