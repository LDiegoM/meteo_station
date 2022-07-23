#include <storage.h>

//////////////////// Constructor
Storage::Storage(TFT_ILI9163C *tft) {
    m_tft = tft;
}

//////////////////// Public methods implementation
bool Storage::begin() {
    Timer *timeOut = new Timer(10 * 1000);
    timeOut->start();

    m_tft->setCursor(2, m_tft->getCursorY() + 20);
    m_tft->print("Mounting FS");

    // Initialize SD card
    while(!LittleFS.begin()) {
        if (timeOut->isTime()) {
            m_tft->setCursor(2, m_tft->getCursorY() + 20);
            m_tft->print("Storage err");
            timeOut->stop();
            free(timeOut);
            delay(1000);
            return false;
        }
    }

    m_tft->setCursor(2, m_tft->getCursorY() + 20);
    m_tft->print("FS mount OK");
    timeOut->stop();
    free(timeOut);
    delay(1000);
    return true;
}

String Storage::readAll(const char *path) {
    File file;
#ifdef ESP8266
    file = LittleFS.open(path, "r");
#else
    file = LittleFS.open(path, FILE_READ);
#endif
    if (!file) {
        Serial.println("Failed to open file for read");
        return "";
    }
    delay(100);

    return file.readString();
}

// Write to the SD card
bool Storage::writeFile(const char *path, const char *message) {
    File file;
#ifdef ESP8266
    file = LittleFS.open(path, "w");
#else
    file = LittleFS.open(path, FILE_WRITE, true);
#endif
    if (!file) {
        Serial.println("Failed to open file for write");
        return false;
    }
    delay(100);

    bool flgOk = false;
    if (file.print(message) > 0)
        flgOk = true;

    file.close();

    return flgOk;
}

bool Storage::appendFile(const char *path, const char *message) {
    File file;
#ifdef ESP8266
    file = LittleFS.open(path, "a");
#else
    file = LittleFS.open(path, FILE_APPEND, true);
#endif
    if (!file) {
        Serial.println("Failed to open file for append");
        return false;
    }
    delay(100);

    size_t prevSize = file.size();

    bool flgOk = false;
    if (file.print(message) > 0)
        flgOk = true;

    file.close();

    size_t newSize = fileSize(path);
    if (prevSize == newSize)
        flgOk = false;

    return flgOk;
}

bool Storage::remove(const char *path) {
    return LittleFS.remove(path);
}

size_t Storage::fileSize(const char *path) {
    size_t size = 0;

    File file = LittleFS.open(path, "r");
    if (!file) {
        Serial.println("Failed to open file to get size");
        return size;
    }
    delay(100);

    size = file.size();
    file.close();

    return size;
}

bool Storage::exists(const char *path) {
    return LittleFS.exists(path);
}

File Storage::open(const char *path) {
    return LittleFS.open(path, "r");
}
File Storage::open(const char *path, const char *mode) {
    return LittleFS.open(path, mode);
}

String Storage::getSize() {
#ifdef ESP8266
    FSInfo fsInfo;
    if (LittleFS.info(fsInfo)) {
        return String((float) fsInfo.totalBytes / 1024) + " kb";
    }
#else
    return String((float) LittleFS.totalBytes() / 1024) + " kb";
#endif
    return "0 kb";
}

String Storage::getUsed() {
#ifdef ESP8266
    FSInfo fsInfo;
    if (LittleFS.info(fsInfo)) {
        return String((float) fsInfo.usedBytes / 1024) + " kb";
    }
#else
    return String((float) LittleFS.usedBytes() / 1024) + " kb";
#endif
    return "0 kb";
}

String Storage::getFree() {
    size_t totalBytes = 0, usedBytes = 0;
#ifdef ESP8266
    FSInfo fsInfo;
    if (LittleFS.info(fsInfo)) {
        totalBytes = fsInfo.totalBytes;
        usedBytes = fsInfo.usedBytes;
    }
#else
    totalBytes = LittleFS.totalBytes();
    usedBytes = LittleFS.usedBytes();
#endif
    float freePercentage = (float) (totalBytes - usedBytes) / (float) totalBytes * 100;
    return String((float) (totalBytes - usedBytes) / 1024) + " kb (" + String(freePercentage) + "%)";
}
