/*
 * Extends the LCD_5110_Basic library: http://www.rinkydinkelectronics.com/library.php?id=44
 * adding the ability to rotate fonts (which is required for the ZX Spectrum font)
 */
#include "nokia_5110.h"


uint8_t outArray[8];
uint8_t fontchar[8];

nokia_5110::nokia_5110(int SCK, int MOSI, int DC, int RST, int CS) : LCD5110(SCK, MOSI, DC, RST, CS) {
}

// from https://forum.arduino.cc/index.php?topic=297436.0
void nokia_5110::rotate() {

    int i, j, val;

    for (i = 0; i < 8; i++) {
        outArray[i] = 0;
    }

    //rotate 90* clockwise
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            val = ((fontchar[i] >> j) & 1); //extract the j-th bit of the i-th element
            outArray[7 - j] |= (val << i); //set the newJ-th bit of the newI-th element
        }
    }
    return;
}

void nokia_5110::print(char *st, int x, int y, int rotations) {
    unsigned char ch;
    int stl, row, xp;

    if (!_sleep) {
        stl = strlen(st);
        if (x == RIGHT)
            x = 84 - (stl * cfont.x_size);
        if (x == CENTER)
            x = (84 - (stl * cfont.x_size)) / 2;
        if (x < 0)
            x = 0;

        row = y / 8;
        xp = x;

        for (int cnt = 0; cnt < stl; cnt++)
            if (rotations == 0)
                _print_char(*st++, x + (cnt * (cfont.x_size)), row);
            else
                _print_char2(*st++, x + (cnt * (cfont.x_size)), row, rotations);
    }
}

// rotated text only works with 8x8 fonts.
void nokia_5110::_print_char2(unsigned char c, int x, int row, int rotations) {
    if (!_sleep) {
        if (((x + cfont.x_size) <= 84) and (row + (cfont.y_size / 8) <= 6)) {
            // copy 8 bytes of font and rotate
            int font_idx = ((c - cfont.offset) * (cfont.x_size * (cfont.y_size / 8))) + 4;
            for (int i = 0; i < 8; i++) {
                fontchar[i] = fontbyte(font_idx + i);
            }
            for (int i = 0; i < rotations; i++) {
                rotate();
            }

            for (int rowcnt = 0; rowcnt < (cfont.y_size / 8); rowcnt++) {
                _LCD_Write(PCD8544_SETYADDR | (row + rowcnt), LCD_COMMAND);
                _LCD_Write(PCD8544_SETXADDR | x, LCD_COMMAND);

                for (int cnt = 0; cnt < cfont.x_size; cnt++) {
                    if (cfont.inverted == 0)
                        _LCD_Write(outArray[cnt], LCD_DATA);
                    else
                        _LCD_Write(~(outArray[cnt]), LCD_DATA);
                }
            }
            _LCD_Write(PCD8544_SETYADDR, LCD_COMMAND);
            _LCD_Write(PCD8544_SETXADDR, LCD_COMMAND);
        }
    }
}

// Is there a better way of doing this without copying the entire method?
// Anyway, prints rotated numbers.
void nokia_5110::printNumIR(long num, int x, int y, int length, char filler) {
    char buf[25];
    char st[27];
    boolean neg = false;
    int c = 0, f = 0;

    if (!_sleep) {
        if (num == 0) {
            if (length != 0) {
                for (c = 0; c < (length - 1); c++)
                    st[c] = filler;
                st[c] = 48;
                st[c + 1] = 0;
            } else {
                st[0] = 48;
                st[1] = 0;
            }
        } else {
            if (num < 0) {
                neg = true;
                num = -num;
            }

            while (num > 0) {
                buf[c] = 48 + (num % 10);
                c++;
                num = (num - (num % 10)) / 10;
            }
            buf[c] = 0;

            if (neg) {
                st[0] = 45;
            }

            if (length > (c + neg)) {
                for (int i = 0; i < (length - c - neg); i++) {
                    st[i + neg] = filler;
                    f++;
                }
            }

            for (int i = 0; i < c; i++) {
                st[i + neg + f] = buf[c - i - 1];
            }
            st[c + neg + f] = 0;

        }

        print(st, x, y, 1);
    }
}
