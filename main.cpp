#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <set>
#include <cstdlib>  //for abs
#include "CELL.h"
#include "NET.h"
using namespace std;
int TOTALSIZE = 0;
int TOTALSIZE_A = 0;
int TOTALSIZE_B = 0;
int TOTALCELLNUM = 0;
int CONSTRAINT;
int MAXPIN = 0;

template <class T>
bool compare(T* const&one, T* const&two){
    return one->getID() < two->getID();
}

void FMAlgorithm(vector<CELL*> &, vector<NET*> &, CELL**&, CELL**&);
void BuildBucklist(vector<CELL*> &, CELL**&, CELL**&);

void InitCut(vector<CELL*> &, vector<NET*> &);
void InitGain(vector<CELL*> &, vector<NET*> &);

void UpdateCut(bool &, CELL*&, vector<CELL*> &, vector<NET*> &);
void UpdateGain(bool &, CELL*&, vector<CELL*> &, vector<NET*> &, CELL**&, CELL**&);


CELL* FindMaxGain(CELL**&, CELL**&);

void insertList(CELL *&, CELL *&);
void removeList(CELL *&, CELL *&);


int main(int argc, char const *argv[]){
    
    vector<CELL*> cells;
    vector<NET*> nets;
    vector<CELL*>::iterator cellsIter;
    vector<NET*>::iterator netsIter;

    //read .cells c1 10
    fstream file(argv[1], fstream::in);
    char c;
    int cid, csize, nid;
    while(file>>c){
        file >> cid >> csize;
        TOTALSIZE += csize;
        ++TOTALCELLNUM;
        CELL* cell = new CELL(cid, csize);
        cells.push_back( cell );
    }
    file.close();
    std::sort(cells.begin(), cells.end(), compare<CELL>);

    //read .nets NET n1 { c1 c2 ... }
    file.open(argv[2], fstream::in);
    string temp;
    CELL* cell = new CELL();
    set<int> tempCell;
    set<int>::iterator tempCellIter;
    while(file>>temp){
        //c: n, nid: net id, c: {, c: c
        file >> c >> nid >> c >> c;    
        NET* net = new NET(nid);
        tempCell.clear();
        while(c == 'c'){
            //cid: cell id
            file >> cid;
            tempCellIter = tempCell.find(cid);
            if( tempCellIter == tempCell.end() ){  
                tempCell.insert(cid);   
                net->addCell(cid);
                //find cell in cell vector by cell id (binary search), and add net id into cell
                cell->setID(cid);
                cellsIter = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
                (*cellsIter)->addNet(nid);
            }
            file >> c;
        }
        nets.push_back( net );
    }
    delete cell;
    file.close();
    std::sort(nets.begin(), nets.end(), compare<NET>);
    

    //get maximal pin number of all cells & initial partition by balanced size
    CONSTRAINT = TOTALSIZE / 10;
    TOTALSIZE_B = TOTALSIZE;
    for(cellsIter=cells.begin() ; cellsIter!=cells.end() ; cellsIter++){
        int t = (*cellsIter)->getPin();
        if( t > MAXPIN) MAXPIN = t;
        
        if( abs(TOTALSIZE_A - TOTALSIZE_B) > CONSTRAINT ){
            (*cellsIter)->setSet(0);
            TOTALSIZE_A += (*cellsIter)->getSize();
            TOTALSIZE_B -= (*cellsIter)->getSize();
        }else (*cellsIter)->setSet(1);
    }

    //initialize bucket list
    CELL **BucketlistA = new CELL *[2 * MAXPIN + 1];
    CELL **BucketlistB = new CELL *[2 * MAXPIN + 1];

    FMAlgorithm(cells, nets, BucketlistA, BucketlistB);

    delete[] BucketlistA;
    delete[] BucketlistB;
    return 0;
}
void FMAlgorithm(vector<CELL*> &cells, vector<NET*> &nets, CELL**&BucketlistA, CELL**&BucketlistB){
    
    InitCut(cells, nets);
    InitGain(cells, nets);
    
    vector<int> gainList;
    vector<CELL *> moveList;
    for(int i=0; i<TOTALCELLNUM; ++i){
        BuildBucklist(cells, BucketlistA, BucketlistB);

        //add the cell with maximal gain to movelist
        CELL *moveCell = FindMaxGain(BucketlistA, BucketlistB);

        //if the movement against the balance condition, reject the movement
        int gain = moveCell->getGain();
        moveCell->setLock(true);
        moveList.push_back( moveCell );
        gainList.push_back( gain );

        //set from, to set & move cell
        bool F = moveCell->getSet();
        //remove the chosen cell from bucklist
        if(F) removeList(moveCell, BucketlistB[gain+MAXPIN]);
        else  removeList(moveCell, BucketlistA[gain+MAXPIN]);

        //update gains of cells connected to moved cell
        UpdateGain(F, moveCell, cells, nets, BucketlistA, BucketlistB);
        
         
    }

    //find maximal partial sum of gains

    gainList.clear();
    gainList.shrink_to_fit();
    moveList.clear();
    moveList.shrink_to_fit();
}


