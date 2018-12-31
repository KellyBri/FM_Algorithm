#include "GLOBALROUTE.h"
#include <iostream>
#include <algorithm>
// #include <cmath>


GLOBALROUTE::GLOBALROUTE(int gx, int gy, int vc, int hc):
                        gridxNum(gx), gridyNum(gy), verticalCap(vc), horizontalCap(hc){
    this->netNum = 0;
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
}

GLOBALROUTE::~GLOBALROUTE(){
    for(int i=0; i<this->gridyNum; ++i){
        for(int j=0; j<this->gridxNum; ++j){
            delete this->bins[i][j];
        }
        this->bins[i].clear();
    }
    this->bins.clear();

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
    
    // std::cout<<"/***********************/\n";
    std::cout<<"/**** Start phase 1 ****/\n";
    //route all of the nets onto grid without considering capacity
    phase1();
    printOverflow();

    //rip off
    std::cout<<"/**** Start phase 2 ****/\n";
    phase2();
  
    // int count = 0;
    // for(int i=0; i<this->netNum; ++i){
    //     if(this->nets[i].getOverflow() != 0) ++count;
    //     // std::cout<<"ID: "<<this->nets[i].getID()<<"\toverflow: "<<this->nets[i].getOverflow()<<std::endl;
    // }
    // std::cout<<"# of overflow net " << count<<std::endl;

    //re-route
    // std::cout<<"/**** Start phase 3 ****/\n";
    std::cout<<"/***********************/\n";
    printOverflow();
}

