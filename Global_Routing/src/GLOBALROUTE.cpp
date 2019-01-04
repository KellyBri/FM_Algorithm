#include "GLOBALROUTE.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>
// #include <set>
// #include <cmath>


GLOBALROUTE::GLOBALROUTE(int gx, int gy, int vc, int hc):
                        gridxNum(gx), gridyNum(gy), verticalCap(vc), horizontalCap(hc){
    this->netNum = 0;
    for(int i=0; i<gy; ++i){
        std::vector<BIN *> temp;
        for(int j=0; j<gx; ++j){
            BIN *bin;
            if(i == gy-1 && j == gx-1) bin = new BIN(j, i);
            else if(i == gy-1) bin = new BIN(j, i, hc, false);
            else if(j == gx-1) bin = new BIN(j, i, vc, true);
            else bin = new BIN(j, i, vc, hc);
    
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
    for(int i=0; i<this->netNum; ++i){
        delete this->nets[i];
    }

}

void GLOBALROUTE::addNet(NET* net){
    this->netNum++;
    this->nets.push_back(net);
}

int GLOBALROUTE::getNetNum(){ return this->netNum; }
NET* GLOBALROUTE::getNet(int i){ return nets[i]; }



/*********************************/
/**************private************/
/*********************************/

void GLOBALROUTE::route(){
    
    // std::cout<<"/***********************/\n";
    // std::cout<<"/**** Start phase 1 ****/\n";
    //route all of the nets onto grid without considering capacity
    phase1();

        // for(int y=0; y<this->gridyNum; ++y){
        //     for(int x=0; x<this->gridxNum; ++x){
        //         if( this->bins[y][x]->getDemand(VERTICAL)==0 ) std::cout<<"- ";
        //         else std::cout<<this->bins[y][x]->getDemand(VERTICAL)<<" ";
        //         // if( this->bins[y][x]->getDemand(HORIZONTAL)==0 ) std::cout<<"- ";
        //         // else std::cout<<this->bins[y][x]->getDemand(HORIZONTAL)<<" ";
        //     }
        //     std::cout<<std::endl;
        // }
        // std::cout<<std::endl;
        // std::cout<<std::endl;
        // std::cout<<std::endl;
        
    // printOverflow();

    //rip off
    // std::cout<<"/**** Start phase 2 ****/\n";
    phase2();
  
    // int count = 0;
    // for(int i=0; i<this->netNum; ++i){
    //     if(this->nets[i]->getOverflow() == 0) ++count;
    //     // std::cout<<"ID: "<<this->nets[i]->getID()<<"\toverflow: "<<this->nets[i]->getOverflow()<<std::endl;
    // }
    // std::cout<<"# of net without overflow: " << count << " / " << this->netNum <<std::endl;

    //re-route
    // std::cout<<"/**** Start phase 3 ****/\n";
    // std::cout<<"/***********************/\n";
    // printOverflow();
}

//implement Lee algorithm with A*-search
void GLOBALROUTE::phase1(){
    
    //Lee algorithm
    for(int i=0; i<this->netNum; ++i){
        route(i);
        //for debugging
        // if(i==12747){
        //     for(int a=0; a<this->gridyNum; ++a){
        //         for(int j=0; j<this->gridxNum; ++j){
        //             if(a==startY && j == startX) std::cout<<"S  ";
        //             else if(a==targetY && j == targetX) std::cout<<"T  ";
        //             else if( this->bins[a][j]->getLabel() + this->bins[a][j]->getCost() == 0 ) std::cout<<"-  ";
        //             else std::cout<<this->bins[a][j]->getLabel() + this->bins[a][j]->getCost()<<" ";
        //         }
        //         std::cout<<a<<std::endl;
        //     }
        //     std::cout<<"Start ("<<startX<<", "<<startY<<")"<<std::endl;
        //     std::cout<<"End   ("<<targetX<<", "<<targetY<<")"<<std::endl;
        //     for(int a=0; a<this->nets[i]->getPathNum(); ++a){
        //         std::cout<<"("<<this->nets[i]->getPathX(a)<<", "<<this->nets[i]->getPathY(a)<<")"<<std::endl;
        //     }
        // }
        // std::cout << i << std::endl;
        // if(i==0) break;
        //count number of overflow bin on the path
    }

    //count number of overflow bin on the path
    for(int y=0; y<this->gridyNum; ++y){
        for(int x=0; x<this->gridxNum; ++x){
            if( this->bins[y][x]->getOverflow(VERTICAL) ){
                // if(this->bins[y][x]->getOverflow(VERTICAL))
                // if(this->bins[y][x]->getDemand(VERTICAL)>this->verticalCap)
                for(int i=0; i<this->bins[y][x]->getNetNum(VERTICAL); ++i){
                    int id = this->bins[y][x]->getNetID(VERTICAL, i);
                    this->nets[id]->addOverflow(1);
                }
            }
            if( this->bins[y][x]->getOverflow(HORIZONTAL) ){
                // if(this->bins[y][x]->getDemand(HORIZONTAL)>this->horizontalCap)
                // if(this->bins[y][x]->getOverflow(HORIZONTAL))
                for(int i=0; i<this->bins[y][x]->getNetNum(HORIZONTAL); ++i){
                    int id = this->bins[y][x]->getNetID(HORIZONTAL, i);
                    this->nets[id]->addOverflow(1);
                }
            }
        }
    }
}

int myrandom (int i) { return std::rand()%i;}
void GLOBALROUTE::phase2(){

    //rip-off & re-route
    int iteration = 0, over = 0, count = 0;
    auto comp  = [](NET* a, NET* b) -> bool{ return a->getOverflow() < b->getOverflow(); };
    // auto comp2 = [](NET* a, NET* b) -> bool{ return a->getBoundBox() > b->getBoundBox(); };
    std::priority_queue< NET*, std::vector<NET*>, decltype( comp )  > pqNET_ripup(comp);
    // std::priority_queue< NET*, std::vector<NET*>, decltype( comp2 ) > pqNET_route(comp2);
    std::vector<NET *> pqNET_route;
    unsigned seed = time(NULL);
    srand(seed);
    std::cout<<std::endl<<std::endl<<"Seed = "<<seed<<std::endl;
    while( time(NULL) - seed < 200 ){
        addHistory();
        // if( time(NULL) - seed > 550 ) break;
        //push all of nets with non-zero overflow into priority_queue
        count = 0;
        for(int i=0; i<this->netNum; ++i){
            if(this->nets[i]->getOverflow() > 0){
                pqNET_ripup.push(this->nets[i]);
                // pqNET_route.push(this->nets[i]);
                pqNET_route.push_back(this->nets[i]);
                ++count;
            }    
        }
        while( !pqNET_ripup.empty() ){
            NET* top = pqNET_ripup.top();
            ripup ( top->getID() );
            pqNET_ripup.pop();
        }
        // std::cout<<"ripup done\n";
        std::random_shuffle( pqNET_route.begin(), pqNET_route.end(), myrandom );
        while( !pqNET_route.empty() ){
            NET* top = pqNET_route.back();
            // NET* top = pqNET_route.top();
            reRoute( top->getID() );
            pqNET_route.pop_back();
            // pqNET_route.pop();
        }
        // std::cout<<"reroute done\n";
        
        for(int i=0; i<this->netNum; ++i) this->nets[i]->resetOverflow();
        

        over = 0;
        for(int y=0; y<this->gridyNum; ++y){
            for(int x=0; x<this->gridxNum; ++x){
                if( this->bins[y][x]->getOverflow(VERTICAL) ){
                    over += this->bins[y][x]->getDemand(VERTICAL) - this->bins[y][x]->getSupply(VERTICAL);
                    for(int i=0; i<this->bins[y][x]->getNetNum(VERTICAL); ++i){
                        int id = this->bins[y][x]->getNetID(VERTICAL, i);
                        this->nets[id]->addOverflow(1);
                    }
                }
                if( this->bins[y][x]->getOverflow(HORIZONTAL) ){
                    over += this->bins[y][x]->getDemand(HORIZONTAL) - this->bins[y][x]->getSupply(HORIZONTAL);
                    for(int i=0; i<this->bins[y][x]->getNetNum(HORIZONTAL); ++i){
                        int id = this->bins[y][x]->getNetID(HORIZONTAL, i);
                        this->nets[id]->addOverflow(1);
                    }
                }
            }
        }

        
        // std::cout<<"Iteration: "<<iteration<<"\tnet = "<<count<<"\toverflow = "<<over<<std::endl;
        // if( count == 0 || over == 0 || ( this->netNum == 27781 && over < 2000 ) ) break;
        if( count == 0 || over == 0 ) break;
        ++iteration;
    }
    // std::cout<<"Iteration: "<<iteration<<"\tnet = "<<count<<"\toverflow = "<<over<<std::endl;
}

void GLOBALROUTE::route(int i){
    int x, y;
    int tempX, tempY;
    int startX,  startY;
    int targetX, targetY;
    int increamentX, increamentY;

    clearLabel();
    x = startX  = this->nets[i]->getPinX(0);
    y = startY  = this->nets[i]->getPinY(0);
    targetX = this->nets[i]->getPinX(1);
    targetY = this->nets[i]->getPinY(1);

    
    increamentX = increamentY = 1;
    if( startX > targetX ) increamentX = -1;
    if( startY > targetY ) increamentY = -1;

    auto comp = [](BIN* a, BIN* b) -> bool{ return a->getLabel()+a->getCost() > b->getLabel()+b->getCost(); };
    std::priority_queue< BIN*, std::vector<BIN*>, decltype( comp ) > pqBin(comp);
    pqBin.push( this->bins[startY][startX] );
    BIN *tempBin;
    //Propagation
    while( x != targetX || y != targetY ){
        // computing label g(x)
        // computing cost  h(x) = max( distToTarget_x, distToTarget_y)
        // f(x) = g(x) + h(x)
        tempBin = pqBin.top();
        pqBin.pop();
        int label, curLabel;
        // double cong, cost, curCost, curCong;
        double cost, curCost;
        x = tempBin->getX();
        y = tempBin->getY();
        tempX = x + increamentX;
        tempY = y + increamentY;
        label = tempBin->getLabel() + 1;
        cost  = tempBin->getCost();
        // cong  = tempBin->getCong();
        if(tempX >= 0 && tempX < this->gridxNum){
            curLabel = this->bins[y][tempX]->getLabel();
            curCost  = this->bins[y][tempX]->getCost();

            
            // if( increamentX ) curCong = this->bins[y][x]->getCongestion(HORIZONTAL);
            // else curCong = this->bins[y][tempX]->getCongestion(HORIZONTAL);
            
            if(this->bins[y][tempX]->getParentX() == -1 || curLabel + curCost > label + cost){
                this->bins[y][tempX]->setLabel(label);
                this->bins[y][tempX]->setCost( std::max( abs(targetX-tempX), abs(targetY-y) ) );
                // this->bins[y][tempX]->setCong(curCong);
                this->bins[y][tempX]->setParent(x, y);
                pqBin.push(this->bins[y][tempX]);
            }
        }
        if(tempY >= 0 && tempY < this->gridyNum){
            curLabel = this->bins[tempY][x]->getLabel();
            curCost  = this->bins[tempY][x]->getCost();

            // if( increamentY ) curCong = this->bins[y][x]->getCongestion(VERTICAL);
            // else curCong = this->bins[tempY][x]->getCongestion(VERTICAL);

            if(this->bins[tempY][x]->getParentX() == -1 || curLabel + curCost > label + cost){
                this->bins[tempY][x]->setLabel(label);
                this->bins[tempY][x]->setCost( std::max( abs(targetX-x), abs(targetY-tempY) ) );
                // this->bins[tempY][x]->setCong(curCong);
                this->bins[tempY][x]->setParent(x, y);
                pqBin.push(this->bins[tempY][x]);
            }
        }
    }
    //Re-trace
    bool firstStep = true;
    bool direction, tempDirection;
    direction = tempDirection = VERTICAL;

    x = targetX; 
    y = targetY;
    this->nets[i]->addPath(targetX, targetY);
    while( x != startX || y != startY ){
        tempX = this->bins[y][x]->getParentX();
        tempY = this->bins[y][x]->getParentY();

        //if change direction, then add it to the path
        if(tempX == x) tempDirection = VERTICAL;
        else tempDirection = HORIZONTAL;

        if(firstStep) firstStep = false;
        else if(tempDirection != direction) this->nets[i]->addPath(x, y);

        //occupy this net to the edge
        if( x+1 == tempX || y+1 == tempY ) this->bins[y][x]->setEdge(tempDirection, i);
        else if( x-1 == tempX ) this->bins[y][tempX]->setEdge(tempDirection, i);
        else if( y-1 == tempY ) this->bins[tempY][x]->setEdge(tempDirection, i);
        
        //next
        x = tempX;
        y = tempY;
        direction = tempDirection;
        
    }
    //add start point
    this->nets[i]->addPath(startX, startY);
}

void GLOBALROUTE::ripup(int id){

    int x1, x2, y1, y2;

    //update demand of each edge on the path
    for(int i=0; i<this->nets[id]->getPathNum()-1; ++i){
        x1 = this->nets[id]->getPathX(i);
        y1 = this->nets[id]->getPathY(i);
        x2 = this->nets[id]->getPathX(i+1);
        y2 = this->nets[id]->getPathY(i+1);

        if(x1 == x2){
            if(y1 > y2) std::swap(y1, y2);
            for(int y = y1; y < y2; ++y){
                bool lastOverflow = this->bins[y][x1]->getOverflow(VERTICAL);
                this->bins[y][x1]->subDemand(VERTICAL, 1, id);
                if( lastOverflow && !this->bins[y][x1]->getOverflow(VERTICAL) )
                for(int i=0; i<this->bins[y][x1]->getNetNum(VERTICAL); ++i){
                    int id = this->bins[y][x1]->getNetID(VERTICAL, i);
                    this->nets[id]->addOverflow(-1);
                }
            }
        }
        else if(y1 == y2){
            if(x1 > x2) std::swap(x1, x2);
            for(int x = x1; x < x2; ++x){
                bool lastOverflow = this->bins[y1][x]->getOverflow(HORIZONTAL);
                this->bins[y1][x]->subDemand(HORIZONTAL, 1, id);
                if( lastOverflow && !this->bins[y1][x]->getOverflow(HORIZONTAL) )
                for(int i=0; i<this->bins[y1][x]->getNetNum(HORIZONTAL); ++i){
                    int id = this->bins[y1][x]->getNetID(HORIZONTAL, i);
                    this->nets[id]->addOverflow(-1);
                }
            }
        }
    }
    this->nets[id]->ripoff();
}

void GLOBALROUTE::reRoute(int id){

    clearLabel();
    int x, y, startX,  startY, targetX, targetY, tempX, tempY;
    
    x = startX  = this->nets[id]->getPinX(0);
    y = startY  = this->nets[id]->getPinY(0);
    targetX = this->nets[id]->getPinX(1);
    targetY = this->nets[id]->getPinY(1);


    auto comp = [](BIN* a, BIN* b) -> bool{ return a->getCost() > b->getCost(); };
    std::priority_queue< BIN*, std::vector<BIN*>, decltype( comp ) > pqBin(comp);
    pqBin.push( this->bins[startY][startX] );
    this->bins[startY][startX]->setInQueue(true);

    // BIN *tempBin = pqBin.top();
    // pqBin.pop();
    
    // x = tempBin->getX();
    // y = tempBin->getY();
    double curCost, cost;
    BIN *tempBin;

    //Propagation
    // std::cout<<"Hi\t"<<std::flush;
    while( x != targetX || y != targetY ){
        
        //next
        tempBin = pqBin.top();
        pqBin.pop();
        
        x = tempBin->getX();
        y = tempBin->getY();

        this->bins[y][x]->setInQueue(false);
        
        if( x-1 >= 0 ){

            curCost = this->bins[y][x-1]->getCost();
            // std::cout<<"Hi x-1\n";
            cost    = this->bins[y][x]->getCost() + this->bins[y][x-1]->getHistory( HORIZONTAL ) * this->bins[y][x-1]->getCongestion( HORIZONTAL );

            // if( curCost < 0 ) std::cout<<"x-1 cost\n";
            // if( this->bins[y][x-1]->getHistory( HORIZONTAL ) < 0 ) std::cout<<"x-1 history\n";
            // if( this->bins[y][x-1]->getCongestion( HORIZONTAL ) < 0 ) std::cout<<"x-1 congestion\n";

            // std::cout<<"Bye x-1\n";
            if(this->bins[y][x-1]->getParentX() == -1 || curCost > cost ){
                this->bins[y][x-1]->setCost( cost );
                this->bins[y][x-1]->setParent(x, y);
                if( !this->bins[y][x-1]->getInQueue() ){
                    this->bins[y][x-1]->setInQueue(true);
                    pqBin.push(this->bins[y][x-1]);
                }
            }
        }
        if( x+1 < this->gridxNum ){
            // std::cout<<"Hi x+1\n";
            curCost = this->bins[y][x+1]->getCost();
            cost    = this->bins[y][x]->getCost() + this->bins[y][x]->getHistory( HORIZONTAL ) * this->bins[y][x]->getCongestion( HORIZONTAL );
            // if( curCost < 0 ) std::cout<<"x+1 cost\n";
            // if( this->bins[y][x]->getHistory( HORIZONTAL ) < 0 ) std::cout<<"x+1 history\n";
            // if( this->bins[y][x]->getCongestion( HORIZONTAL ) < 0 ) std::cout<<"x+1 congestion\n";
            // std::cout<<"Bye x+1\n";
            if(this->bins[y][x+1]->getParentX() == -1 || curCost > cost ){
                this->bins[y][x+1]->setCost( cost );
                this->bins[y][x+1]->setParent(x, y);
                if( !this->bins[y][x+1]->getInQueue() ){
                    this->bins[y][x+1]->setInQueue(true);
                    pqBin.push(this->bins[y][x+1]);
                }
            }
        }
        if( y-1 >= 0 ){
            // std::cout<<"Hi y-1\n";
            curCost = this->bins[y-1][x]->getCost();
            cost    = this->bins[y][x]->getCost() + this->bins[y-1][x]->getHistory( VERTICAL ) * this->bins[y-1][x]->getCongestion( VERTICAL );
            
            // if( curCost < 0 ) std::cout<<"y-1 cost\n";
            // if( this->bins[y-1][x]->getHistory( VERTICAL ) < 0 ) std::cout<<"y-1 history\n";
            // if( this->bins[y-1][x]->getCongestion( VERTICAL ) < 0 ) std::cout<<"y-1 congestion\n";

            // std::cout<<"Bye y-1\n";
            if(this->bins[y-1][x]->getParentX() == -1 || curCost > cost ){
                this->bins[y-1][x]->setCost( cost );
                this->bins[y-1][x]->setParent(x, y);
                if( !this->bins[y-1][x]->getInQueue() ){
                    this->bins[y-1][x]->setInQueue(true);
                    pqBin.push(this->bins[y-1][x]);
                }
            }
        }
        if( y+1 < this->gridyNum ){
            // std::cout<<"Hi y+1\n";
            curCost = this->bins[y+1][x]->getCost();
            cost    = this->bins[y][x]->getCost() + this->bins[y][x]->getHistory( VERTICAL ) * this->bins[y][x]->getCongestion( VERTICAL );
            
            // if( curCost < 0 ) std::cout<<"y+1 cost\n";
            // if( this->bins[y][x]->getHistory( VERTICAL ) < 0 ) std::cout<<"y+1 history\n";
            // if( this->bins[y][x]->getCongestion( VERTICAL ) < 0 ) std::cout<<"y+1 congestion\n";
            
            // std::cout<<"Bye y+1\n";
            if(this->bins[y+1][x]->getParentX() == -1 || curCost > cost ){
                this->bins[y+1][x]->setCost( cost );
                this->bins[y+1][x]->setParent(x, y);
                if( !this->bins[y+1][x]->getInQueue() ){
                    this->bins[y+1][x]->setInQueue(true);
                    pqBin.push(this->bins[y+1][x]);
                }
            }
        }
        
        
    }
    // std::cout<<"Bye\t"<<std::flush;
    // for(int y=0; y<this->gridyNum; ++y){
    //      for(int x=0; x<this->gridxNum; ++x){
    //          if( x==startX && y==startY) std::cout<<"S"<<"("<<this->bins[y][x]->getParentX()<<","<<this->bins[y][x]->getParentY()<<") ";
    //          else if(x==targetX && y==targetY) std::cout<<"T"<<"("<<this->bins[y][x]->getParentX()<<","<<this->bins[y][x]->getParentY()<<") ";
    //          if( this->bins[y][x]->getParentX() == -1 ) std::cout<<"- ";
    //          else std::cout<<"("<<this->bins[y][x]->getParentX()<<","<<this->bins[y][x]->getParentY()<<") ";
    //      }
    //      std::cout<<std::endl;
    //  }
    //  std::cout<<std::endl;
    //  std::cout<<std::endl;
    //  std::cout<<std::endl;
    
    //Re-trace
    bool firstStep = true;
    bool direction, tempDirection;
    direction = tempDirection = VERTICAL;

    x = targetX; 
    y = targetY;
    this->nets[id]->addPath(targetX, targetY);
    //  std::cout<<"Hi retrace\t"<<std::flush;
    while( x != startX || y != startY ){
        tempX = this->bins[y][x]->getParentX();
        tempY = this->bins[y][x]->getParentY();

        //if change direction, then add it to the path
        if(tempX == x) tempDirection = VERTICAL;
        else tempDirection = HORIZONTAL;

        if(firstStep) firstStep = false;
        else if(tempDirection != direction) this->nets[id]->addPath(x, y);

        //occupy this net to the edge
        if( x+1 == tempX || y+1 == tempY ) this->bins[y][x]->setEdge(tempDirection, id);
        else if( x-1 == tempX ) this->bins[y][tempX]->setEdge(tempDirection, id);
        else if( y-1 == tempY ) this->bins[tempY][x]->setEdge(tempDirection, id);

        //next
        x = tempX;
        y = tempY;
        direction = tempDirection;

    }
    //add start point
    this->nets[id]->addPath(startX, startY);
    //  std::cout<<"Bye retrace\n"<<std::flush;
    // std::cout<<id<<std::endl;
}

void GLOBALROUTE::addHistory(){
    for(int y=0; y<this->gridyNum; ++y){
        for(int x=0; x<this->gridxNum; ++x){
            this->bins[y][x]->addHistory(VERTICAL);
            this->bins[y][x]->addHistory(HORIZONTAL);
        }
    }
}


void GLOBALROUTE::clearLabel(){
    for(int i=0; i<this->gridyNum; ++i)
        for(int j=0; j<this->gridxNum; ++j){
            this->bins[i][j]->setLabel(0);
            this->bins[i][j]->setCost(0);
            this->bins[i][j]->setParent(-1,-1);
            this->bins[i][j]->setInQueue(false);
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

