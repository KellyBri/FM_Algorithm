#ifndef BIN_H
#define BIN_H
#include "EDGE.h"
#include <iostream>

class BIN{
public:
    BIN(int, int);
    BIN(int, int, int, int);
    BIN(int, int, int, bool);
    ~BIN();

    // int print();
    int getLabel();
    void setLabel(int);

    int getCost();
    void setCost(int);
    
    bool getOverflow(bool);
    float getCongestion(bool);
    void subDemand(bool, int, int);

    int getNetID(bool, int);
    int getNetNum(bool);

    void setEdge(bool, int);
    bool hasEdge(bool);


private:
    const int x, y;
    int label, cost;
    EDGE *vertical, *horizontal;

};

#endif