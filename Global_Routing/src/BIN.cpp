#include "BIN.h"


BIN::BIN(int x, int y): x(x), y(y){
    this->vertical   = NULL;
    this->horizontal = NULL;
}

BIN::BIN(int x, int y, int vc, int hc): x(x), y(y){
    this->vertical   = new EDGE(vc);
    this->horizontal = new EDGE(hc);
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
}

// int BIN::print(){
//     int c = 0;
//     if(this->vertical != NULL) ++c;
//     if(this->horizontal != NULL) ++c;
//     return c;
// }