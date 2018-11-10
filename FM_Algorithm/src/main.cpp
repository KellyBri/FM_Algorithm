#include <iostream>
#include <fstream>
#include <cstdlib>  //for abs
#include <ctime>    //for time, difftime
#include <algorithm> //for sort, lower_bound
#include <vector>
#include <set>

#include "CELL.h"
#include "NET.h"

using namespace std;

int TOTALSIZE = 0;
int TOTALSIZE_A = 0;
int TOTALSIZE_B = 0;
int TOTALCELLNUM = 0;
float CONSTRAINT;
int MAXPIN = 0;

CELL **BUCKETLIST_A, **BUCKETLIST_B;
vector<CELL*> CELLS;
vector<NET*> NETS;


template <class T>
bool compare(T* const&one, T* const&two){
    return one->getID() < two->getID();
}

bool FMAlgorithm();
void BuildBucklist();

void InitCut();
void InitGain();

void UpdateSizeInPart();
void UpdateCut(bool &, CELL*&);
void UpdateGain(bool &, CELL*&);

bool SelectCell(CELL*);
CELL* FindMaxGain();

void insertList(CELL *&, CELL *&);
void removeList(CELL *&, CELL *&);


int main(int argc, char const *argv[]){
    cout.setf(ios::unitbuf);
    //get total start time  
    time_t start_time;
    time(&start_time); 

    
    time_t startIO;
    time(&startIO);
    //read .cells c1 10
    fstream file(argv[1], fstream::in);
    char c;
    int cid, csize, nid;
    while(file>>c){
        file >> cid >> csize;
        TOTALSIZE += csize;
        ++TOTALCELLNUM;
        CELL* cell = new CELL(cid, csize);
        CELLS.push_back( cell );
    }
    file.close();
    sort(CELLS.begin(), CELLS.end(), compare<CELL>);
    //get maximal pin number of all cells & initial partition by balanced size
    CONSTRAINT = 1.0*TOTALSIZE / 10;

    //read .nets NET n1 { c1 c2 ... }
    file.open(argv[2], fstream::in);
    string temp;
    CELL* cell = new CELL();
    set<int> tempCell;
    while(file>>temp){
        //c: n, nid: net id, c: {, c: c
        file >> c >> nid >> c >> c;    
        NET* net = new NET(nid);
        tempCell.clear();
        while(c == 'c'){
            //cid: cell id
            file >> cid;
            auto tempCellIter = tempCell.find(cid);
            if( tempCellIter == tempCell.end() ){  
                tempCell.insert(cid);   
                net->addCell(cid);
                //find cell in cell vector by cell id (binary search), and add net id into cell
                cell->setID(cid);
                auto cellsIter = lower_bound(CELLS.begin(), CELLS.end(), cell, compare<CELL>);
                (*cellsIter)->addNet(nid);
            }
            file >> c;
        }
        NETS.push_back( net );
    }
    delete cell;
    file.close();
    sort(NETS.begin(), NETS.end(), compare<NET>);

    time_t endIO;
    time(&endIO);
    int diffIO = difftime(endIO, startIO);

    time_t start;
    time(&start);
    //initial partition
    TOTALSIZE_B = TOTALSIZE;
    for(auto cellsIter=CELLS.begin() ; cellsIter!=CELLS.end() ; cellsIter++){
        int t = (*cellsIter)->getPin();
        if( t > MAXPIN) MAXPIN = t;
        
        if( abs(TOTALSIZE_A - TOTALSIZE_B) >= CONSTRAINT ){
            (*cellsIter)->setSet(0);
            TOTALSIZE_A += (*cellsIter)->getSize();
            TOTALSIZE_B -= (*cellsIter)->getSize();
        }else (*cellsIter)->setSet(1);
    }
    //FM algorithm
    int lastCutSize = NETS.size();
    for(int i=0; i<100; ++i){
        InitCut();
        InitGain();
        BuildBucklist();
        if( FMAlgorithm() ){
            delete[] BUCKETLIST_A;
            delete[] BUCKETLIST_B;
            break;
        }
        //update size of each partition
        UpdateSizeInPart();
        //clean bucketlist
        delete[] BUCKETLIST_A;
        delete[] BUCKETLIST_B;

        InitCut();
        int currentCutSize = 0;
        for(auto netsIter = NETS.begin(); netsIter != NETS.end(); ++netsIter ){
            (*netsIter)->updateCut();
            if( (*netsIter)->getIsCut() ) ++currentCutSize;
        }
        //cout<<"cut_size "<<currentCutSize<<"\t"<<i<<endl;
        //if ratio of improvement is less, end
        float improve = 1 - (1.0 * currentCutSize / lastCutSize);
        if(improve >= 0.001) lastCutSize = currentCutSize;
        else break;    
        
    }

    time_t end;
    time(&end);
    int diff = difftime(end, start);
    cout<<"Algo. time: "<<diff/60<<" min. "<<diff%60<<" sec."<<endl;

    time(&startIO);
    //output
    file.open(argv[3], fstream::out);
    int cutSize = 0;
    for(auto netsIter = NETS.begin(); netsIter != NETS.end(); ++netsIter ){
        (*netsIter)->updateCut();
        if( (*netsIter)->getIsCut() ) ++cutSize;
    }
    //cout<<"cut_size "<<cutSize<<endl;
    file<<"cut_size "<<cutSize<<endl;
    vector<int> cellInA, cellInB;
    for(auto cellsIter=CELLS.begin() ; cellsIter!=CELLS.end() ; cellsIter++){
        if( (*cellsIter)->getSet() ) cellInB.push_back( (*cellsIter)->getID() );
        else cellInA.push_back( (*cellsIter)->getID() );
    }
    file<<"A "<<cellInA.size()<<endl;
    for(auto it=cellInA.begin(); it!=cellInA.end(); ++it){
        file<<"c"<<(*it)<<endl;
    }
    file<<"B "<<cellInB.size()<<endl;
    for(auto it=cellInB.begin(); it!=cellInB.end(); ++it){
        file<<"c"<<(*it)<<endl;
    }
    file.close();

    time(&endIO);
    diffIO += difftime(endIO, startIO);
    cout<<"IO time: "<<diffIO/60<<" min. "<<diffIO%60<<" sec."<<endl;

    time_t end_time;
    time(&end_time);
    diff = difftime(end_time, start_time);
    cout<<"Total time: "<<diff/60<<" min. "<<diff%60<<" sec."<<endl;
    return 0;
}

