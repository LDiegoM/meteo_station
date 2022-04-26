#ifndef date_time_h
#define date_time_h

#include <Arduino.h>
#include <time.h>

class DateTime {
    private:
        tm m_timeInfo;
        String m_year, m_month, m_day, m_hour, m_minutes, m_seconds;

        void setValues();

    public:
        DateTime(long gmtOffset_hours, int daylightOffset_hours, const char* server);

        bool refresh();
        String toString();
        String year();
        String month();
        String day();
        String hour();
        String minutes();
        String seconds();
};

#endif