//implement Lee algorithm with A*-search
void GLOBALROUTE::phase1(){

    int x, y;
    int startX,  startY;
    int targetX, targetY;
    
    //Lee algorithm
    for(int i=0; i<this->netNum; ++i){
        clearLabel();
        x = startX  = this->nets[i].getPinX(0);
        y = startY  = this->nets[i].getPinY(0);
        targetX = this->nets[i].getPinX(1);
        targetY = this->nets[i].getPinY(1);
        //add start point
        this->nets[i].addPath(startX, startY);

        bool firstStep = true;
        // bool tempDirection = VERTICAL;
        bool direction, tempDirection;
        direction = tempDirection = VERTICAL;
        while( x != targetX || y != targetY ){
            // computing label g(x)
            // computing cost  h(x) = max( distToTarget_x, distToTarget_y)
            // f(x) = g(x) + h(x)
            int min = 2147483647;
            int tempX = x, tempY = y;
            if(x+1 < this->gridxNum ){
                this->bins[y][x+1]->setLabel(this->bins[y][x]->getLabel()+1);
                this->bins[y][x+1]->setCost( std::max( abs(targetX-x-1), abs(targetY-y) ) );
                if( this->bins[y][x+1]->getLabel() + this->bins[y][x+1]->getCost() < min ){
                    min = this->bins[y][x+1]->getLabel() + this->bins[y][x+1]->getCost();
                    tempX = x+1;
                    tempY = y;
                    tempDirection = HORIZONTAL;
                }
            }
            if(x-1 >= 0){
                this->bins[y][x-1]->setLabel(this->bins[y][x]->getLabel()+1);
                this->bins[y][x-1]->setCost( std::max( abs(targetX-x+1), abs(targetY-y) ) );
                if( this->bins[y][x-1]->getLabel() + this->bins[y][x-1]->getCost() < min ){
                    min = this->bins[y][x-1]->getLabel() + this->bins[y][x-1]->getCost();
                    tempX = x-1;
                    tempY = y;
                    tempDirection = HORIZONTAL;
                }   
            }
            if(y+1 < this->gridyNum){
                this->bins[y+1][x]->setLabel(this->bins[y][x]->getLabel()+1);
                this->bins[y+1][x]->setCost( std::max( abs(targetX-x), abs(targetY-y-1) ) );
                if( this->bins[y+1][x]->getLabel() + this->bins[y+1][x]->getCost() < min ){
                    min = this->bins[y+1][x]->getLabel() + this->bins[y+1][x]->getCost();
                    tempX = x;
                    tempY = y+1;
                    tempDirection = VERTICAL;
                }   
            }
            if(y-1 >= 0){
                this->bins[y-1][x]->setLabel(this->bins[y][x]->getLabel()+1);
                this->bins[y-1][x]->setCost( std::max( abs(targetX-x), abs(targetY-y+1) ) );
                if( this->bins[y-1][x]->getLabel() + this->bins[y-1][x]->getCost() < min ){
                    min = this->bins[y-1][x]->getLabel() + this->bins[y-1][x]->getCost();
                    tempX = x;
                    tempY = y-1;
                    tempDirection = VERTICAL;
                }
            }

            if( x+1 == tempX || y+1 == tempY ) this->bins[y][x]->setEdge(tempDirection, i);
            else if( x-1 == tempX ) this->bins[y][tempX]->setEdge(tempDirection, i);
            else if( y-1 == tempY ) this->bins[tempY][x]->setEdge(tempDirection, i);

            // if( x+1 == tempX || y+1 == tempY ){
            //     this->bins[y][x]->setEdge(tempDirection, i);
            //     if(x==30 && y==30) std::cout<<i<<std::endl;
            // }else if( x-1 == tempX ){
            //     this->bins[y][tempX]->setEdge(tempDirection, i);
            //     if(tempX==30 && y==30) std::cout<<i<<std::endl;
            // }else if( y-1 == tempY ){
            //     this->bins[tempY][x]->setEdge(tempDirection, i);
            //     if(x==30 && tempY==30) std::cout<<i<<std::endl;
            // }


            //add path to net 
            // this->nets[i].addPath(tempX, tempY);
            if(firstStep) firstStep = false;
            else if(tempDirection != direction) this->nets[i].addPath(x, y);
            
            //next x: minimal f(x) = g(x) + h(x)
            x = tempX;
            y = tempY;
            direction = tempDirection;
        }
        // for(int i=0; i<this->bins[30][30]->getNetNum(VERTICAL); ++i){
        //     std::cout<<this->bins[30][30]->getNetID(VERTICAL, i)<<" ";
        // }
        // std::cout<<std::endl;
        this->nets[i].addPath(targetX, targetY);

        //for debugging
        // if(i==10){
        //     for(int a=0; a<this->gridyNum; ++a){
        //         for(int j=0; j<this->gridxNum; ++j){
        //             if(a==startY && j == startX) std::cout<<"S ";
        //             else if(a==targetY && j == targetX) std::cout<<"T ";
        //             else std::cout<<this->bins[a][j]->getLabel() + this->bins[a][j]->getCost()<<" ";
        //         }
        //         std::cout<<std::endl;
        //     }
        //     std::cout<<"Start ("<<startX<<", "<<startY<<")"<<std::endl;
        //     std::cout<<"End   ("<<targetX<<", "<<targetY<<")"<<std::endl;
        //     for(int a=0; a<this->nets[i].getPathNum(); ++a){
        //         std::cout<<"("<<this->nets[i].getPathX(a)<<", "<<this->nets[i].getPathY(a)<<")"<<std::endl;
        //     }
        // }
        // std::cout << i << std::endl;
    }

    // for(int i=0; i<this->bins[30][30]->getNetNum(VERTICAL); ++i){
    //     std::cout<<i<<"\t";
    //     int id = this->bins[30][30]->getNetID(VERTICAL, i);
    //     std::cout<<"call "<<id<<"\n";
    // }
    // std::cout<<std::endl;


    //count number of overflow bin on the path
    for(y=0; y<this->gridyNum; ++y){
        for(x=0; x<this->gridxNum; ++x){
            if(this->bins[y][x]->getOverflow(VERTICAL)){
                for(int i=0; i<this->bins[y][x]->getNetNum(VERTICAL); ++i){
                    int id = this->bins[y][x]->getNetID(VERTICAL, i);
                    this->nets[id].addOverflow(1);
                }
            }
            if(this->bins[y][x]->getOverflow(HORIZONTAL)){
                for(int i=0; i<this->bins[y][x]->getNetNum(HORIZONTAL); ++i){
                    int id = this->bins[y][x]->getNetID(HORIZONTAL, i);
                    this->nets[id].addOverflow(1);
                }
            }
        }
    }


    // std::cout<<"Overflow = "<<count<<std::endl;
    // int a = 111;
    // std::cout<<this->nets[a].getPathNum()<<"\n";
    // for(int i=0; i<this->nets[a].getPathNum(); ++i){
    //     std::cout<<"("<<this->nets[a].getPathX(i)<<", "<<this->nets[a].getPathY(i)<<")\n";
    // }
    // std::cout<<this->nets[a].getOverflow()<<std::endl;
}