/* update nets are cut or not & find distibution of each net*/
void InitCut(){
    vector<int> cellInNet;
    for(auto netsIter = NETS.begin() ; netsIter != NETS.end() ; ++netsIter){

        CELL *cell = new CELL;
        cellInNet = (*netsIter)->getCells();
        cellInNet.shrink_to_fit();

        int cellIn[2] = {0, 0}; //set=1 -> B
        for(auto cellID = cellInNet.begin() ; cellID != cellInNet.end(); ++cellID){
            //find the cell 
            cell->setID( (*cellID) );
            auto cellIndex = lower_bound(CELLS.begin(), CELLS.end(), cell, compare<CELL>);
            //count the number of cell in set B
            cellIn[ (*cellIndex)->getSet() ] ++;
        }
        (*netsIter)->setDistribution(cellIn[0], cellIn[1]);
        delete cell;
        cellInNet.clear();
        cellInNet.shrink_to_fit();
    }
}

/* calculate gains of cells */
void InitGain(){

    for(auto cellsIter = CELLS.begin(); cellsIter != CELLS.end(); ++cellsIter){
        bool F = (*cellsIter)->getSet();
        bool T = !F;
        int gain = 0;
        //unlock cell
        (*cellsIter)->setLock(0);

        vector<int> net = (*cellsIter)->getNet();
        //net.shrink_to_fit();
        for(auto netsID = net.begin(); netsID != net.end(); ++netsID){
            NET *tempNet = new NET( (*netsID) );
            auto netIndex = lower_bound(NETS.begin(), NETS.end(), tempNet, compare<NET>);
            if( (*netIndex)->getDistribution(F) == 1 ) gain++;
            if( (*netIndex)->getDistribution(T) == 0 ) gain--;
            delete tempNet;
        }
        (*cellsIter)->setGain(gain);
        net.clear();
        net.shrink_to_fit();
    }
}

