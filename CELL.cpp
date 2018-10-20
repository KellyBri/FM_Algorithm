#include <iostream>
#include "CELL.h"

//constructor
CELL::CELL(){
    this->ID = 0;
    this->size = 0;
    this->next = NULL;
    this->pre = NULL;
    this->lock = false;
    this->set = false;
}
CELL::CELL(int id, int size){
    this->ID = id;
    this->size = size;
    this->next = NULL;
    this->pre = NULL;
    this->lock = false;
    this->set = false;
}

void CELL::setID(int id){ this->ID = id; }
void CELL::setSize(int size){ this->size = size; }
void CELL::setGain(int g){ this->gain = g; }
void CELL::setNext(CELL *c){ this->next = c; }
void CELL::setPre(CELL *c){ this->pre = c; }
void CELL::setSet(bool s){ this->set = s; }
void CELL::setLock(bool l){ this->lock = l; }

int CELL::getID(){ return this->ID; }
int CELL::getSize(){ return this->size; }
int CELL::getPin(){ return this->nets.size(); }
int CELL::getGain(){ return this->gain; }
CELL *CELL::getPre(){ return this->pre; }
CELL *CELL::getNext(){ return this->next; }
bool CELL::getSet(){ return this->set; }
bool CELL::getLock(){ return this->lock; }
std::vector<int> CELL::getNet(){ return this->nets; }

void CELL::addNet(int net){ this->nets.push_back(net); }

//print the informance of the cell
void CELL::print(){
    std::cout << "ID: " << this->ID << std::endl;
    // std::cout << "Size: " << this->size << std::endl;
    // std::cout << "Set: "<< this->set << std::endl;
    std::cout << "Gain: "<< this->gain << std::endl;

    // std::cout <<"Nets: ";
    // for(std::vector<int>::iterator it=this->nets.begin(); it!=this->nets.end(); ++it)
    //     std::cout << *it << "\t";
    // std::cout<<std::endl;
}