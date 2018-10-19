#include <iostream>
#include "NET.h"

//constructor
NET::NET(int id){
    this->ID = id;
    this->isCut = false;
}

void NET::setIsCut(bool c){ this->isCut = c; }

int NET::getID(){ return this->ID; }
int NET::getSize(){ return this->cells.size(); }
bool NET::getIsCut(){ return this->isCut; }
std::vector<int> NET::getCells(){ return this->cells; }

void NET::addCell(int cell){
    this->cells.push_back(cell);
}
//print the informance of the net
void NET::print(){
    std::cout << "ID: " << this->ID << "\tSize: " << getSize() << std::endl <<"Cells: ";
    for(std::vector<int>::iterator it = this->cells.begin(); it!=this->cells.end(); ++it)
        std::cout << *it << "\t";
    std::cout<<std::endl;
}