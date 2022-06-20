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

char* Storage::readAll(const char *path) {
    uint8_t* buffer;
    buffer = (uint8_t*)malloc(1);  // Allocate memory for the file and a terminating null char.
    buffer[0] = '\0';              // Add the terminating null char.

    File file = LittleFS.open(path, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for read");
        return (char*) buffer;
    }
    delay(100);

    unsigned int fileSize = file.size();     // Get the file size.
    buffer = (uint8_t*)malloc(fileSize + 1); // Allocate memory for the file and a terminating null char.
    file.read(buffer, fileSize);             // Read the file into the buffer.
    buffer[fileSize] = '\0';                 // Add the terminating null char.
    file.close();
    
    return (char*) buffer;
}

// Write to the SD card
bool Storage::writeFile(const char *path, const char *message) {
    File file = LittleFS.open(path, FILE_WRITE, true);
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
    File file = LittleFS.open(path, FILE_APPEND, true);
    if (!file) {
        Serial.println("Failed to open file for append");
        return false;
    }
    delay(100);

    bool flgOk = false;
    if (file.print(message) > 0)
        flgOk = true;

    file.close();

    return flgOk;
}

bool Storage::remove(const char *path) {
    return LittleFS.remove(path);
}

size_t Storage::fileSize(const char *path) {
    size_t size = 0;

    File file = LittleFS.open(path, FILE_READ);
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
    return LittleFS.open(path);
}
File Storage::open(const char *path, const char *mode, const bool create) {
    return LittleFS.open(path, mode, create);
}

String Storage::getSize() {
    return String((float) LittleFS.totalBytes() / 1024) + " kb";
}

String Storage::getUsed() {
    return String((float) LittleFS.usedBytes() / 1024) + " kb";

}

String Storage::getFree() {
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    float freePercentage = (float) (totalBytes - usedBytes) / (float) totalBytes * 100;
    return String((float) (totalBytes - usedBytes) / 1024) + " kb (" + String(freePercentage) + "%)";
}
