#include "BIN.h"
#include <iostream>

BIN::BIN(int x, int y): x(x), y(y){
    this->vertical   = NULL;
    this->horizontal = NULL;
    this->label = -1;
    this->cost = -1;
}

BIN::BIN(int x, int y, int vc, int hc): x(x), y(y){
    this->vertical   = new EDGE(vc);
    this->horizontal = new EDGE(hc);
    this->label = -1;
    this->cost = -1;
}

BIN::BIN(int x, int y, int c, bool vh): x(x), y(y){
    //this bin only has vertical edge
    if(vh){
        this->vertical   = new EDGE(c);
        this->horizontal = NULL;
    }else{
        this->vertical   = NULL;
        this->horizontal = new EDGE(c);
    }
    this->label = -1;
    this->cost = -1;
}

BIN::~BIN(){
    if(this->vertical   != NULL) delete this->vertical;
    if(this->horizontal != NULL) delete this->horizontal;
}

int BIN::getLabel(){ return this->label; }
void BIN::setLabel(int l){ this->label = l; }

int BIN::getCost(){ return this->cost; }
void BIN::setCost(int c){ this->cost = c; }

void BIN::setEdge(bool d, int id){
    //set vertical edge
    if(d) this->vertical  ->addNet(id);
    else  this->horizontal->addNet(id);
    getCongestion(d);
    // if(x==30&&y==30) std::cout<<"aa "<<id<<std::endl;
}
bool BIN::hasEdge(bool d){
    //set vertical edge
    if(  d && this->vertical  ==NULL) return false;
    if( !d && this->horizontal==NULL) return false;
    return true;
}

bool BIN::getOverflow(bool vh){
    //overflow of vertical edge
    if(vh && this->vertical != NULL){
        return this->vertical->overflow();
    }else if(!vh && this->horizontal != NULL){
        return this->horizontal->overflow();
    }
    return false;
}

float BIN::getCongestion(bool vh){
    //overflow of vertical edge
    if(vh && this->vertical != NULL){
        this->vertical->calCongestion();
        return this->vertical->getCongestion();
    }else if(!vh && this->horizontal != NULL){
        this->horizontal->calCongestion();
        return this->horizontal->getCongestion();
    }
    return -1;
}

void BIN::subDemand(bool vh, int s, int id){
    if(vh && this->vertical != NULL){
        this->vertical->subDemand(s, id);
    }else if(!vh && this->horizontal != NULL){
        this->horizontal->subDemand(s, id);
    }
}


//broken, but I don't figure out why
int BIN::getNetID(bool vh, int i){
    //overflow of vertical edge
    if(vh) return this->vertical->getNetID(i);
    else   return this->horizontal->getNetID(i);
    // if(vh){
    //     this->netId = this->vertical->getNetID(i);
    //     std::cout<<"v "<<this->vertical->getNetID(i)<<"\t";
    //     return this->netId;
    // }
    // else{
    //     this->netId = this->horizontal->getNetID(i);
    //     std::cout<<"h "<<this->netId<<"\t";
    //     return this->netId;
    // }
    // return this->netId;
}



int BIN::getNetNum(bool vh){
    //overflow of vertical edge
    if(vh && this->vertical != NULL){
        return this->vertical->getNetNum();
    }else if(!vh && this->horizontal != NULL){
        return this->horizontal->getNetNum();
    }
    return 0;
    // if(vh && ) return this->vertical->getNetNum();
    // else   return this->horizontal->getNetNum();
}

// int BIN::print(){
//     int c = 0;
//     if(this->vertical   != NULL) ++c;
//     if(this->horizontal != NULL) ++c;
//     return c;
// }