/* Rebuild bucketlist */
void BuildBucklist(){

    BUCKETLIST_A = new CELL *[2 * MAXPIN + 1];
    BUCKETLIST_B = new CELL *[2 * MAXPIN + 1];

    for(int i = 0; i < 2 * MAXPIN + 1; ++i){
        BUCKETLIST_A[i] = NULL;
        BUCKETLIST_B[i] = NULL;
    } 
    
    //build
    for(auto cellsIter = CELLS.begin() ; cellsIter != CELLS.end() ; cellsIter++){
        (*cellsIter)->setNext(NULL);
        (*cellsIter)->setPre(NULL);
        bool F = (*cellsIter)->getSet();
        if(F) insertList( (*cellsIter), BUCKETLIST_B[ (*cellsIter)->getGain() + MAXPIN ] );
        else  insertList( (*cellsIter), BUCKETLIST_A[ (*cellsIter)->getGain() + MAXPIN ] );
    }
}

bool FMAlgorithm(){
    
    int maxPartialSum = (-1) * MAXPIN - 1;
    int currentPartialSum = 0;
    int moveNum = -1;
    vector<CELL *> moveList;
    for(int i=0; i<TOTALCELLNUM; ++i){
        //add the cell with maximal gain to movelist
        CELL *moveCell = FindMaxGain();
        if(moveCell == NULL) break;

        int gain = moveCell->getGain();
        moveCell->setLock(true);
        moveList.push_back( moveCell );

        //find maximal partial sum of gains
        currentPartialSum += moveCell->getGain();
        if(currentPartialSum > maxPartialSum){
            maxPartialSum = currentPartialSum;
            moveNum = i;
        }
        //set from, to set & move cell
        bool F = moveCell->getSet();
        //remove the chosen cell from bucklist
        if(F) removeList(moveCell, BUCKETLIST_B[gain+MAXPIN]);
        else  removeList(moveCell, BUCKETLIST_A[gain+MAXPIN]);
        //update gains(bucketlist) of cells connected to moved cell
        UpdateGain(F, moveCell);
    }

    if(maxPartialSum > 0 && moveNum != -1){
        //move cell(s)
        for(int i=0; i<=moveNum; ++i){
            bool from = moveList[i]->getSet();
            moveList[i]->setSet( !from );
        }    
        //clear vector
        moveList.clear();
        moveList.shrink_to_fit();
        return false;
    }
    else{
        //clear vector
        moveList.clear();
        moveList.shrink_to_fit();
        return true; //FM has finished
    }
}

/* find a cell with maximal gain */
CELL* FindMaxGain(){
    
    for(int i = 2*MAXPIN; i>=0; --i){
        if( (BUCKETLIST_A[i] != NULL) && (BUCKETLIST_B[i] != NULL) ){
            //select cell from set A first
            if( SelectCell(BUCKETLIST_A[i]) ) return BUCKETLIST_A[i];
            CELL *temp = BUCKETLIST_A[i]->getNext();
            while( temp != BUCKETLIST_A[i] && temp != NULL ){
                if( SelectCell(temp) ) return temp;
                else temp=temp->getNext();
            }
            //select cell from set B
            if( SelectCell(BUCKETLIST_B[i]) ) return BUCKETLIST_B[i];
            temp = BUCKETLIST_B[i]->getNext();
            while( temp != BUCKETLIST_B[i] && temp != NULL ){
                if( SelectCell(temp) ) return temp;
                else temp=temp->getNext();
            }
        }else if( (BUCKETLIST_A[i] != NULL) && (BUCKETLIST_B[i] == NULL) ){
            if( SelectCell(BUCKETLIST_A[i]) ) return BUCKETLIST_A[i];

            CELL *temp = BUCKETLIST_A[i]->getNext();
            while( temp != BUCKETLIST_A[i] && temp != NULL ){
                if( SelectCell(temp) ) return temp;
                else temp=temp->getNext();
            }
        }else if( (BUCKETLIST_A[i] == NULL) && (BUCKETLIST_B[i] != NULL) ){
            if( SelectCell(BUCKETLIST_B[i]) ) return BUCKETLIST_B[i];

            CELL *temp = BUCKETLIST_B[i]->getNext();
            while( temp != BUCKETLIST_B[i] && temp != NULL ){
                if( SelectCell(temp) ) return temp;
                else temp=temp->getNext();
            }
        }
    }
    return NULL;
}

