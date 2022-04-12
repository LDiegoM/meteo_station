#include <number_set.h>

//////////////////// Constructor
NumberSet::NumberSet(TFT_ILI9163C* tft, uint8_t x, uint8_t y, uint8_t format,
                     uint8_t size, uint16_t background, uint16_t fontColor) {
    this->m_tft = tft;
    this->m_x = x;
    this->m_y = y;

    this->m_format = NS_UINT;
    if (format == NS_HHMM || format == NS_UINT || format == NS_SINT || format == NS_UFLT || format == NS_SFLT)
        this->m_format = format;

    this->m_size = NS_SIZE_1;
    if (size >= 1 && size <= 3)
        this->m_size = size;

    this->m_background = background;
    this->m_fontColor = fontColor;
    this->m_unsigned = this->m_format == NS_HHMM || this->m_format == NS_UINT || this->m_format == NS_UFLT;

    this->m_signWidth = SIZES[this->m_size - 1].signWidth;
    this->m_numberHeight = SIZES[this->m_size - 1].numberHeight;
    this->m_numberWidth = SIZES[this->m_size - 1].numberWidth;

    this->resetChars();
    this->resetPrevChars();

    this->tmrReset = new Timer(3000);
    tmrReset->start();
}

//////////////////// Public methods implementation
uint8_t NumberSet::x() {
    return this->m_x;
}

uint8_t NumberSet::y() {
    return this->m_y;
}

uint8_t NumberSet::height() {
    return MARGIN_SIZE + this->m_numberHeight + MARGIN_SIZE;
}

uint8_t NumberSet::width() {
    uint8_t signWidth = 0;
    if (!this->m_unsigned)
        signWidth = this->m_signWidth + this->m_size; // Add the distance between chars

    uint8_t punctuationWidth = 0;
    if (this->m_format == NS_HHMM || this->m_format == NS_UFLT || this->m_format == NS_SFLT)
        punctuationWidth = this->m_size + this->m_size;

    return MARGIN_SIZE + signWidth + punctuationWidth + ((this->m_numberWidth + this->m_size) * 4) - this->m_size + MARGIN_SIZE;
}

void NumberSet::setValue(float number) {
    if (number < -9999 || number > 9999 || (number < 0 && this->m_unsigned)) {
        this->resetChars();
        return;
    }

    float absNumber = abs(number);
    String sNumber = "0000" + String(absNumber);
    uint8_t chars = 4;
    if (sNumber.lastIndexOf(".") != -1) {
        // For numbers between 100 and 999 truncate centesimal position
        if (absNumber >= 100 && absNumber <= 999) {
            sNumber = sNumber.substring(0, sNumber.length() - 1);
        }

        // For numbers greater or equal than 1000, remove decimal positions
        if (absNumber >= 1000) {
            sNumber = sNumber.substring(0, sNumber.lastIndexOf("."));
        } else {
            chars++;
        }
    }

    sNumber = sNumber.substring(sNumber.length() - chars);
    if (number < 0)
        sNumber = "-" + sNumber;
    this->setValue(sNumber);
}

void NumberSet::setValue(int16_t number) {
    if (number < -9999 || number > 9999 || (number < 0 && this->m_unsigned)) {
        this->resetChars();
        return;
    }

    uint16_t absNumber = abs(number);
    String sNumber = "0000" + String(absNumber);
    sNumber = sNumber.substring(sNumber.length() - 4);
    if (number < 0)
        sNumber = "-" + sNumber;
    this->setValue(sNumber);
}

void NumberSet::setValue(String number) {
    this->resetChars();
    if (!this->isValidNumber(number))
        return;

    String sNumber = number;
    if (this->m_format == NS_HHMM && number.length() == 4)
        sNumber = number.substring(0, 2) + ":" + number.substring(2, 4);

    for (unsigned int c = 0; c < sNumber.length(); c++) {
        this->m_chars[c] = sNumber[c];
    }
}

String NumberSet::getValue() {
    return String(this->m_chars);
}

void NumberSet::refresh() {
    if (!this->charsChanged() && !this->tmrReset->isTime())
        return;
    
    // For the first refresh, or after an error value, reset background
    if (this->m_prevChars[0] == 0x00U || this->tmrReset->isTime()) {
        // Serial.println("Reset background");
        this->m_tft->fillRect(this->m_x, this->m_y, this->width(), this->height(), this->m_background);
    }

    if (this->m_chars[0] == 0x00U) {
        // Print "Err" string
        this->printErr();
    } else {
        // Print chars to screen
        this->printChars();
    }

    this->setPreviousChars();
}

//////////////////// Private methods implementation
void NumberSet::resetChars() {
    for (uint8_t c = 0; c < 7; c++)
        this->m_chars[c] = 0x00U;
}

void NumberSet::resetPrevChars() {
    for (uint8_t c = 0; c < 7; c++)
        this->m_prevChars[c] = 0x00U;
}

