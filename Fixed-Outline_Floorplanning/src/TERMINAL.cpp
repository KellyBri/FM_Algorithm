#include "TERMINAL.h"
#include <iostream>

TERMINAL::TERMINAL(int id){
    this->ID = id;
}

TERMINAL::TERMINAL(int id, int x, int y){
    this->ID = id;
    this->x = x;
    this->y = y;
}

void TERMINAL::setCoordinate(int x, int y){
    this->x = x;
    this->y = y;
}

int TERMINAL::getID(){ return this->ID; }
int TERMINAL::getX(){ return this->x; }
int TERMINAL::getY(){ return this->y; }

void TERMINAL::print(){
    std::cout << "p" << this->ID << "\t";
    std::cout << "(" << this->x << ", " << this->y << ")" << std::endl;
}