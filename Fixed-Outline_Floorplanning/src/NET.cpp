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
    this->kind.push_back(k);
    this->nodeID.push_back(id);
}

void NET::print(){
    std::cout << this->ID << ": ";
    for(int i=0; i<this->degree; ++i){
        if( this->kind[i] == PL )
            std::cout << "p" << this->nodeID[i]<<"\t";
        else std::cout << "sb" << this->nodeID[i]<<"\t";
    }
    std::cout<<std::endl;
}