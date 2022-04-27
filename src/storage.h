#ifndef storage_h
#define storage_h

#include "FS.h"
#include "SD.h"
#include <TFT_ILI9163C.h>
#include <timer.h>

class Storage {
    private:
        FS *m_fs;
        uint8_t m_sdCsPin;
        TFT_ILI9163C *m_tft;

    public:
        Storage(uint8_t sdCsPin, TFT_ILI9163C *tft);

        bool begin();
        char* readAll(const char *path);
        bool writeFile(const char *path, const char *message);
        bool deleteFile(const char *path);
        FS* fs();
};

#endif