/* Decide whether the cell is going to be chosen or not */
bool SelectCell(CELL* bucket){
    int sizeCell = bucket->getSize();
    if( bucket->getSet() ){
        TOTALSIZE_B -= sizeCell;
        TOTALSIZE_A += sizeCell;
    }else{
        TOTALSIZE_B += sizeCell;
        TOTALSIZE_A -= sizeCell;
    }
    //if the movement against the balance condition, reject the movement
    if( abs(TOTALSIZE_A - TOTALSIZE_B) < CONSTRAINT ) return true;
    else{
        if( bucket->getSet() ){
            TOTALSIZE_B += sizeCell;
            TOTALSIZE_A -= sizeCell;
        }else{
            TOTALSIZE_B -= sizeCell;
            TOTALSIZE_A += sizeCell;
        }
        return false;
    }
}
/* update size in both sets */
void UpdateSizeInPart(){

    TOTALSIZE_B = TOTALSIZE;
    TOTALSIZE_A = 0;
    for(auto cellsIter=CELLS.begin() ; cellsIter!=CELLS.end() ; cellsIter++){
       if( !(*cellsIter)->getSet() ){
           TOTALSIZE_A += (*cellsIter)->getSize();
           TOTALSIZE_B -= (*cellsIter)->getSize();
       }
    }
}

/* update cut of nets connected with move cell */
void UpdateCut(bool &F, CELL* &moveCell){
    bool T = !F;
    vector<int> net = moveCell->getNet();
    //all of nets connected to moved cell
    for(auto netID = net.begin(); netID != net.end(); ++netID){
        //find net in nets
        NET *tempNet = new NET( (*netID) );
        auto netIndex = lower_bound(NETS.begin(), NETS.end(), tempNet, compare<NET>);
        //update distribution of net
        int FNum = (*netIndex)->getDistribution(F) - 1;
        int TNum = (*netIndex)->getDistribution(T) + 1;
        (*netIndex)->setDistribution(F, FNum);
        (*netIndex)->setDistribution(T, TNum);
        delete tempNet;
    }
    net.clear();
    net.shrink_to_fit();
}

