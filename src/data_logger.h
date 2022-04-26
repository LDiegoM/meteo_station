#ifndef data_logger_h
#define data_logger_h

#include <sensors.h>
#include <date_time.h>
#include <storage.h>
#include <timer.h>

class DataLogger {
    private:
        Sensors *m_sensors;
        DateTime *m_dateTime;
        Storage *m_storage;
        String m_filePath;
        Timer *m_tmrLog;
        String m_lastLogTime;

        String getLogData();
        bool writeData();
    
    public:
        DataLogger(Sensors *sensors, DateTime *dateTime, Storage *storage, String filePath, uint16_t logPeriod);

        void start();
        void loop();
        void logData();
        String getLastLogTime();
};

#endif