bool NumberSet::isValidNumber(String number) {
    if (number.length() < 4 || number.length() > 6)
        return false;
    
    return std::regex_match(std::string(number.c_str()), REG_EXPR[this->m_format]);
}

bool NumberSet::charsChanged() {
    for (uint8_t c = 0; c < 7; c++) {
        if (this->m_prevChars[c] != this->m_chars[c])
            return true;
    }
    return false;
}

void NumberSet::setPreviousChars() {
    for (uint8_t c = 0; c < 7; c++)
        this->m_prevChars[c] = m_chars[c];
}

void NumberSet::printSign(uint16_t color) {
    uint8_t x = this->m_x + MARGIN_SIZE;
    uint8_t y = this->m_y + ((this->height() - this->m_size) / 2);
    switch (this->m_size) {
        case 1:
            this->m_tft->drawFastHLine(x, y, this->m_signWidth, color);
            break;
        case 2:
            this->m_tft->drawRect(x, y, this->m_signWidth, this->m_size, color);
            break;
        case 3:
            this->m_tft->fillRect(x + 1, y, this->m_signWidth - 2, this->m_size, color);
            this->m_tft->drawPixel(x, y + 1, color);
            this->m_tft->drawPixel(x + this->m_signWidth - 1, y + 1, color);
            break;
    }
}

void NumberSet::printChars() {
    uint8_t c = 0;

    uint8_t x = this->m_x + MARGIN_SIZE;
    if (!this->m_unsigned) {
        // Add sign width and it's margin
        x += this->m_signWidth + this->m_size;

        if (this->m_chars[c] == '-') {
            this->printSign(this->m_fontColor);
            c++;
        } else {
            this->printSign(this->m_background);
        }
    }

    while (c < 7) {
        if (this->m_chars[c] == 0x00U)
            break;

        if (this->m_chars[c] != ':' && this->m_chars[c] != '.') {
            this->printNumber((uint8_t)m_chars[c] - '0', x);
            x += this->m_numberWidth + this->m_size;
        } else {
            // Print punctuation
            this->printPunctuation(this->m_chars[c], x, this->m_fontColor);
            x += this->m_size * 2;
        }
        c++;
    }
}

void NumberSet::printNumber(uint8_t number, uint8_t x) {
    this->printSideA(x, (DIGITS[number][0] == 1 ? this->m_fontColor : this->m_background));
    this->printSideB(x, (DIGITS[number][1] == 1 ? this->m_fontColor : this->m_background));
    this->printSideC(x, (DIGITS[number][2] == 1 ? this->m_fontColor : this->m_background));
    this->printSideD(x, (DIGITS[number][3] == 1 ? this->m_fontColor : this->m_background));
    this->printSideE(x, (DIGITS[number][4] == 1 ? this->m_fontColor : this->m_background));
    this->printSideF(x, (DIGITS[number][5] == 1 ? this->m_fontColor : this->m_background));
    this->printSideG(x, (DIGITS[number][6] == 1 ? this->m_fontColor : this->m_background));
}

void NumberSet::printErr() {
    this->m_tft->fillRect(this->m_x, this->m_y, this->width(), this->height(), this->m_background);
    uint8_t x = this->m_x + MARGIN_SIZE;
    this->printSideA(x, this->m_fontColor);
    this->printSideD(x, this->m_fontColor);
    this->printSideE(x, this->m_fontColor);
    this->printSideF(x, this->m_fontColor);
    this->printSideG(x, this->m_fontColor);

    x += this->m_numberWidth + this->m_size;
    this->printSideE(x, this->m_fontColor);
    this->printSideG(x, this->m_fontColor);

    x += this->m_numberWidth + this->m_size;
    this->printSideE(x, this->m_fontColor);
    this->printSideG(x, this->m_fontColor);
}

void NumberSet::printSideA(uint8_t x, uint16_t color) {
    uint8_t y = this->m_y + MARGIN_SIZE;
    uint8_t numberWidth = this->m_numberWidth;

    for (uint8_t line = 1; line <= this->m_size; line++)
        m_tft->drawFastHLine(x + line, y + line - 1, numberWidth - (line * 2), color);
}

void NumberSet::printSideB(uint8_t x, uint16_t color) {
    uint8_t y = this->m_y + MARGIN_SIZE;

    switch (this->m_size) {
        case 1:
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 1, y + 1, this->m_numberHeight / 2 - 1, color);
            break;
        case 2:
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 1, y + 1, this->m_numberHeight / 2 - 1, color);
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 2, y + 2, this->m_numberHeight / 2 - 3, color);
            break;
        case 3:
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 1, y + 1, this->m_numberHeight / 2 - 1, color);
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 2, y + 2, this->m_numberHeight / 2 - 3, color);
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 3, y + 3, this->m_numberHeight / 2 - 5, color);
            break;
    }
}

