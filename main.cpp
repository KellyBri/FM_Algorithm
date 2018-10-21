#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cstdlib>  //for abs
#include "CELL.h"
#include "NET.h"
using namespace std;
int TOTALSIZE = 0;
int TOTALCELLNUM = 0;
int CONSTRAINT;
int MAXPIN = 0;

template <class T>
bool compare(T* const&one, T* const&two){
    return one->getID() < two->getID();
}

void FMAlgorithm(vector<CELL*> &, vector<NET*> &, CELL**&, CELL**&);
void BuildBucklist(vector<CELL*> &, CELL**&, CELL**&);
void UpdateCut(vector<CELL*> &, vector<NET*> &);
void InitGain(vector<CELL*> &, vector<NET*> &);
void UpdateGain(vector<CELL*> &, vector<NET*> &);
CELL* FindMaxGain(CELL**&, CELL**&);


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
    while(file>>temp){
        //c: n, nid: net id, c: {, c: c
        file >> c >> nid >> c >> c;    
        NET* net = new NET(nid);
        while(c == 'c'){
            //cid: cell id
            file >> cid;     
            net->addCell(cid);
            //find cell in cell vector by cell id (binary search), and add net id into cell
            cell->setID(cid);
            cellsIter = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
            (*cellsIter)->addNet(nid);
            file >> c;
        }
        nets.push_back( net );
    }
    delete cell;
    file.close();
    std::sort(nets.begin(), nets.end(), compare<NET>);


    // for(cellsIter=cells.begin() ; cellsIter!=cells.end() ; cellsIter++){
    //     (*cellsIter)->print();
    // }

    // for(netsIter=nets.begin() ; netsIter!=nets.end() ; netsIter++){
    //     (*netsIter)->print();
    // }

    

    
    //get maximal pin number of all cells & initial partition by balanced size
    int sizeA = 0, sizeB = TOTALSIZE;
    CONSTRAINT = TOTALSIZE / 10;
    for(cellsIter=cells.begin() ; cellsIter!=cells.end() ; cellsIter++){
        int t = (*cellsIter)->getPin();
        if( t > MAXPIN) MAXPIN = t;
        
        if( abs(sizeA - sizeB) > CONSTRAINT ){
            (*cellsIter)->setSet(0);
            sizeA += (*cellsIter)->getSize();
            sizeB -= (*cellsIter)->getSize();
        }else (*cellsIter)->setSet(1);
    }

    //initialize bucket list
    CELL **BucketlistA = new CELL *[2 * MAXPIN + 1];
    CELL **BucketlistB = new CELL *[2 * MAXPIN + 1];

    FMAlgorithm(cells, nets, BucketlistA, BucketlistB);

    return 0;
}
void FMAlgorithm(vector<CELL*> &cells, vector<NET*> &nets, CELL**&BucketlistA, CELL**&BucketlistB){
    bool F, T;
    UpdateCut(cells, nets);
    InitGain(cells, nets);
    BuildBucklist(cells, BucketlistA, BucketlistB);
    vector<int> moveList;
    for(int i=0; i<TOTALCELLNUM; ++i){
        CELL *moveCell = FindMaxGain(BucketlistA, BucketlistB);
        moveCell->setLock(true);
        moveList.push_back( moveCell->getID() );
    }
}

