/*
    This is a set of 4 numbers.
    It allows to print number in the following formats:
        1234, 12:34, 12.34, 123.4, 1.234, -1234, -1.234, -12.34, -123.4
    It also have 3 sizes (1, 2, 3)

    The numbers-set starts at a given pixel coordinate (x, y),
    and returns width and height, depending on the size.

    It has 1 margin pixel at all sides.

    A
   ---
F |   | B
  | G |
   ---
E |   | C
  |   |
   ---
    D

    Diego M. Lopez - 2021 (ldiegom@gmail.com)
*/

#ifndef number_set_h
#define number_set_h

//#include <iostream>
#include <string>
#include <regex>
#include <Arduino.h>
#include <TFT_ILI9163C.h>
#include <timer.h>

#define NS_SIZE_1 1;
#define NS_SIZE_2 2;
#define NS_SIZE_3 3;

#define NS_HHMM 0 // HH:MM (can set integrer number "HHMM" or "HH:MM")
#define NS_UINT 1 // Unsigned integer (only positive values)
#define NS_SINT 2 // Integer with sign
#define NS_UFLT 3 // Unsigned float (only positive values)
#define NS_SFLT 4 // Float with sign

#define NS_SIZE_1_H 13
#define NS_HHMM_1_W 31
#define NS_UINT_1_W 29
#define NS_SINT_1_W 32
#define NS_UFLT_1_W 31
#define NS_SFLT_1_W 34

#define NS_SIZE_2_H 20
#define NS_HHMM_2_W 52
#define NS_UINT_2_W 48
#define NS_SINT_2_W 53
#define NS_UFLT_2_W 52
#define NS_SFLT_2_W 57

#define NS_SIZE_3_H 45
#define NS_HHMM_3_W 101
#define NS_UINT_3_W 95
#define NS_SINT_3_W 106
#define NS_UFLT_3_W 101
#define NS_SFLT_3_W 112

const uint8_t NSET_MARGIN_SIZE = 1;

// Regex definition and test cases: https://regexr.com/6dlos
extern const char* NSET_REG_EXPR[5];

const uint8_t NSET_DIGITS[10][7] = {
//   a  b  c  d  e  f  g
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}  // 9
};

struct nset_sizes_t {
    uint8_t signWidth;
    uint8_t numberHeight;
    uint8_t numberWidth;
};
const nset_sizes_t NSET_SIZES[3] = {
    {2, 11, 6},
    {3, 18, 10},
    {8, 43, 21}
};

class NumberSet {
    private:
        TFT_ILI9163C* m_tft;
        uint8_t m_x, m_y;
        uint8_t m_format;
        uint8_t m_size;
        uint16_t m_background;
        uint16_t m_fontColor;
        bool m_unsigned;
        char m_chars[7], m_prevChars[7];
        uint8_t m_signWidth, m_numberHeight, m_numberWidth;
        Timer *tmrReset;
        
        void resetChars();
        void resetPrevChars();
        bool isValidNumber(String number);
        bool charsChanged();
        void setPreviousChars();
        void printSign(uint16_t color);
        void printChars();
        void printNumber(uint8_t number, uint8_t x);
        void printErr();
        void printSideA(uint8_t x, uint16_t color);
        void printSideB(uint8_t x, uint16_t color);
        void printSideC(uint8_t x, uint16_t color);
        void printSideD(uint8_t x, uint16_t color);
        void printSideE(uint8_t x, uint16_t color);
        void printSideF(uint8_t x, uint16_t color);
        void printSideG(uint8_t x, uint16_t color);
        void printPunctuation(char chr, uint8_t x, uint16_t color);
        void printColon(uint8_t x, uint16_t color);
        void printDot(uint8_t x, uint16_t color);

    public:
        NumberSet(TFT_ILI9163C* tft, uint8_t x, uint8_t y, uint8_t format,
                  uint8_t size = 1, uint16_t background = 0x0000U, uint16_t fontColor = 0xFFFFU);

        uint8_t x();
        uint8_t y();
        uint8_t height();
        uint8_t width();
        void setValue(float number);
        void setValue(int16_t number);
        void setValue(String number);
        String getValue();

        void refresh();
};

#endif