void GLOBALROUTE::phase2(){

    //rip-off & re-route
    int count = 0;
    while(count<10){
        //sort nets by the number of overflow
        std::sort(this->nets.begin(), this->nets.end(), [](NET a, NET b) -> bool{ return a.getOverflow() > b.getOverflow(); } );
        //find all of nets with non-zero overflow
        int i;
        for(i=0; i<this->netNum; ++i){
            if(this->nets[i].getOverflow() == 0) break;    
        }
        //push all of nets with non-zero overflow into priority_queue
        auto comp = [](NET a, NET b) -> bool{ return a.getOverflow() < b.getOverflow(); };
        std::priority_queue< NET, std::vector<NET>, decltype( comp ) > pqNET(this->nets.begin(), this->nets.begin()+i/2, comp );
        std::sort(this->nets.begin(), this->nets.end(), [](NET a, NET b) -> bool{ return a.getID() < b.getID(); } );
        ++count;
        std::cout<<count<<"\tnet = "<<i<<std::endl;
        while( !pqNET.empty() ){
            
            NET top = pqNET.top();
            ripoff ( top.getID() );
            reRoute( top.getID() );
            // break;
            pqNET.pop();
        }
    }
}

void GLOBALROUTE::ripoff(int id){

    //update demand of each edge on the path
    for(int i=0; i<this->nets[id].getPathNum()-1; ++i){
        int x1 = this->nets[id].getPathX(i);
        int y1 = this->nets[id].getPathY(i);
        int x2 = this->nets[id].getPathX(i+1);
        int y2 = this->nets[id].getPathY(i+1);

        if(x1 == x2){
            if(y1 > y2) std::swap(y1, y2);
            for(int y=y1; y<y2; ++y){
                this->bins[y][x1]->subDemand(VERTICAL, 1, id);
                for(int i=0; i<this->bins[y][x1]->getNetNum(VERTICAL); ++i){
                    int id = this->bins[y][x1]->getNetID(VERTICAL, i);
                    this->nets[id].addOverflow(-1);
                }
            }
        }
        else if(y1 == y2){
            if(x1 > x2) std::swap(x1, x2);
            for(int x=x1; x<x2; ++x){
                this->bins[y1][x]->subDemand(HORIZONTAL, 1, id);
                for(int i=0; i<this->bins[y1][x]->getNetNum(HORIZONTAL); ++i){
                    int id = this->bins[y1][x]->getNetID(HORIZONTAL, i);
                    this->nets[id].addOverflow(-1);
                }
            }
        }
    }
    this->nets[id].ripoff();
}