void BuildBucklist(vector<CELL*> &cells, CELL**&BucketlistA, CELL**&BucketlistB){
    
    /* Rebuild bucketlist */
    //clean bucketlist
    for(int i = 0; i < 2 * MAXPIN + 1; ++i){
        BucketlistA[i] = NULL;
        BucketlistB[i] = NULL;
    } 
    
    //build
    for( vector<CELL*>::iterator cellsIter = cells.begin() ; cellsIter != cells.end() ; cellsIter++){
        (*cellsIter)->setNext(NULL);
        (*cellsIter)->setPre(NULL);

        //if a cell is lock, ignore it
        if( (*cellsIter)->getLock() ) continue;
        if( (*cellsIter)->getSet() )   //set == 1(B)
            insertList( (*cellsIter), BucketlistB[ (*cellsIter)->getGain() + MAXPIN ] );
        else    //set == 0(A)
            insertList( (*cellsIter), BucketlistA[ (*cellsIter)->getGain() + MAXPIN ] );
    }
    
}

/* update nets are cut or not & find distibution of each net*/
void InitCut(vector<CELL*> &cells, vector<NET*> &nets){
    vector<NET*>::iterator netsIter;
    vector<int> cellInNet;
    for(netsIter = nets.begin() ; netsIter != nets.end() ; ++netsIter){
        (*netsIter)->setIsCut(false);
        CELL *cell = new CELL;
        int cellInB = 0; //set=1 -> B
        cellInNet = (*netsIter)->getCells();
        cellInNet.shrink_to_fit();
        for(vector<int>::iterator cellID = cellInNet.begin() ; cellID != cellInNet.end(); ++cellID){
            //find the cell 
            cell->setID( (*cellID) );
            vector<CELL*>::iterator cellIndex = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
            //count the number of cell in set B
            if( (*cellIndex) -> getSet() ) cellInB ++;
        }
        delete cell;

        int cellInA = (*netsIter)->getSize() - cellInB;
        (*netsIter)->setDistribution(cellInA, cellInB);
        (*netsIter)->updateCut();

        cellInNet.clear();
        cellInNet.shrink_to_fit();
    }
}

void InitGain(vector<CELL*> &cells, vector<NET*> &nets){
    vector<CELL*>::iterator cellsIter;
    vector<NET*>::iterator netsIter;
    vector<int>::iterator netsID;
    vector<int> net;
    bool F, T;
    for(cellsIter = cells.begin(); cellsIter != cells.end(); ++cellsIter){
        F = (*cellsIter)->getSet();
        T = !F;
        int gain = 0;
        net = (*cellsIter)->getNet();
        net.shrink_to_fit();
        for(netsID = net.begin(); netsID != net.end(); ++netsID){
            NET *tempNet = new NET( (*netsID) );
            vector<NET*>::iterator netIndex = lower_bound(nets.begin(), nets.end(), tempNet, compare<NET>);
            if( (*netIndex)->getDistribution(F) == 1 ) gain++;
            if( (*netIndex)->getDistribution(T) == 0 ) gain--;
            delete tempNet;
        }
        (*cellsIter)->setGain(gain);
        net.clear();
        net.shrink_to_fit();
    }
}

CELL* FindMaxGain(CELL**&BucketlistA, CELL**&BucketlistB){
    for(int i = 2*MAXPIN; i>=0; --i){
        if( (BucketlistA[i] != NULL) && (BucketlistB[i] != NULL) ) return BucketlistA[i];
        else if( (BucketlistA[i] != NULL) && (BucketlistB[i] == NULL) ) return BucketlistA[i];
        else if( (BucketlistA[i] == NULL) && (BucketlistB[i] != NULL) ) return BucketlistB[i];
    }
}

/* update cut of nets connected with move cell */
void UpdateCut(bool &F, CELL* &moveCell, vector<CELL*> &cells, vector<NET*> &nets){
    bool T = !F;
    vector<int> net = moveCell->getNet();
    //all of nets connected to moved cell
    for(vector<int>::iterator netID = net.begin(); netID != net.end(); ++netID){
        //find net in nets
        NET *tempNet = new NET( (*netID) );
        vector<NET*>::iterator netIndex = lower_bound(nets.begin(), nets.end(), tempNet, compare<NET>);
        //update distribution of net
        int FNum = (*netIndex)->getDistribution(F) - 1;
        int TNum = (*netIndex)->getDistribution(T) + 1;
        (*netIndex)->setDistribution(F, FNum);
        (*netIndex)->setDistribution(T, TNum);
        (*netIndex)->updateCut();
        delete tempNet;
    }
    net.clear();
    net.shrink_to_fit();
}

