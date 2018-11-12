#include "NET.h"
#include <iostream>

#define PL 0    //PL stands for TERMINAL
#define SB 1    //SB stands for hardblock

NET::NET(int id){
    this->ID = id;
    this->degree = 0;
}

void NET::setDegree(int d){ this->degree = d; }
void NET::addNode(bool k, int id){
    if(k) this->blockID.push_back(id);
    else this->terminalID.push_back(id);
}

int NET::getID(){ return this->ID; }
int NET::getTerminalSize(){ return this->terminalID.size(); }
int NET::getTerminal(int i){ return this->terminalID[i]; }
int NET::getBlockSize(){ return this->blockID.size(); }
int NET::getBlock(int i){ return this->blockID[i]; }

void NET::print(){
    std::cout << this->ID << ": ";
    for(auto it=this->blockID.begin(); it!=this->blockID.end(); ++it)
        std::cout << "sb" << (*it) << "\t";
    for(auto it=this->terminalID.begin(); it!=this->terminalID.end(); ++it)
        std::cout << "p" << (*it) << "\t";
    std::cout<<std::endl;
}