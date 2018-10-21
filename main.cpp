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
void UpdateGain(bool &, CELL*&, vector<CELL*> &, vector<NET*> &);
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


    // for(cellsIter=cells.begin() ; cellsIter!=cells.end() ; cellsIter++){
    //     (*cellsIter)->print();
    // }

    // for(netsIter=nets.begin() ; netsIter!=nets.end() ; netsIter++){
    //     (*netsIter)->print();
    // }

    

    
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
    cout<<"InitCut Done\n";
    InitGain(cells, nets);
    cout<<"InitGain Done\n";
    
    vector<int> moveList, gainList;
    for(int i=0; i<TOTALCELLNUM; ++i){
        BuildBucklist(cells, BucketlistA, BucketlistB);
        cout<<"BuildBucketlist Done\n";
        //add the cell with maximal gain to movelist
        CELL *moveCell = FindMaxGain(BucketlistA, BucketlistB);
        cout<<"FindMaxGain Done\n";
        //if the movement against the balance condition, reject the movement
        moveCell->setLock(true);
        moveList.push_back( moveCell->getID() );
        gainList.push_back( moveCell->getGain() );
        //set from, to set & move cell
        bool F = moveCell->getSet();
        /*update net status of cut & cell gains
          update gains of cells connected to moved cell*/
        UpdateGain(F, moveCell, cells, nets);
        cout<<"UpdateGain Done\n\n";
    }

    //find maximal partial sum of gains

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
        //if a cell is lock, ignore it
        if( (*cellsIter)->getLock() ) continue;
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
    // cout<<endl<<endl;

}

/* update nets are cut or not & find distibution of each net*/
void InitCut(vector<CELL*> &cells, vector<NET*> &nets){
    vector<NET*>::iterator netsIter;
    vector<int> cellInNet;
    for(netsIter = nets.begin() ; netsIter != nets.end() ; ++netsIter){
        (*netsIter)->setIsCut(false);
        CELL *cell = new CELL;
        int cellInB = 0; //set=1 -> B
        cellInNet.clear();
        cellInNet.shrink_to_fit();
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
    }
    cellInNet.clear();
    cellInNet.shrink_to_fit();
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
        net.clear();
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

void UpdateGain(bool &F, CELL* &moveCell, vector<CELL*> &cells, vector<NET*> &nets){
    vector<int> net = moveCell->getNet();
    vector<int> cellInNet;
    //check critical nets before move
    //all of nets connected to moved cell
    for(vector<int>::iterator netID = net.begin(); netID != net.end(); ++netID){
        //find net in nets
        NET *tempNet = new NET( (*netID) );
        vector<NET*>::iterator netIndex = lower_bound(nets.begin(), nets.end(), tempNet, compare<NET>);

        int TNum = (*netIndex)->getDistribution(!F);
        CELL *cell = new CELL;
        cellInNet.clear();
        cellInNet.shrink_to_fit();
        cellInNet = (*netIndex)->getCells();
        cellInNet.shrink_to_fit();

        if(TNum == 0){
            for(vector<int>::iterator cellID = cellInNet.begin() ; cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                vector<CELL*>::iterator cellIndex = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
                //update gain for all free cells in this net
                if( !(*cellIndex)->getLock() ){
                    int gain = (*cellIndex)->getGain() + 1;
                    (*cellIndex)->setGain(gain);
                }
            }
        }else if(TNum == 1){
            for(vector<int>::iterator cellID = cellInNet.begin() ; cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                vector<CELL*>::iterator cellIndex = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
                //update gain for all free cells in set T in this net
                if( !(*cellIndex)->getLock() &&  (*cellIndex)->getSet()!=F ){
                    int gain = (*cellIndex)->getGain() - 1;
                    (*cellIndex)->setGain(gain);
                }
            }
        }
        delete cell;
        delete tempNet;
    }
    cout<<"UpdateGain-1 Done!\n";
    //change F(n), T(n) to reflect the move
    UpdateCut(F, moveCell, cells, nets);
    cout<<"UpdateCut Done!\n";

    //check critical nets after move
    //all of nets connected to moved cell
    for(vector<int>::iterator netID = net.begin(); netID != net.end(); ++netID){
        //find net in nets
        NET *tempNet = new NET( (*netID) );
        vector<NET*>::iterator netIndex = lower_bound(nets.begin(), nets.end(), tempNet, compare<NET>);

        int FNum = (*netIndex)->getDistribution(F);
        CELL *cell = new CELL;
        cellInNet.clear();
        cellInNet.shrink_to_fit();
        cellInNet = (*netIndex)->getCells();
        cellInNet.shrink_to_fit();
        if(FNum == 0){
            for(vector<int>::iterator cellID = cellInNet.begin(); cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                vector<CELL*>::iterator cellIndex = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
                //update gain for all free cells in this net
                if( !(*cellIndex)->getLock() ){
                    int gain = (*cellIndex)->getGain() - 1;
                    (*cellIndex)->setGain(gain);
                }
            }
        }else if(FNum == 1){
            for(vector<int>::iterator cellID = cellInNet.begin() ; cellID != cellInNet.end(); ++cellID){
                //find the cell 
                cell->setID( (*cellID) );
                vector<CELL*>::iterator cellIndex = lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
                //update gain for all free cells in set F in this net
                if( !(*cellIndex)->getLock() &&  (*cellIndex)->getSet()==F ){
                    int gain = (*cellIndex)->getGain() + 1;
                    (*cellIndex)->setGain(gain);
                }
            }
        }
        delete cell;
        delete tempNet;
    }
    net.clear();
    net.shrink_to_fit();
    cellInNet.clear();
    cellInNet.shrink_to_fit();
}