void NumberSet::printSideC(uint8_t x, uint16_t color) {
    uint8_t y = this->m_y + MARGIN_SIZE + this->m_numberHeight / 2;

    switch (this->m_size) {
        case 1:
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 1, y + 1, this->m_numberHeight / 2 - 1, color);
            break;
        case 2:
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 1, y, this->m_numberHeight / 2 - 1, color);
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 2, y + 1, this->m_numberHeight / 2 - 3, color);
            break;
        case 3:
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 1, y + 1, this->m_numberHeight / 2 - 1, color);
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 2, y + 2, this->m_numberHeight / 2 - 3, color);
            this->m_tft->drawFastVLine(x + this->m_numberWidth - 3, y + 3, this->m_numberHeight / 2 - 5, color);
            break;
    }
}

void NumberSet::printSideD(uint8_t x, uint16_t color) {
    uint8_t y = this->m_y + MARGIN_SIZE + this->m_numberHeight;

    for (uint8_t line = 1; line <= this->m_size; line++)
        m_tft->drawFastHLine(x + line, y - line, this->m_numberWidth - (line * 2), color);
}

void NumberSet::printSideE(uint8_t x, uint16_t color) {
    uint8_t y = this->m_y + MARGIN_SIZE + this->m_numberHeight / 2;

    switch (this->m_size) {
        case 1:
            this->m_tft->drawFastVLine(x, y + 1, this->m_numberHeight / 2 - 1, color);
            break;
        case 2:
            this->m_tft->drawFastVLine(x, y, this->m_numberHeight / 2 - 1, color);
            this->m_tft->drawFastVLine(x + 1, y + 1, this->m_numberHeight / 2 - 3, color);
            break;
        case 3:
            this->m_tft->drawFastVLine(x, y + 1, this->m_numberHeight / 2 - 1, color);
            this->m_tft->drawFastVLine(x + 1, y + 2, this->m_numberHeight / 2 - 3, color);
            this->m_tft->drawFastVLine(x + 2, y + 3, this->m_numberHeight / 2 - 5, color);
            break;
    }
}

void NumberSet::printSideF(uint8_t x, uint16_t color) {
    uint8_t y = this->m_y + MARGIN_SIZE;

    switch (this->m_size) {
        case 1:
            this->m_tft->drawFastVLine(x, y + 1, this->m_numberHeight / 2 - 1, color);
            break;
        case 2:
            this->m_tft->drawFastVLine(x, y + 1, this->m_numberHeight / 2 - 1, color);
            this->m_tft->drawFastVLine(x + 1, y + 2, this->m_numberHeight / 2 - 3, color);
            break;
        case 3:
            this->m_tft->drawFastVLine(x, y + 1, this->m_numberHeight / 2 - 1, color);
            this->m_tft->drawFastVLine(x + 1, y + 2, this->m_numberHeight / 2 - 3, color);
            this->m_tft->drawFastVLine(x + 2, y + 3, this->m_numberHeight / 2 - 5, color);
            break;
    }
}

void NumberSet::printSideG(uint8_t x, uint16_t color) {
    uint8_t y = this->m_y + MARGIN_SIZE + (this->m_numberHeight / 2);

    switch (this->m_size) {
        case 1:
            this->m_tft->drawFastHLine(x + 1, y, this->m_numberWidth - 2, color);
            break;
        case 2:
            this->m_tft->fillRect(x + 2, y - 1, this->m_numberWidth - 4, this->m_size, color);
            break;
        case 3:
            this->m_tft->fillRect(x + 2, y - 1, this->m_numberWidth - 4, this->m_size, color);
            this->m_tft->drawPixel(x + 1, y, color);
            this->m_tft->drawPixel(x + this->m_numberWidth - 2, y, color);
            break;
    }
}

void NumberSet::printPunctuation(char chr, uint8_t x, uint16_t color) {
    if (chr == ':') {
        this->printColon(x, color);
    } else if (chr == '.') {
        this->printDot(x, color);
    }
}

void NumberSet::printColon(uint8_t x, uint16_t color){
    uint8_t y = this->m_y + MARGIN_SIZE + (this->m_numberHeight / 4);
    if (this->m_size == 3)
        y -= 1;
    this->m_tft->fillRect(x, y, this->m_size, this->m_size, color);

    y += this->m_numberHeight / 2;
    if (this->m_size == 2)
        y -= 1;
    if (this->m_size == 1)
        y += 1;
    this->m_tft->fillRect(x, y, this->m_size, this->m_size, color);
}

void NumberSet::printDot(uint8_t x, uint16_t color) {
    uint8_t y = this->m_y + MARGIN_SIZE + this->m_numberHeight - this->m_size;
    this->m_tft->fillRect(x, y, this->m_size, this->m_size, color);
}
