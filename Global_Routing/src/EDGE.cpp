#include "EDGE.h"

EDGE::EDGE(int s): supply(s){
    this->demand = 0;
}

void EDGE::addNet(int id){
    this->demand++;
    this->netID.push_back(id);
}