#ifndef BIN_H
#define BIN_H
#include "EDGE.h"
#include <iostream>

class BIN{
public:
    BIN(int, int);
    BIN(int, int, int, int);
    BIN(int, int, int, bool);

    // int print();

private:
    const int x, y;
    EDGE *vertical, *horizontal;

};

#endif