#include "BIN.h"
#include <iostream>

BIN::BIN(int x, int y): x(x), y(y){
    this->vertical   = NULL;
    this->horizontal = NULL;
    this->label = -1;
    this->cost = -1;
    this->parentX = 0;
    this->parentY = 0;
    this->cong = 0;
    this->history = 1;
    this->inQueue = false;
}

BIN::BIN(int x, int y, int vc, int hc): x(x), y(y){
    this->vertical   = new EDGE(vc);
    this->horizontal = new EDGE(hc);
    this->label = -1;
    this->cost = -1;
    this->parentX = 0;
    this->parentY = 0;
    this->cong = 0;
    this->history = 1;
    this->inQueue = false;
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
    this->parentX = 0;
    this->parentY = 0;
    this->cong = 0;
    this->history = 1;
    this->inQueue = false;
}

BIN::~BIN(){
    if(this->vertical   != NULL) delete this->vertical;
    if(this->horizontal != NULL) delete this->horizontal;
}

int BIN::getX(){ return this->x; }
int BIN::getY(){ return this->y; }

void BIN::setInQueue(bool q){ this->inQueue = q; }
bool BIN::getInQueue(){ return this->inQueue; }

int BIN::getLabel(){ return this->label; }
void BIN::setLabel(int l){ this->label = l; }

double BIN::getCost(){ return this->cost; }
void BIN::setCost(double c){ this->cost = c; }

double BIN::getCong(){ return this->cong; }
void BIN::setCong(double c){ this->cong = c; }


void BIN::setParent(int x, int y){
    this->parentX = x;
    this->parentY = y;
}
int BIN::getParentX(){ return this->parentX; }
int BIN::getParentY(){ return this->parentY; }

void BIN::setEdge(bool d, int id){
    //set vertical edge
    if(d) this->vertical  ->addNet(id);
    else  this->horizontal->addNet(id);
}
bool BIN::hasEdge(bool d){
    //set vertical edge
    if(  d && this->vertical  ==NULL) return false;
    if( !d && this->horizontal==NULL) return false;
    return true;
}


int BIN::getDemand(bool vh){
    //overflow of vertical edge
    if(vh && this->vertical != NULL){
        return this->vertical->getDemand();
    }else if(!vh && this->horizontal != NULL){
        return this->horizontal->getDemand();
    }
    return 0;
}

int BIN::getSupply(bool vh){
    //overflow of vertical edge
    if(vh && this->vertical != NULL){
        return this->vertical->getSupply();
    }else if(!vh && this->horizontal != NULL){
        return this->horizontal->getSupply();
    }
    return 0;
}

void BIN::addHistory(bool vh){
    if( vh && this->vertical != NULL){
        this->vertical->addHistory();
    }else if(!vh && this->horizontal != NULL){
        this->horizontal->addHistory();
    }
}
int BIN::getHistory(bool vh){
    if( vh && this->vertical != NULL){
        return this->vertical->getHistory();
    }else if(!vh && this->horizontal != NULL){
        return this->horizontal->getHistory();
    }else return 0;
}
int BIN::getHistory(){ return this->history; }




bool BIN::getOverflow(bool vh){
    //overflow of vertical edge
    if(vh && this->vertical != NULL){
        return this->vertical->overflow();
    }else if(!vh && this->horizontal != NULL){
        return this->horizontal->overflow();
    }
    return false;
}

double BIN::getCongestion(bool vh){
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
}

// int BIN::print(){
//     int c = 0;
//     if(this->vertical   != NULL) ++c;
//     if(this->horizontal != NULL) ++c;
//     return c;
// }