void GLOBALROUTE::reRoute(int id){

    clearLabel();
    int x, y, startX,  startY, targetX, targetY;
    
    x = startX  = this->nets[id].getPinX(0);
    y = startY  = this->nets[id].getPinY(0);
    targetX = this->nets[id].getPinX(1);
    targetY = this->nets[id].getPinY(1);
    //add start point
    this->nets[id].addPath(startX, startY);

    bool firstStep = true;
    // bool tempDirection = VERTICAL;
    bool direction, tempDirection;
    direction = tempDirection = VERTICAL;
    while( x != targetX || y != targetY ){
        // computing label g(x)
        // computing cost  h(x) = max( distToTarget_x, distToTarget_y)
        // f(x) = g(x) + h(x)
        bool find = false;
        float min = 2147483647;
        int tempX = x, tempY = y;
        if(x+1 < this->gridxNum ){
            this->bins[y][x+1]->setLabel(this->bins[y][x]->getLabel()+1);
            this->bins[y][x+1]->setCost( std::max( abs(targetX-x-1), abs(targetY-y) ) );
            // if( this->bins[y][x+1]->getLabel() + this->bins[y][x+1]->getCost() < min && !this->bins[y][x]->getOverflow(HORIZONTAL) ){
            if( this->bins[y][x+1]->getLabel() + this->bins[y][x+1]->getCongestion(HORIZONTAL) < min ){
                min = this->bins[y][x+1]->getLabel() + this->bins[y][x+1]->getCongestion(HORIZONTAL);
                tempX = x+1;
                tempY = y;
                find = true;
                tempDirection = HORIZONTAL;
            }
        }
        if(x-1 >= 0){
            this->bins[y][x-1]->setLabel(this->bins[y][x]->getLabel()+1);
            this->bins[y][x-1]->setCost( std::max( abs(targetX-x+1), abs(targetY-y) ) );
            // if( this->bins[y][x-1]->getLabel() + this->bins[y][x-1]->getCost() < min && !this->bins[y][x-1]->getOverflow(HORIZONTAL) ){
            if( this->bins[y][x-1]->getLabel() + this->bins[y][x-1]->getCongestion(HORIZONTAL) < min ){
                min = this->bins[y][x-1]->getLabel() + this->bins[y][x-1]->getCongestion(HORIZONTAL);
                tempX = x-1;
                tempY = y;
                find = true;
                tempDirection = HORIZONTAL;
            }   
        }
        if(y+1 < this->gridyNum){
            this->bins[y+1][x]->setLabel(this->bins[y][x]->getLabel()+1);
            this->bins[y+1][x]->setCost( std::max( abs(targetX-x), abs(targetY-y-1) ) );
            // if( this->bins[y+1][x]->getLabel() + this->bins[y+1][x]->getCost() < min && !this->bins[y][x]->getOverflow(VERTICAL) ){
            if( this->bins[y+1][x]->getLabel() + this->bins[y+1][x]->getCongestion(VERTICAL) < min ){
                min = this->bins[y+1][x]->getLabel() + this->bins[y+1][x]->getCongestion(VERTICAL);
                tempX = x;
                tempY = y+1;
                find = true;
                tempDirection = VERTICAL;
            }   
        }
        if(y-1 >= 0){
            this->bins[y-1][x]->setLabel(this->bins[y][x]->getLabel()+1);
            this->bins[y-1][x]->setCost( std::max( abs(targetX-x), abs(targetY-y+1) ) );
            // if( this->bins[y-1][x]->getLabel() + this->bins[y-1][x]->getCost() < min && !this->bins[y-1][x]->getOverflow(VERTICAL) ){
            if( this->bins[y-1][x]->getLabel() + this->bins[y-1][x]->getCongestion(VERTICAL) < min ){
                min = this->bins[y-1][x]->getLabel() + this->bins[y-1][x]->getCongestion(VERTICAL);
                tempX = x;
                tempY = y-1;
                find = true;
                tempDirection = VERTICAL;
            }
        }




        if( !find ){
            if(x+1 < this->gridxNum ){
                this->bins[y][x+1]->setLabel(this->bins[y][x]->getLabel()+1);
                this->bins[y][x+1]->setCost( std::max( abs(targetX-x-1), abs(targetY-y) ) );
                if( this->bins[y][x+1]->getLabel() + this->bins[y][x+1]->getCost() < min ){
                    min = this->bins[y][x+1]->getLabel() + this->bins[y][x+1]->getCost();
                    tempX = x+1;
                    tempY = y;
                    tempDirection = HORIZONTAL;
                }
            }
            if(x-1 >= 0){
                this->bins[y][x-1]->setLabel(this->bins[y][x]->getLabel()+1);
                this->bins[y][x-1]->setCost( std::max( abs(targetX-x+1), abs(targetY-y) ) );
                if( this->bins[y][x-1]->getLabel() + this->bins[y][x-1]->getCost() < min ){
                    min = this->bins[y][x-1]->getLabel() + this->bins[y][x-1]->getCost();
                    tempX = x-1;
                    tempY = y;
                    tempDirection = HORIZONTAL;
                }   
            }
            if(y+1 < this->gridyNum){
                this->bins[y+1][x]->setLabel(this->bins[y][x]->getLabel()+1);
                this->bins[y+1][x]->setCost( std::max( abs(targetX-x), abs(targetY-y-1) ) );
                if( this->bins[y+1][x]->getLabel() + this->bins[y+1][x]->getCost() < min ){
                    min = this->bins[y+1][x]->getLabel() + this->bins[y+1][x]->getCost();
                    tempX = x;
                    tempY = y+1;
                    tempDirection = VERTICAL;
                }   
            }
            if(y-1 >= 0){
                this->bins[y-1][x]->setLabel(this->bins[y][x]->getLabel()+1);
                this->bins[y-1][x]->setCost( std::max( abs(targetX-x), abs(targetY-y+1) ) );
                if( this->bins[y-1][x]->getLabel() + this->bins[y-1][x]->getCost() < min ){
                    min = this->bins[y-1][x]->getLabel() + this->bins[y-1][x]->getCost();
                    tempX = x;
                    tempY = y-1;
                    tempDirection = VERTICAL;
                }
            }
        }

        if( x+1 == tempX || y+1 == tempY ) this->bins[y][x]->setEdge(tempDirection, id);
        else if( x-1 == tempX ) this->bins[y][tempX]->setEdge(tempDirection, id);
        else if( y-1 == tempY ) this->bins[tempY][x]->setEdge(tempDirection, id);


        //add path to net 
        // this->nets[id].addPath(tempX, tempY);
        if(firstStep) firstStep = false;
        else if(tempDirection != direction) this->nets[id].addPath(x, y);
        
        //next x: minimal f(x) = g(x) + h(x)
        x = tempX;
        y = tempY;
        direction = tempDirection;
    }
    this->nets[id].addPath(targetX, targetY);  



    //count number of overflow bin on the path
    for(y=0; y<this->gridyNum; ++y){
        for(x=0; x<this->gridxNum; ++x){
            if(this->bins[y][x]->getOverflow(VERTICAL)){
                for(int i=0; i<this->bins[y][x]->getNetNum(VERTICAL); ++i){
                    int id = this->bins[y][x]->getNetID(VERTICAL, i);
                    this->nets[id].addOverflow(1);
                }
            }
            if(this->bins[y][x]->getOverflow(HORIZONTAL)){
                for(int i=0; i<this->bins[y][x]->getNetNum(HORIZONTAL); ++i){
                    int id = this->bins[y][x]->getNetID(HORIZONTAL, i);
                    this->nets[id].addOverflow(1);
                }
            }
        }
    }
}


void GLOBALROUTE::clearLabel(){
    for(int i=0; i<this->gridyNum; ++i)
        for(int j=0; j<this->gridxNum; ++j){
            this->bins[i][j]->setLabel(0);
            this->bins[i][j]->setCost(0);
        }
}

void GLOBALROUTE::printOverflow(){
    for(int y=0; y<this->gridyNum; ++y){
        for(int x=0; x<this->gridxNum; ++x){
            int over = 0;
            if(this->bins[y][x]->getOverflow(VERTICAL)) over += 2;
            if(this->bins[y][x]->getOverflow(HORIZONTAL)) over += 3;
            if(over == 0) std::cout<<"- ";
            else if(over == 2) std::cout<<"v ";
            else if(over == 3) std::cout<<"h ";
            else if(over == 5) std::cout<<"o ";
            // std::cout<<this->bins[y][x]->getCongestion(VERTICAL)<<" ";
        }
        std::cout<<y<<std::endl;
    }
}

