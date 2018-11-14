#include <iostream>
#include <fstream>  //for file read/write
#include <cstdlib>  //for atof, rand
#include <algorithm>//for sort, lower_bound, min, max
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
int REGION_Side;
int *Y_CONTOUR;
int MAX_WIDTH, MAX_HEIGHT;
double WHITE_Ratio;

double GLOBAL_MIN_COST = 100000000000;
double LOCAL_MIN_COST = 100000000000;
double GLOBAL_WIRE_LENGTH, LOCAL_WIRE_LENGTH;
std::vector<BLOCK> GLOBAL_BLOCKS, LOCAL_BLOCKS;

std::vector<BLOCK *> BLOCKS;
std::vector<NET *> NETS;
std::vector<TERMINAL *> TERMINALS;

template <class T>
bool compareID(T* const&one, T* const&two){
    return one->getID() < two->getID();
}

void Floorplan(int &);
void ClearContour();
int UpdateContour(BLOCK *&);
BLOCK *InitialBTree();
void Swapping(BLOCK *&, BLOCK *&);
void Rotate(BLOCK *);
void Packing(BLOCK *&);
double CalcWireLength();


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
                temp->addNode(PL, (*it)->getID() );
                delete tempTerminal;
            }else{
                file >> c >> num;
                BLOCK *tempBlock = new BLOCK(num);
                auto it = lower_bound(BLOCKS.begin(), BLOCKS.end(), tempBlock, compareID<BLOCK>);
                temp->addNode(SB, (*it)->getID() );
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
    Floorplan(blockNum);


    delete[] Y_CONTOUR;
    //output file
    file.open(argv[5], std::fstream::out);

    file.close();
    return 0;
}

void travel(BLOCK *root){
    if( root != NULL ) root->print();
    if( root->getChild(L) != NULL ) travel(root->getChild(L));
    if( root->getChild(R) != NULL ) travel(root->getChild(R));
}

void Floorplan(int &blockNum){
    double alpha, beta;
    double last_cost, cur_cost, diff_cost, uphill_cost = 0;
    double last_wireLength, cur_wireLength, diff_wireLength;
    unsigned seed = (unsigned)time(NULL);
    srand(seed);
    
    //Initialize contour
    Y_CONTOUR = new int[ 3 * REGION_Side ];
    ClearContour();
    
    //Initialize B*-tree with input blocks
    BLOCK *root = InitialBTree();
    //calculate wire length
    cur_wireLength = CalcWireLength();
    cur_cost = cur_wireLength;

    std::cout << "Seed = " << seed <<std::endl;
    std::cout << "------------------------[Initial]------------------------\n";
    std::cout << "Max height = " << MAX_HEIGHT << "\tMax width = " << MAX_WIDTH << std::endl;
    std::cout << "Wire length = " << cur_wireLength << "\tCost = " << cur_cost << std::endl << std::endl;

    
    //Adaptive fast Simulated Annealing
    double prob_accept = 0.9999;//probability for accepting uphill solution
    double T = 2;               //initial temperature
    int n = 0;                  //iteration


    while(true){
        
        last_wireLength = cur_wireLength;
        last_cost = cur_cost;

        /* Perturb the B*-tree */
        //randomly select 2 blocks to swap
        bool perturbType = rand() & 1;
        if(perturbType){
            int a = rand() % blockNum;
            int b = rand() % blockNum;
            printf("---------------[Swap Blocks ID: %d and %d]---------------\n", BLOCKS[a]->getID(), BLOCKS[b]->getID());
            Swapping( BLOCKS[a], BLOCKS[b] );
        }else{
            int a = rand() % blockNum;
            printf("------------------[Rotate Block ID: %d]------------------\n", BLOCKS[a]->getID() );
            Rotate( BLOCKS[a] );
        }
        // travel(root);

        /* Pack macro blocks */
        ClearContour();
        Packing(root);
        
        /* Evaluate the B*-tree cost */
        //estimate wire length
        cur_wireLength = CalcWireLength();
        diff_wireLength = cur_wireLength - last_wireLength;
        //calculate cost and difference of last cost and current cost
        cur_cost = cur_wireLength;
        diff_cost = cur_cost - last_cost;

        std::cout << "Max height = " << MAX_HEIGHT << "\tMax width = " << MAX_WIDTH << std::endl;
        std::cout << "Wire length = " << cur_wireLength << "\tCost = " << cur_cost << std::endl;

        /* Decide if we should accept the new B*-tree */
        prob_accept = std::min(1.0, exp( (-1)*diff_cost/T) );
        bool accept = rand()%101 < prob_accept*100;
        if( accept ){

        }
        std::cout << "Probability = " << prob_accept << "\tAccept = " << accept << std::endl;
        /* Modify the weights in the cost function */
        alpha = 0;
        beta = 0;

        /* Update T */
        if(diff_cost > 0) uphill_cost += cur_cost;
        T *= 0.85;

        std::cout<<std::endl;
        if(n > 1) break;
        ++n;
    }
}

/* Clear all of elements in Y_CONTOUR and MAX_HEIGHT */
void ClearContour(){
    for(int i=0; i<3*REGION_Side; ++i)
        Y_CONTOUR[i] = 0;
    //reset max width and height of floorplan
    MAX_HEIGHT = 0;
    MAX_WIDTH = 0;
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
    MAX_HEIGHT = std::max(MAX_HEIGHT, y+height);
}

