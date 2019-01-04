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
    
    int getX();
    int getY();

    // int print();
    int getLabel();
    void setLabel(int);

    double getCost();
    void setCost(double);

    int getDemand(bool);
    int getSupply(bool);

    double getCong();
    void setCong(double);
    
    bool getOverflow(bool);
    double getCongestion(bool);
    void subDemand(bool, int, int);

    int getNetID(bool, int);
    int getNetNum(bool);

    void setEdge(bool, int);
    bool hasEdge(bool);

    void setParent(int, int);
    int getParentX();
    int getParentY();

    int  getHistory();
    int  getHistory(bool);
    void setHistory(int);
    void addHistory(bool);

    void setInQueue(bool);
    bool getInQueue();

private:
    const int x, y;
    int label, history;
    double cost, cong;
    int parentX, parentY;
    bool inQueue;
    EDGE *vertical, *horizontal;

};

#endif