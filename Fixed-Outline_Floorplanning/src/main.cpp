#include <iostream>
#include <fstream>  //for file read/write
#include <cstdlib>  //for atof
#include <vector>
#include <cmath>

#include "NET.h"
#include "BLOCK.h"
#include "TERMINAL.h"

#define R 0
#define L 1

#ifndef PL
#define PL 0    //PL stands for TERMINAL
#endif

#ifndef SB
#define SB 1    //SB stands for hardblock
#endif

int TOTAL_AREA = 0;
double WHITE_Ratio;
int REGION_Side;
int *X_CONTOUR, *Y_CONTOUR;

std::vector<BLOCK *> BLOCKS;
std::vector<NET *> NETS;
std::vector<TERMINAL *> TERMINALS;

void Floorplan();
void InitialBTree();
int UpdateContour(BLOCK *);

int main(int argc, char const **argv){
    
    if(argc != 6){
        std::cout<<"Error: The number of arguments is wrong!\n";
        std::cout<<"Three input files, white space ratio and a output file must be given!\n";
        return -1;
    }
    /************************** read .hardblocks file *****************************/
    std::fstream file(argv[1], std::ifstream::in);
    if( !file.is_open() ){
        std::cout<<"Error: Failed to open .hardblocks input file!\n";
        return -1;
    }
    //read first line of .hardblocks file
    std::string text;
    file >> text;
    if(text != "NumHardRectilinearBlocks"){
        std::cout<<"Error: The format of input file is wrong!\n";
        file.close();
        return -1;
    }
    char c;
    int blockNum;
    file >> c >> blockNum; //read a ':' and a number

    //read second line of .hardblocks file
    file >> text;
    if(text != "NumTerminals"){
        std::cout<<"Error: The format of input file .hardblocks is wrong!\n";
        file.close();
        return -1;
    }
    int terminalNum;
    file >> c >> terminalNum; //read a ':' and a number

    
    //read BLOCKS
    for(int i=0; i<blockNum; ++i){
        file >> c >> c; //read 's' and 'b'
        
        int num;
        file >> num;    //read sb "number"
        BLOCK *temp = new BLOCK(num);

        file >> text >> num;   //read "hardrectilinear" and vertex "number"
        temp->setVertexNum(num);
        TOTAL_AREA += temp->getArea();

        int x, y;
        for(int j=0; j<num; ++j){
            file >> c >> x >> c >> y >> c;    //read '(', x, ',', y, ')'
            temp->addVertex(x, y);
        }
        temp->calcWidthHeight();
        BLOCKS.push_back(temp);
    }
    // for(auto it = BLOCKS.begin(); it != BLOCKS.end(); ++it)
    //     (*it)->print();

    // //read terminals (p)
    // for(int i=0; i<terminalNum; ++i){
    //     int num;
    //     file >> c >> num >> text;
    //     TERMINALS.push_back( new TERMINAL(num) );
    // }
    // for(auto it = TERMINALS.begin(); it != TERMINALS.end(); ++it)
    //     (*it)->print();
    file.close();



    /******************************* read .nets file **********************************/
    file.open(argv[2], std::ifstream::in);
    if( !file.is_open() ){
        std::cout<<"Error: Failed to open .nets input file!\n";
        return -1;
    }

    
    file >> text;
    if(text != "NumNets"){
        std::cout<<"Error: The format of input file .nets is wrong!\n";
        file.close();
        return -1;
    }

    int num, netNum;
    file >> c >> netNum;
    file >> text >> c >> num;   //read "NumPins", ':', pinNum
    //read NETS
    for(int i=0; i<netNum; ++i){
        int degree;
        NET* temp = new NET(i);
        file >> text >> c >> degree;   //read "NetDegree", ':', 2
        temp->setDegree(degree);
        for(int j=0; j<degree; ++j){
            file >> c;
            if(c == 'p'){
                file >> num;
                temp->addNode(PL, num);
            }else{
                file >> c >> num;
                temp->addNode(SB, num);
            }
        }
        NETS.push_back(temp);
    }
    // for(auto it = NETS.begin(); it != NETS.end(); ++it)
    //     (*it)->print();
    file.close();


    /****************************** read .pl file *********************************/
    file.open(argv[3], std::ifstream::in);
    if( !file.is_open() ){
        std::cout<<"Error: Failed to open .pl input file!\n";
        return -1;
    }
    //read terminals (p)
    for(int i=0; i<terminalNum; ++i){
        int id, x, y;
        file >> c >> id >> x >> y;
        TERMINALS.push_back( new TERMINAL(id, x, y) );
    }
    // for(auto it = TERMINALS.begin(); it != TERMINALS.end(); ++it)
    //     (*it)->print();
    file.close();


    WHITE_Ratio = atof(argv[4]);
    REGION_Side = sqrt( TOTAL_AREA * (1+WHITE_Ratio) );
    

    Floorplan();


    delete[] X_CONTOUR;
    delete[] Y_CONTOUR;
    //output file
    file.open(argv[5], std::fstream::out);

    file.close();
    return 0;
}

void Floorplan(){
    //Initialize contour
    //X_CONTOUR = new int[REGION_Side];
    Y_CONTOUR = new int[REGION_Side];

    //Initialize B*-tree with input blocks
    InitialBTree();
    
    //Adaptive fasr Simulated Annealing
    int T = 0;  //initial temperature
    while(true){
        //perturb the B*-tree
        //pack macro blocks
        //evaluate the B*-tree cost
        //decide if we should accept the new B*-tree
        //modify the weights in the cost function
        //update T
        break;
    }
}

/* Update contour due to adding new block */
void UpdateContour(const int &x, const int &y, const int width, const int height, int &max_y){

    for(int i=x; i < x+width; ++i){
        Y_CONTOUR[i] = y + height;
    }
    if( max_y < y + height) max_y = y + height;
}

/* Initial B*-tree with input blocks */
void InitialBTree(){

    int temp_x = 0, temp_y = 0;
    int max_y = 0;
    BLOCK *leftMostBlock, *rightMostBlock, *root = NULL;

    //for each block in each net
    for(auto netIt = NETS.begin(); netIt != NETS.end(); ++netIt){
        for(int i=0; i<(*netIt)->getBlockSize(); ++i){

            BLOCK *block = BLOCKS[ (*netIt)->getBlock(i) ];
            if( block->getPlace() ) continue;

            if(root != NULL){
                block->setParent(block);
                block->setPlace(true);
                block->setCoordinate(0, 0);
                //update contour
                UpdateContour( temp_x, temp_y, block->getWidth(), block->getHeight(), max_y);

                temp_x += block->getWidth();
                rightMostBlock = block;
                leftMostBlock = block;
                root = block;

            }else if(temp_x + block->getWidth() > REGION_Side){
                //place block to next row
                temp_x = 0;
                leftMostBlock->setChild(R, block);
                block->setParent(leftMostBlock);
                block->setPlace(true);
                block->setCoordinate(0, temp_y);
                //update contour
                UpdateContour( temp_x, temp_y, block->getWidth(), block->getHeight(), max_y);
                
                temp_x = block->getWidth();
                temp_y = max_y;
                rightMostBlock = block;
                leftMostBlock = block;

            }else{
                //place block on the current row
                rightMostBlock->setChild(L, block);
                block->setParent(rightMostBlock);
                block->setPlace(true);
                block->setCoordinate(temp_x, temp_y);
                //update contour
                UpdateContour( temp_x, temp_y, block->getWidth(), block->getHeight(), max_y);

                temp_x += block->getWidth();
                rightMostBlock = block;
            }
        }
    }
}