/* Initial B*-tree with input blocks */
BLOCK *InitialBTree(){

    int temp_x = 0, temp_y = 0;
    BLOCK *leftMostBlock = NULL, *rightMostBlock = NULL, *root = NULL;

    //for each block
    for(auto it:BLOCKS){

        if(root == NULL){
            //update contour and set coordinate of the current block
            UpdateContour( temp_x, temp_y, it->getWidth(), it->getHeight() );
            it->setCoordinate(0, 0);
            it->setParent( it );

            temp_x += it->getWidth();
            rightMostBlock = it;
            leftMostBlock = it;
            root = it;

            MAX_WIDTH = std::max(MAX_WIDTH, temp_x);

        }else if(temp_x + it->getWidth() > REGION_Side){
            //place the block to next row
            temp_x = 0;
            
            //update contour and set coordinate of the current block
            UpdateContour( temp_x, temp_y, it->getWidth(), it->getHeight() );
            it->setCoordinate(0, temp_y);
            it->setParent(leftMostBlock);
            leftMostBlock->setChild(R, it);

            temp_x = it->getWidth();
            rightMostBlock = it;
            leftMostBlock = it;

        }else{
            //place the block next to the last block on the same row
            //update contour and set coordinate of the current block
            UpdateContour( temp_x, temp_y, it->getWidth(), it->getHeight() );
            it->setCoordinate(temp_x, temp_y);
            it->setParent(rightMostBlock);
            rightMostBlock->setChild(L, it);

            temp_x += it->getWidth();
            rightMostBlock = it;

            MAX_WIDTH = std::max(MAX_WIDTH, temp_x);
        }
        // it->print();
    }
    return root;
}

/* Rotate a block */
void Rotate(BLOCK *block){
    int width = block->getWidth();
    int height = block->getHeight();
    bool r = block->getRotate();

    block->setWidth( height );
    block->setHeight( width );
    block->setRotate( !r );
}

/* Swap two blocks in b* tree */
void Swapping(BLOCK *&A, BLOCK *&B){

    if( A->getParent() == B || B->getParent() == A ) return;

    //exchange parents
    if( A->getParent() == NULL || B->getParent() == NULL ){
        BLOCK *block, *root;
        if( A->getParent() == NULL ){
            root = A;
            block = B;
        }else{
            root = B;
            block = A;
        }

        BLOCK *parent = block->getParent();
        root->setParent( parent );
        block->setParent(NULL);
        if( parent->getChild(L) == block ) parent->setChild(L, root);
        else parent->setChild(R, root);

        root = block;
    }else{
        BLOCK *parent_A = A->getParent();
        BLOCK *parent_B = B->getParent();
        A->setParent( parent_B );
        B->setParent( parent_A );

        if( parent_A->getChild(L) == A ) parent_A->setChild(L, B);
        else parent_A->setChild(R, B);
        if( parent_B->getChild(L) == B ) parent_B->setChild(L, A);
        else parent_B->setChild(R, A);
    }


    //exchange left child
    BLOCK *temp = A->getChild(L);
    A->setChild( L, B->getChild(L) );
    B->setChild( L, temp );

    //exchange right child
    temp = A->getChild(R);
    A->setChild( R, B->getChild(R) );
    B->setChild( R, temp );
}


/* Travel all of blocks on b* tree (pre-order) and packing the blocks */
void Packing(BLOCK *&root){

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
        //update MAX_WIDTH
        MAX_WIDTH = std::max(MAX_WIDTH, temp_x);

        // temp->print();

        //finish to put blocks on the current row
        if( temp->getChild(L) == NULL ) temp_x = 0;

        if( temp->getChild(R) != NULL ) 
            blockStack.push_back( temp->getChild(R) );
        if( temp->getChild(L) != NULL )
            blockStack.push_back( temp->getChild(L) );   
    }
}

/* Using HPWL to estimate wire length of all the nets */
double CalcWireLength(){
    double length = 0;
    for(const auto &net:NETS){
        double min_x = 100000000, min_y = 100000000;
        double max_x = 0, max_y = 0; 
        for(int i = 0; i < net->getTerminalSize(); ++i){
            int index = net->getTerminal(i);
            TERMINAL temp(index);
            auto it = lower_bound( TERMINALS.begin(), TERMINALS.end(), &temp, compareID<TERMINAL> );
            double x = (*it)->getX();
            double y = (*it)->getY();

            min_x = std::min(min_x, x);
            min_y = std::min(min_y, y);
            max_x = std::max(max_x, x);
            max_y = std::max(max_y, y);
        }
        for(int i = 0; i < net->getBlockSize(); ++i){
            int index = net->getBlock(i);
            BLOCK temp(index);
            auto it = lower_bound( BLOCKS.begin(), BLOCKS.end(), &temp, compareID<BLOCK> );
            double x = (*it)->getX() + 0.5 * (*it)->getWidth();
            double y = (*it)->getY() + 0.5 * (*it)->getHeight();

            min_x = std::min(min_x, x);
            min_y = std::min(min_y, y);
            max_x = std::max(max_x, x);
            max_y = std::max(max_y, y);
        }
        
        double parameter = max_x - min_x + max_y - min_y;
        // std::cout << "Net " << net->getID() << "\t" << parameter << std::endl;
        length += parameter;
    }

    return length;
}

