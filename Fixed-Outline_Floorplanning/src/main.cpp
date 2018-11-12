#include <iostream>
#include <fstream>  //for file read/write
#include <cstdlib>  //for atof
#include <algorithm>//for sort, lower_bound
#include <vector>
#include <cmath>    //for sqrt

#include "NET.h"
#include "BLOCK.h"
#include "TERMINAL.h"

#define R 0
#define L 1
#define ROTATE 1
#define UNROTATE 0

#ifndef PL
#define PL 0    //PL stands for TERMINAL
#endif

#ifndef SB
#define SB 1    //SB stands for hardblock
#endif

int TOTAL_AREA = 0;
double WHITE_Ratio;
int REGION_Side;
int *Y_CONTOUR;
int MAX_HEIGHT;

std::vector<BLOCK *> BLOCKS;
std::vector<NET *> NETS;
std::vector<TERMINAL *> TERMINALS;

template <class T>
bool compareID(T* const&one, T* const&two){
    return one->getID() < two->getID();
}

void Floorplan();
void ClearContour();
int UpdateContour(BLOCK *);
BLOCK *InitialBTree();
void Packing(BLOCK *);


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

        int num;
        file >> c >> c >> num; //read 's' and 'b', "number"
        BLOCK *temp = new BLOCK(num);

        file >> text >> num;   //read "hardrectilinear" and vertex "number"
        
        int max_x=0, max_y=0, min_x=100000000, min_y=100000000, x, y;
        for(int j=0; j<num; ++j){
            file >> c >> x >> c >> y >> c;    //read '(', x, ',', y, ')'

            if(max_x < x) max_x = x;
            else if(min_x > x) min_x = x;

            if(max_y < y) max_y = y;
            else if(min_y > y) min_y = y;
        }
        temp->setWidth(max_x - min_x);
        temp->setHeight(max_y - min_y);
        temp->calcArea();
        TOTAL_AREA += temp->getArea();
        BLOCKS.push_back(temp);
    }
    std::sort( BLOCKS.begin(), BLOCKS.end(), compareID<BLOCK> );
    // for(auto it = BLOCKS.begin(); it != BLOCKS.end(); ++it)
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
    std::sort( TERMINALS.begin(), TERMINALS.end(), compareID<TERMINAL> );
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
                TERMINAL *tempTerminal = new TERMINAL(num);
                auto it = lower_bound(TERMINALS.begin(), TERMINALS.end(), tempTerminal, compareID<TERMINAL>);
                temp->addNode(PL, std::distance(TERMINALS.begin(), it) );
                delete tempTerminal;
            }else{
                file >> c >> num;
                BLOCK *tempBlock = new BLOCK(num);
                auto it = lower_bound(BLOCKS.begin(), BLOCKS.end(), tempBlock, compareID<BLOCK>);
                temp->addNode(SB, std::distance(BLOCKS.begin(), it) );
                delete tempBlock;
            }
        }
        NETS.push_back(temp);
    }
    // for(auto it = NETS.begin(); it != NETS.end(); ++it)
    //     (*it)->print();
    file.close();


    WHITE_Ratio = atof(argv[4]);
    REGION_Side = sqrt( TOTAL_AREA * (1+WHITE_Ratio) );
    std::cout << "REGION_Side = " << REGION_Side << std::endl;
    Floorplan();


    delete[] Y_CONTOUR;
    //output file
    file.open(argv[5], std::fstream::out);

    file.close();
    return 0;
}

void Floorplan(){
    //Initialize contour
    Y_CONTOUR = new int[REGION_Side];
    ClearContour();
    
    //Initialize B*-tree with input blocks
    BLOCK *root = InitialBTree();
    
    //Adaptive fast Simulated Annealing
    int T = 0;  //initial temperature
    while(true){
        //perturb the B*-tree
        //pack macro blocks
        ClearContour();
        Packing(root);
        //evaluate the B*-tree cost
        //decide if we should accept the new B*-tree
        //modify the weights in the cost function
        //update T
        break;
    }
}

/* Clear all of elements in Y_CONTOUR and MAX_HEIGHT */
void ClearContour(){
    for(int i=0; i<REGION_Side; ++i)
        Y_CONTOUR[i] = 0;
    MAX_HEIGHT = 0;
}

/* Update contour due to adding new block */
void UpdateContour(const int &x, int &y, const int width, const int height){
    y = 0;
    //find max_y in the region (x, x+width)
    for(int i=x; i < x+width; ++i)
        if( y < Y_CONTOUR[i] ) y = Y_CONTOUR[i];
    //update contour
    for(int i=x; i < x+width; ++i) Y_CONTOUR[i] = y + height ;
    //update max_y of floorplan
    if( MAX_HEIGHT < y + height) MAX_HEIGHT = y + height;
}

/* Initial B*-tree with input blocks */
BLOCK *InitialBTree(){

    int temp_x = 0, temp_y = 0;
    BLOCK *leftMostBlock = NULL, *rightMostBlock = NULL, *root = NULL;

    //for each block
    for(auto it = BLOCKS.begin(); it != BLOCKS.end(); ++it){

        if(root == NULL){
            //update contour and set coordinate of the current block
            UpdateContour( temp_x, temp_y, (*it)->getWidth(), (*it)->getHeight() );
            (*it)->setCoordinate(0, 0);
            (*it)->setParent( *it );

            temp_x += (*it)->getWidth();
            rightMostBlock = (*it);
            leftMostBlock = (*it);
            root = (*it);

        }else if(temp_x + (*it)->getWidth() > REGION_Side){
            //place the block to next row
            temp_x = 0;
            
            //update contour and set coordinate of the current block
            UpdateContour( temp_x, temp_y, (*it)->getWidth(), (*it)->getHeight() );
            (*it)->setCoordinate(0, temp_y);
            (*it)->setParent(leftMostBlock);
            leftMostBlock->setChild(R, *it);

            temp_x = (*it)->getWidth();
            rightMostBlock = (*it);
            leftMostBlock = (*it);

        }else{
            //place the block next to the last block on the same row
            //update contour and set coordinate of the current block
            UpdateContour( temp_x, temp_y, (*it)->getWidth(), (*it)->getHeight() );
            (*it)->setCoordinate(temp_x, temp_y);
            (*it)->setParent(rightMostBlock);
            rightMostBlock->setChild(L, *it);

            temp_x += (*it)->getWidth();
            rightMostBlock = (*it);
        }
        // (*it)->print();
    }
    return root;
}

/* Travel all of blocks on b* tree (pre-order) and packing the blocks */
void Packing(BLOCK *root){

    int temp_x = 0, temp_y = 0;
    std::vector<BLOCK *> blockStack;
    blockStack.push_back(root);

    while( !blockStack.empty() ){

        BLOCK *temp = blockStack.back();
        blockStack.pop_back();

        //put the block on the current row
        UpdateContour(temp_x, temp_y, temp->getWidth(), temp->getHeight() );
        temp->setCoordinate(temp_x, temp_y);
        temp_x += temp->getWidth();

        // temp->print();

        //finish to put blocks on the current row
        if( temp->getChild(L) == NULL ) temp_x = 0;

        if( temp->getChild(R) != NULL ) 
            blockStack.push_back( temp->getChild(R) );
        if( temp->getChild(L) != NULL )
            blockStack.push_back( temp->getChild(L) );   
    }
}