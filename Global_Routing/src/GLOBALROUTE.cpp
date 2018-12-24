#include "GLOBALROUTE.h"

GLOBALROUTE::GLOBALROUTE(int netNum){
    this->netNum = netNum;
}

void GLOBALROUTE::setGrid(int x, int y){
    this->gridxNum = x;
    this->gridyNum = y;
}

void GLOBALROUTE::setCapacity(int v, int h){
    this->verticalCap   = v;
    this->horizontalCap = h;
}

void GLOBALROUTE::addNet(NET net){
    this->nets.push_back(net);
}