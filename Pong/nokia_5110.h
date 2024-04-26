// Additional methods added for rotating fonts
#ifndef NOKIA_5110_h
#define NOKIA_5110_h

#include <LCD5110_Basic.h>

class nokia_5110 : public LCD5110 {
public:
    nokia_5110(int SCK, int MOSI, int DC, int RST, int CS);

    void rotate(void);

    void print(char *st, int x, int y, int rotations = 0);

    void printNumIR(long num, int x, int y, int length = 0, char filler = ' ');

protected:
    void _print_char2(unsigned char c, int x, int row, int rotations);
};

#endif