void UpdateGain(bool &F, CELL* &moveCell, vector<CELL*> &cells, vector<NET*> &nets, CELL**&BucketlistA, CELL**&BucketlistB){
    vector<int> net = moveCell->getNet();
    vector<int> cellInNet;
    vector<int>::iterator cellID;
    vector<CELL*>::iterator cellIndex;
    vector<NET*>::iterator netIndex;
    CELL *cell = new CELL;
    int gain;
    //check critical nets before move
    //all of nets connected to moved cell
    for(vector<int>::iterator netID = net.begin(); netID != net.end(); ++netID){
        //find net in nets
        NET *tempNet = new NET( (*netID) );
        netIndex = lower_bound(nets.begin(), nets.end(), tempNet, compare<NET>);

        int TNum = (*netIndex)->getDistribution(!F);  
        if(TNum == 0){
            cellInNet = (*netIndex)->getCells();
            cellInNet.shrink_to_fit();
            for(cellID = cellInNet.begin() ; cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                cellIndex = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
                //if cell is lock, ignore it
                if( (*cellIndex)->getLock() ) continue;
                //update gain
                gain = (*cellIndex)->getGain();
                (*cellIndex)->setGain(gain+1);
                gain += MAXPIN;
                //update bucketlist (cell in B else A)
                if( (*cellIndex)->getSet() ){
                    removeList( (*cellIndex), BucketlistB[ gain ]);
                    insertList( (*cellIndex), BucketlistB[ gain + 1 ]);
                }else{
                    removeList( (*cellIndex), BucketlistA[ gain ]);
                    insertList( (*cellIndex), BucketlistA[ gain + 1 ]);
                }
            }
        }else if(TNum == 1){
            cellInNet = (*netIndex)->getCells();
            cellInNet.shrink_to_fit();
            for(cellID = cellInNet.begin() ; cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                cellIndex = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
                if( (*cellIndex)->getLock() || (*cellIndex)->getSet()==F ) continue;

                gain = (*cellIndex)->getGain();
                (*cellIndex)->setGain(gain-1);
                gain += MAXPIN;
                //cell in B else A
                if( (*cellIndex)->getSet() ){
                    removeList( (*cellIndex), BucketlistB[ gain ]);
                    insertList( (*cellIndex), BucketlistB[ gain - 1 ]);
                }else{
                    removeList( (*cellIndex), BucketlistA[ gain ]);
                    insertList( (*cellIndex), BucketlistA[ gain - 1 ]);
                }
            }
        }
        delete tempNet;
        cellInNet.clear();
        cellInNet.shrink_to_fit();
    }

    UpdateCut(F, moveCell, cells, nets);


    //check critical nets after move
    //all of nets connected to moved cell
    for(vector<int>::iterator netID = net.begin(); netID != net.end(); ++netID){
        //find net in nets
        NET *tempNet = new NET( (*netID) );
        netIndex = lower_bound(nets.begin(), nets.end(), tempNet, compare<NET>);

        int FNum = (*netIndex)->getDistribution(F);
        if(FNum == 0){
            cellInNet = (*netIndex)->getCells();
            cellInNet.shrink_to_fit();
            for(cellID = cellInNet.begin(); cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                cellIndex = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
                if( (*cellIndex)->getLock() ) continue;

                gain = (*cellIndex)->getGain();
                (*cellIndex)->setGain(gain-1);
                gain += MAXPIN;
                //cell in B else A
                if( (*cellIndex)->getSet() ){
                    removeList( (*cellIndex), BucketlistB[ gain ]);
                    insertList( (*cellIndex), BucketlistB[ gain - 1 ]);
                }else{
                    removeList( (*cellIndex), BucketlistA[ gain ]);
                    insertList( (*cellIndex), BucketlistA[ gain - 1 ]);
                }
            }
        }else if(FNum == 1){
            cellInNet = (*netIndex)->getCells();
            cellInNet.shrink_to_fit();
            for(cellID = cellInNet.begin() ; cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                cellIndex = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
                if( (*cellIndex)->getLock() ||  (*cellIndex)->getSet()!=F ) continue;

                gain = (*cellIndex)->getGain();
                (*cellIndex)->setGain(gain+1);
                gain += MAXPIN;
                //cell in B else A
                if( (*cellIndex)->getSet() ){
                    removeList( (*cellIndex), BucketlistB[ gain ]);
                    insertList( (*cellIndex), BucketlistB[ gain + 1 ]);
                }else{
                    removeList( (*cellIndex), BucketlistA[ gain ]);
                    insertList( (*cellIndex), BucketlistA[ gain + 1 ]);
                }
            }
        }
        delete tempNet;
        cellInNet.clear();
        cellInNet.shrink_to_fit();
    }

    UpdateCut(F, moveCell, cells, nets);
    net.clear();
    net.shrink_to_fit();
}

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