void BuildBucklist(vector<CELL*> &cells, CELL**&BucketlistA, CELL**&BucketlistB){
    
    /* Rebuild bucketlist */
    //clean bucketlist
    for(int i = 0; i < 2 * MAXPIN + 1; ++i){
        BucketlistA[i] = NULL;
        BucketlistB[i] = NULL;
    } 
    
    //build
    vector<CELL*>::iterator cellsIter;
    for(cellsIter = cells.begin() ; cellsIter != cells.end() ; cellsIter++){
        (*cellsIter)->setNext(NULL);
        (*cellsIter)->setPre(NULL);
        //set == 1(B)
        if( (*cellsIter)->getSet() ){
            CELL *&bucket = BucketlistB[ (*cellsIter)->getGain() + MAXPIN ];
            if( !(*cellsIter)->getLock() ){
                if( bucket == NULL ){
                    bucket = (*cellsIter);
                }else{
                    bucket->setPre( (*cellsIter) );
                    (*cellsIter)->setNext(bucket);
                    bucket = (*cellsIter);
                }
            }
        //set == 0(A)
        }else{
            CELL *&bucket = BucketlistA[ (*cellsIter)->getGain() + MAXPIN ];
            if( !(*cellsIter)->getLock() ){
                if( bucket == NULL ){
                    bucket = (*cellsIter);
                }else{
                    bucket->setPre( (*cellsIter) );
                    (*cellsIter)->setNext(bucket);
                    bucket = (*cellsIter);
                }
            }
        }
    }
    // cout<<"bucket A: "<<endl;
    // for(int i = 0; i < 2 * MAXPIN + 1; ++i){
    //     if(BucketlistA[i] != NULL){
    //         CELL *temp = BucketlistA[i];
    //         while(temp->getNext() != BucketlistA[i]){
    //             temp->print();
    //             temp = temp->getNext();
    //             if(temp == NULL) break;
    //         }
    //     }
    // } 
    // cout<<"bucket B: "<<endl;
    // for(int i = 0; i < 2 * MAXPIN + 1; ++i){
    //     if(BucketlistB[i]!=NULL){
    //         CELL *temp = BucketlistB[i];
    //         while(temp->getNext() != BucketlistB[i]){
    //             temp->print();
    //             temp = temp->getNext();
    //             if(temp == NULL) break;
    //         }
    //     }
    // } 

}

/* update nets are cut or not & find distibution of each net*/
void UpdateCut(vector<CELL*> &cells, vector<NET*> &nets){
    vector<NET*>::iterator netsIter;

    for(netsIter = nets.begin() ; netsIter != nets.end() ; ++netsIter){
        bool flag = false;
        (*netsIter)->setIsCut(false);

        vector<int>::iterator cellID = (*netsIter)->getCells().begin();
        CELL *cell = new CELL( (*cellID), 0);
        vector<CELL*>::iterator cellIndex = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
        bool set = (*cellIndex)->getSet();
        int cellInB = 0; //set=1 -> B
        for(cellID ; cellID!=(*netsIter)->getCells().end() ; ++cellID){
            //find the cell & count the number of cell in set B
            cell->setID( (*cellID) );
            cellIndex = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
            if( (*cellIndex) -> getSet() ) cellInB ++;
            //check if the net is cut
            if( !flag && (*cellIndex) -> getSet()  != set ){
                (*netsIter)->setIsCut(true);
                flag = true;
            }
        }
        delete cell;
        int total = (*netsIter)->getSize();
        (*netsIter)->setDistribution( total - cellInB, cellInB);
    }
}

void InitGain(vector<CELL*> &cells, vector<NET*> &nets){
    vector<CELL*>::iterator cellsIter;
    vector<NET*>::iterator netsIter;
    vector<int>::iterator netsID;
    bool F, T;
    for(cellsIter = cells.begin(); cellsIter != cells.end(); ++cellsIter){
        F = (*cellsIter)->getSet();
        T = !F;
        int gain = 0;
        for(netsID = (*cellsIter)->getNet().begin(); netsID != (*cellsIter)->getNet().end(); ++netsID){
            if( nets[ (*netsID) - 1]->getDistribution(F) == 1 ) gain++;
            if( nets[ (*netsID) - 1]->getDistribution(T) == 0 ) gain--;
        }
        (*cellsIter)->setGain(gain);
    }
}

CELL* FindMaxGain(CELL**&BucketlistA, CELL**&BucketlistB){
    for(int i = 2*MAXPIN; i>=0; --i){
        if( (BucketlistA[i] != NULL) && (BucketlistB[i] != NULL) ) return BucketlistA[i];
        else if( (BucketlistA[i] != NULL) && (BucketlistB[i] == NULL) ) return BucketlistA[i];
        else if( (BucketlistA[i] == NULL) && (BucketlistB[i] != NULL) ) return BucketlistB[i];
    }
}