/* update gains of cells connected with the moving cell */
void UpdateGain(bool &F, CELL* &moveCell){

    vector<int> net = moveCell->getNet();
    vector<int> cellInNet;
    CELL *cell = new CELL;
    int gain;
    //check critical nets connected to moved cell before move
    for(auto netID = net.begin(); netID != net.end(); ++netID){
        //find net in nets
        NET *tempNet = new NET( (*netID) );
        auto netIndex = lower_bound(NETS.begin(), NETS.end(), tempNet, compare<NET>);
        bool T = !F;
        int FNum = (*netIndex)->getDistribution(F);
        int TNum = (*netIndex)->getDistribution(T); 
        if(TNum == 0){
            cellInNet = (*netIndex)->getCells();
            cellInNet.shrink_to_fit();
            for(auto cellID = cellInNet.begin() ; cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                auto cellIndex = lower_bound(CELLS.begin(), CELLS.end(), cell, compare<CELL>);
                //if cell is lock, ignore it
                if( (*cellIndex)->getLock() ) continue;
                //update gain
                gain = (*cellIndex)->getGain();
                (*cellIndex)->setGain(gain+1);
                gain += MAXPIN;
                //update bucketlist (cell in B else A)
                if( (*cellIndex)->getSet() ){
                    removeList( (*cellIndex), BUCKETLIST_B[ gain ]);
                    insertList( (*cellIndex), BUCKETLIST_B[ gain + 1 ]);
                }else{
                    removeList( (*cellIndex), BUCKETLIST_A[ gain ]);
                    insertList( (*cellIndex), BUCKETLIST_A[ gain + 1 ]);
                }
            }
        }else if(TNum == 1){
            cellInNet = (*netIndex)->getCells();
            cellInNet.shrink_to_fit();
            for(auto cellID = cellInNet.begin() ; cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                auto cellIndex = lower_bound(CELLS.begin(), CELLS.end(), cell, compare<CELL>);
                if( (*cellIndex)->getLock() || (*cellIndex)->getSet()==F ) continue;

                gain = (*cellIndex)->getGain();
                (*cellIndex)->setGain(gain-1);
                gain += MAXPIN;
                //cell in B else A
                if( (*cellIndex)->getSet() ){
                    removeList( (*cellIndex), BUCKETLIST_B[ gain ]);
                    insertList( (*cellIndex), BUCKETLIST_B[ gain - 1 ]);
                }else{
                    removeList( (*cellIndex), BUCKETLIST_A[ gain ]);
                    insertList( (*cellIndex), BUCKETLIST_A[ gain - 1 ]);
                }
            }
        }
        //update distribution of net
        --FNum; ++TNum;
        (*netIndex)->setDistribution(F, FNum);
        (*netIndex)->setDistribution(T, TNum);

        delete tempNet;
        cellInNet.clear();
        cellInNet.shrink_to_fit();
    }

    //check critical nets after move
    //all of nets connected to moved cell
    for(vector<int>::iterator netID = net.begin(); netID != net.end(); ++netID){
        //find net in nets
        NET *tempNet = new NET( (*netID) );
        auto netIndex = lower_bound(NETS.begin(), NETS.end(), tempNet, compare<NET>);

        int FNum = (*netIndex)->getDistribution(F);
        if(FNum == 0){
            cellInNet = (*netIndex)->getCells();
            cellInNet.shrink_to_fit();
            for(auto cellID = cellInNet.begin(); cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                auto cellIndex = lower_bound(CELLS.begin(), CELLS.end(), cell, compare<CELL>);
                if( (*cellIndex)->getLock() ) continue;

                gain = (*cellIndex)->getGain();
                (*cellIndex)->setGain(gain-1);
                gain += MAXPIN;
                //cell in B else A
                if( (*cellIndex)->getSet() ){
                    removeList( (*cellIndex), BUCKETLIST_B[ gain ]);
                    insertList( (*cellIndex), BUCKETLIST_B[ gain - 1 ]);
                }else{
                    removeList( (*cellIndex), BUCKETLIST_A[ gain ]);
                    insertList( (*cellIndex), BUCKETLIST_A[ gain - 1 ]);
                }
            }
        }else if(FNum == 1){
            cellInNet = (*netIndex)->getCells();
            cellInNet.shrink_to_fit();
            for(auto cellID = cellInNet.begin() ; cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                auto cellIndex = lower_bound(CELLS.begin(), CELLS.end(), cell, compare<CELL>);
                if( (*cellIndex)->getLock() ||  (*cellIndex)->getSet()!=F ) continue;

                gain = (*cellIndex)->getGain();
                (*cellIndex)->setGain(gain+1);
                gain += MAXPIN;
                //cell in B else A
                if( (*cellIndex)->getSet() ){
                    removeList( (*cellIndex), BUCKETLIST_B[ gain ]);
                    insertList( (*cellIndex), BUCKETLIST_B[ gain + 1 ]);
                }else{
                    removeList( (*cellIndex), BUCKETLIST_A[ gain ]);
                    insertList( (*cellIndex), BUCKETLIST_A[ gain + 1 ]);
                }
            }
        }
        delete tempNet;
        cellInNet.clear();
        cellInNet.shrink_to_fit();
    }

    delete cell;
    net.clear();
    net.shrink_to_fit();
}

/* insert element to double linked list */
void insertList(CELL *&cell, CELL *&bucket){
    if( bucket == NULL ){
        bucket = cell;
    }else if( bucket->getNext()==NULL && bucket->getPre()==NULL ){
        bucket->setNext( cell );
        bucket->setPre( cell);
        cell->setNext( bucket );
        cell->setPre( bucket );
    }else{
        cell->setNext( bucket->getNext() );
        cell->setPre( bucket );
        bucket->getNext()->setPre( cell );
        bucket->setNext( cell );
    }
}
/* remove element from double linked list */
void removeList(CELL *&cell, CELL *&bucket){
    if( cell->getNext()==NULL && cell->getPre()==NULL ){
        bucket = NULL;
    }else if( cell->getNext()==cell->getPre() ){
        bucket = cell->getNext();
        cell->setNext(NULL);
        cell->setPre(NULL);
        bucket->setNext(NULL);
        bucket->setPre(NULL);
    }else{
        bucket = cell->getNext();
        bucket->setPre( cell->getPre() );
        bucket->getPre()->setNext(bucket);
        cell->setNext(NULL);
        cell->setPre(NULL);
    }
}