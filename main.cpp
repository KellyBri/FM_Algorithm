#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include "CELL.h"
#include "NET.h"
using namespace std;

template <class T>
bool compare(T* const&one, T* const&two){
    return one->getID() < two->getID();
}

void FMAlgorithm(vector<CELL*> &, vector<NET*> &, CELL**&, CELL**&, const int&);
void UpdateGain(bool &, vector<CELL*> &, vector<NET*> &, CELL**&, CELL**&, const int&);


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
        CELL* cell = new CELL(cid, csize);
        cells.push_back( cell );
    }
    file.close();
    std::sort(cells.begin(), cells.end(), compare<CELL>);

    //read .nets NET n1 { c1 c2 ... }
    file.open(argv[2], fstream::in);
    string temp;
    file >> temp;
    CELL* cell = new CELL();
    while(temp == "NET"){
        //c: n, nid: net id, c: {, c: c
        file >> c >> nid >> c >> c;    
        NET* net = new NET(nid);
        while(c == 'c'){
            //cid: cell id
            file >> cid;     
            net->addCell(cid);
            //find cell in cell vector by cell id (binary search), and add net id into cell
            cell->setID(cid);
            cellsIter = std::lower_bound(cells.begin(), cells.end(), cell, compare<CELL>);
            (*cellsIter)->addNet(nid);
            file >> c;
        }
        nets.push_back( net );
        file >> temp;
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
    int MaxPin = 0;
    int sizeA = 0, sizeB = 0;
    for(cellsIter=cells.begin() ; cellsIter!=cells.end() ; cellsIter++){
        int t = (*cellsIter)->getPin();
        if( t > MaxPin) MaxPin = t;

        if(sizeA <= sizeB){
            (*cellsIter)->setSet(0);
            sizeA += (*cellsIter)->getSize();
        }else{
            (*cellsIter)->setSet(1);
            sizeB += (*cellsIter)->getSize();
        }
    }

    //initialize bucket list
    CELL **BucketlistA = new CELL *[2 * MaxPin + 1];
    CELL **BucketlistB = new CELL *[2 * MaxPin + 1];
    
    FMAlgorithm(cells, nets, BucketlistA, BucketlistB, MaxPin);

    return 0;
}
void FMAlgorithm(vector<CELL*> &cells, vector<NET*> &nets, CELL**&BucketlistA, CELL**&BucketlistB, const int &MaxPin){
    bool F = 0, T = 1, turn = F;
    UpdateGain(F, cells, nets, BucketlistA, BucketlistB, MaxPin);
}

void UpdateGain(bool &F, vector<CELL*> &cells, vector<NET*> &nets, CELL**&BucketlistA, CELL**&BucketlistB, const int &MaxPin){
    bool T = !F;
    //clean bucketlist
    for(int i = 0; i < 2 * MaxPin + 1; ++i){
        BucketlistA[i] = NULL;
        BucketlistB[i] = NULL;
    }
    //update nets are cut or not
    for(vector<NET*>::iterator netsIter=nets.begin() ; netsIter!=nets.end() ; netsIter++){
        bool flag = false;
        (*netsIter)->setIsCut(false);
        for(vector<int>::iterator cellNum=(*netsIter)->getCells().begin() ; cellNum!=(*netsIter)->getCells().end() ; cellNum++){
            vector<int>::iterator cellNum1 = cellNum;
            cellNum++;
            vector<int>::iterator cellNum2 = cellNum;
            if( cells[*cellNum1]->getSet() != cells[*cellNum2]->getSet() ){
                (*netsIter)->setIsCut(true);
                flag = true;
                break;
            }
        }
        if(flag) break;
    }
    //update Gain

}