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
int NET::getDistribution(bool i){
    if(i) return this->distribution[1];
    else return this->distribution[0];
}

void NET::addCell(int cell){
    this->cells.push_back(cell);
}

void NET::setDistribution(int A, int B){
    this->distribution[0] = A;
    this->distribution[1] = B;
}

//print the informance of the net
void NET::print(){
    std::cout << "ID: " << this->ID << "\tCut: " << getIsCut() << std::endl;
    std::cout << "Disrtibution: (" << this->distribution[0] <<", " << this->distribution[1] << ")" << std::endl;
    std::cout <<"Cells: ";
    for(std::vector<int>::iterator it = this->cells.begin(); it!=this->cells.end(); ++it)
        std::cout << *it << "\t";
    std::cout<<std::endl;
}