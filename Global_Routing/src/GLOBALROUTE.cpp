#include "GLOBALROUTE.h"
#include <iostream>

GLOBALROUTE::GLOBALROUTE(int gx, int gy, int vc, int hc):
                        gridxNum(gx), gridyNum(gy), verticalCap(vc), horizontalCap(hc){
    this->netNum = 0;
    this->curLabel   = new int *[gy];
    for(int i=0; i<gy; ++i){
        this->curLabel[i]   = new int [gx];
        memset(this->curLabel[i], 0, sizeof(int)*gx);
    }

    for(int i=0; i<gy; ++i){
        std::vector<BIN *> temp;
        for(int j=0; j<gx; ++j){
            BIN *bin;
            if(i == gy-1 && j == gx-1) bin = new BIN(i, j);
            else if(i == gy-1) bin = new BIN(i, j, hc, false);
            else if(j == gx-1) bin = new BIN(i, j, vc, true);
            else bin = new BIN(i, j, vc, hc);
    
            temp.push_back(bin);
        }
        this->bins.push_back(temp);
    }
    // for(int i=0; i<gy; ++i){
    //     for(int j=0; j<gx; ++j){
    //         std::cout<<this->bins[i][j]->print()<<" ";
    //     }
    //     std::cout<<std::endl;
    // }
}

void GLOBALROUTE::addNet(NET net){
    this->netNum++;
    this->nets.push_back(net);
}

int GLOBALROUTE::getNetNum(){ return this->netNum; }
NET GLOBALROUTE::getNet(int i){ return nets[i]; }



/*********************************/
/**************private************/
/*********************************/

void GLOBALROUTE::route(){
    //route all of the nets onto grid without considering capacity
    std::cout<<"\t/***********************/\n";
    std::cout<<"\t/**** Start phase 1 ****/\n";
    std::cout<<"\t/***********************/\n";
    phase1();


    //rip off

    //re-route
}

//implement Lee algorithm with A*-search
void GLOBALROUTE::phase1(){

    int startX,  startY;
    int targetX, targetY;
    for(int i=0; i<this->netNum; ++i){
        clearGrid(this->curLabel);
        startX  = this->nets[i].getPinX(0);
        startY  = this->nets[i].getPinY(0);
        targetX = this->nets[i].getPinX(1);
        targetY = this->nets[i].getPinY(1);

        //computing label g(x)
        //computing cost  h(x) = max( distToTarget_x, distToTarget_y)
        //next x: minimal f(x) = g(x) + h(x)
    }
}

void GLOBALROUTE::clearGrid(int **a){
    for(int i=0; i<this->gridyNum; ++i)
        memset(a[i],   0, sizeof(int)*this->gridxNum);
}