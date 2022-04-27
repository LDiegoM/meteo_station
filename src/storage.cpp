#include <storage.h>

//////////////////// Constructor
Storage::Storage(uint8_t sdCsPin, TFT_ILI9163C *tft) {
    m_sdCsPin = sdCsPin;
    m_tft = tft;
}

//////////////////// Public methods implementation
bool Storage::begin() {
    Timer *timeOut = new Timer(10 * 1000);
    timeOut->start();

    m_tft->setCursor(2, m_tft->getCursorY() + 20);
    m_tft->print("Mounting SD");

    // Initialize SD card
    while(!SD.begin(m_sdCsPin)) {
        if (timeOut->isTime()) {
            m_tft->setCursor(2, m_tft->getCursorY() + 20);
            m_tft->print("Insert SD");
            timeOut->stop();
            free(timeOut);
            delay(1000);
            return false;
        }
    }
    if (SD.cardType() == CARD_NONE) {
        m_tft->setCursor(2, m_tft->getCursorY() + 20);
        m_tft->print("Insert SD");
        timeOut->stop();
        free(timeOut);
        SD.end();
        delay(1000);
        return false;
    }

    m_tft->setCursor(2, m_tft->getCursorY() + 20);
    m_tft->print("SD mount OK");
    m_fs = &SD;
    timeOut->stop();
    free(timeOut);
    delay(1000);
    return true;
}

char* Storage::readAll(const char *path) {
    uint8_t* buffer;
    buffer = (uint8_t*)malloc(1);  // Allocate memory for the file and a terminating null char.
    buffer[0] = '\0';               // Add the terminating null char.

    File file = m_fs->open(path, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
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
    File file = m_fs->open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    delay(100);

    bool flgOk = false;
    if (file.print(message) > 0)
        flgOk = true;

    file.close();

    return flgOk;
}

bool Storage::deleteFile(const char *path) {
    return m_fs->remove(path);
}

FS* Storage::fs() {
    return m_fs;
}
