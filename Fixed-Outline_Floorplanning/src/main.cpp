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

int BLOCK_NUM;
int TOTAL_AREA = 0;
int REGION_Side;
int *Y_CONTOUR;
int MAX_WIDTH, MAX_HEIGHT;
double WHITE_Ratio;
BLOCK *ROOT = NULL, *accept_root = NULL;

unsigned SEED = (unsigned)time(NULL);

double GLOBAL_WIDTH, GLOBAL_HEIGHT;
double GLOBAL_WIRE_LENGTH = 100000000000;
double LOCAL_WIRE_LENGTH = 100000000000;
BLOCK *GLOBAL_ROOT, *LOCAL_ROOT;
std::vector<BLOCK *> GLOBAL_BLOCKS, LOCAL_BLOCKS, accept_Blocks;

std::vector<BLOCK *> BLOCKS;
std::vector<NET *> NETS;
std::vector<TERMINAL *> TERMINALS;

template <class T>
bool compareID(T* const&one, T* const&two){
    return one->getID() < two->getID();
}

void Floorplan();
void ClearContour();
int UpdateContour(BLOCK *&);
void InitialBTree();
void Swapping(BLOCK *&, BLOCK *&);
void Rotate(BLOCK *);
bool Perturb();
void Packing();
int CalcWireLength();

void storeResult(const bool, std::vector<BLOCK *>&, const int type);


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
    file >> c >> BLOCK_NUM; //read a ':' and a number

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
    for(int i=0; i<BLOCK_NUM; ++i){

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
    std::cout << "Seed = " << SEED <<std::endl;
    std::cout << "REGION_Side = " << REGION_Side << std::endl << std::endl;
    Floorplan();


    delete[] Y_CONTOUR;
    //output file
    file.open(argv[5], std::fstream::out);

    file.close();
    return 0;
}

void travel(BLOCK *A){
    if( A != NULL ) A->print();
    if( A->getChild(L) != NULL ) travel(A->getChild(L));
    if( A->getChild(R) != NULL ) travel(A->getChild(R));
}

void Floorplan(){
    
    //Initialize contour
    Y_CONTOUR = new int[ 3 * REGION_Side ];
    ClearContour();
    
    //Initialize B*-tree with input blocks
    InitialBTree();

    // int a = 0;
    while(true){
        if( !Perturb() ) continue;
        if( MAX_HEIGHT <= REGION_Side && MAX_WIDTH <= REGION_Side ) break;
        // ++a;
        // if(a>1000) break;
    }

    // //calculate wire length
    // cur_wireLength = CalcWireLength();
    // cur_cost = cur_wireLength;

    
    std::cout << "------------------------[Initial]------------------------\n";
    std::cout << "Max height = " << MAX_HEIGHT << "\tMax width = " << MAX_WIDTH << std::endl;
    std::cout << "Wire length = " << CalcWireLength() << std::endl << std::endl;



    /*************************************/
    /* Adaptive fast Simulated Annealing */
    /*************************************/

    int n = 0, phase = 0;       //iteration and phase number
    // double feasibleNum = 0;
    // double avg_diff_cost = 0;   //average uphill cost of first 500 times perturb
    double prob_accept;         //probability for accepting uphill solution
    double T=2000000, T1;         //temperature and temperature of iteration 1
    double c = 100;             //parameter

    int penalty;
    // bool feasible[500]{false};
    double alpha = 0.5;

    int last_wireLength, cur_wireLength = CalcWireLength(), diff_wireLength;
    double last_cost, cur_cost = alpha * cur_wireLength, diff_cost;
    

    
    for(int i=0; i<BLOCK_NUM; ++i){
        LOCAL_BLOCKS.push_back( new BLOCK(i) );
        GLOBAL_BLOCKS.push_back( new BLOCK(i) );
        accept_Blocks.push_back( new BLOCK(i) );
    }
    storeResult(false, accept_Blocks, 0);    //store current solution from BLOCKS to accept_Blocks
    storeResult(false, GLOBAL_BLOCKS, 1);    //store current solution from BLOCKS to GLOBAL_BLOCKS
    storeResult(false, LOCAL_BLOCKS, 2);    //store current solution from BLOCKS to LOCAL_BLOCKS
                
    GLOBAL_WIRE_LENGTH = cur_wireLength;
    GLOBAL_HEIGHT = MAX_HEIGHT;
    GLOBAL_WIDTH = MAX_WIDTH;
    LOCAL_WIRE_LENGTH = cur_wireLength;

    srand(SEED);
    
    
    while(true){
        if( n > 499 ) break;

        last_wireLength = cur_wireLength;
        last_cost = cur_cost;

        /* Perturb the B*-tree */
        if( !Perturb() ) continue;

        /* Pack macro blocks */
        ClearContour();
        Packing();

        
        /* Evaluate the B*-tree cost */
        //estimate wire length
        cur_wireLength = CalcWireLength();
        diff_wireLength = cur_wireLength - last_wireLength;
        //calculate cost and difference of last cost and current cost
        penalty = 0;
        if( MAX_HEIGHT > REGION_Side ) penalty += 1000000;
        if( MAX_WIDTH > REGION_Side ) penalty += 1000000;
        cur_cost = alpha * cur_wireLength + (1-alpha) * penalty ;
        diff_cost = cur_cost - last_cost;

        // std::cout << "----------------------[ Iteration " << n << " ]---------------------\n";
        // std::cout << "Max height = " << MAX_HEIGHT << "\tMax width = " << MAX_WIDTH << std::endl;
        // std::cout << "Wire length = " << cur_wireLength << std::endl;
        

        /* Decide if we should accept the new B*-tree */
        if( MAX_HEIGHT <= REGION_Side && MAX_WIDTH <= REGION_Side ){

            storeResult(false, accept_Blocks, 0);    //store current solution from BLOCKS to accept_Blocks

            if(GLOBAL_WIRE_LENGTH > cur_wireLength){

                storeResult(false, GLOBAL_BLOCKS, 1);    //store current solution from BLOCKS to GLOBAL_BLOCKS
                GLOBAL_WIRE_LENGTH = cur_wireLength;
                GLOBAL_HEIGHT = MAX_HEIGHT;
                GLOBAL_WIDTH = MAX_WIDTH;
            }
            if(LOCAL_WIRE_LENGTH > cur_wireLength){
                storeResult(false, LOCAL_BLOCKS, 2);    //store current solution from BLOCKS to LOCAL_BLOCKS
                LOCAL_WIRE_LENGTH = cur_wireLength;
            }

        }else{
            prob_accept = std::min(1.0, exp( (-1)*diff_cost/T) );
            if(rand()%101 < prob_accept * 100){
                storeResult(false, accept_Blocks, 0);    //store current solution from BLOCKS to accept_Blocks
            }else{
                storeResult(true, accept_Blocks, 0);    //store current solution from accept_Blocks to BLOCKS
            }
            
        }
              

        /* Modify the weights in the cost function */
        double temp = 1 - GLOBAL_WIRE_LENGTH/cur_wireLength;
        if( temp < 0.9 && temp > 0.75 ) alpha = std::min( 1.0, alpha * 1.2 );
        else if( temp > 0.5 ) alpha = std::min( 1.0, alpha * 1.5 );
        else alpha = std::min( 1.0, alpha * 1.75 );

        // /* Update T */
        T *= 0.9;
        prob_accept = std::min(1.0, exp( (-1)*diff_cost/T) );
        if( T <= 0.000001 ) break;
        


        
        ++n;

        
        // std::cout<<std::endl;
    }
    std::cout << "Best wirelength = " << GLOBAL_WIRE_LENGTH << std::endl;
    std::cout << "height = " << GLOBAL_HEIGHT << "\twidth = " << GLOBAL_WIDTH << std::endl;
    travel(GLOBAL_ROOT);
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
void InitialBTree(){

    int temp_x = 0, temp_y = 0;
    BLOCK *leftMostBlock = NULL, *rightMostBlock = NULL;
    ROOT = NULL;

    //for each block
    for(auto it:BLOCKS){

        if(ROOT == NULL){
            //update contour and set coordinate of the current block
            UpdateContour( temp_x, temp_y, it->getWidth(), it->getHeight() );
            it->setCoordinate(0, 0);
            it->setParent( NULL );

            temp_x += it->getWidth();
            rightMostBlock = it;
            leftMostBlock = it;
            ROOT = it;

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

    //exchange parents
    if( A == ROOT || B == ROOT ){
        BLOCK *block;
        if( A == ROOT ) block = B;
        else block = A;
        
        BLOCK *parent = block->getParent();
        ROOT->setParent( parent );
        block->setParent(NULL);
        if( parent->getChild(L) == block ) parent->setChild(L, ROOT);
        else parent->setChild(R, ROOT);

        ROOT = block;
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
    BLOCK *child_A = A->getChild(L);
    BLOCK *child_B = B->getChild(L);
    A->setChild( L, child_B );
    B->setChild( L, child_A );
    if(child_A != NULL) child_A->setParent(B);
    if(child_B != NULL) child_B->setParent(A);
    
    //exchange right child
    child_A = A->getChild(R);
    child_B = B->getChild(R);
    A->setChild( R, child_B );
    B->setChild( R, child_A );
    if(child_A != NULL) child_A->setParent(B);
    if(child_B != NULL) child_B->setParent(A);
}

/* Perturb the b* tree */
bool Perturb(){
    bool perturbType = rand() & 1;

    //randomly select 2 blocks to swap
    if(perturbType){
        int a = rand() % BLOCK_NUM;
        int b = rand() % BLOCK_NUM;
        if( BLOCKS[a] == BLOCKS[b] ) return false;
        if( BLOCKS[a]->getParent() == BLOCKS[b] || BLOCKS[b]->getParent() == BLOCKS[a] ) return false;
        // printf("---------------[Swap Blocks ID: %d and %d]---------------\n", BLOCKS[a]->getID(), BLOCKS[b]->getID());
        Swapping(BLOCKS[a], BLOCKS[b] );
    }else{
        int a = rand() % BLOCK_NUM;
        // printf("------------------[Rotate Block ID: %d]------------------\n", BLOCKS[a]->getID() );
        Rotate( BLOCKS[a] );
    }
    return true;
}


/* Travel all of blocks on b* tree (pre-order) and packing the blocks */
void Packing(){

    int x = 0, y = 0;
    std::vector<BLOCK *> blockStack;
    blockStack.push_back(ROOT);

    while( !blockStack.empty() ){

        BLOCK *temp = blockStack.back();
        blockStack.pop_back();

        if(temp != ROOT){
            BLOCK *parent = temp->getParent();
            if(parent->getChild(R) == temp) x = parent->getX();
            else x = parent->getX() + parent->getWidth();
        }
        UpdateContour(x, y, temp->getWidth(), temp->getHeight() );
        temp->setCoordinate(x, y);
        //update MAX_WIDTH
        MAX_WIDTH = std::max( MAX_WIDTH, x+temp->getWidth() );

        // temp->print();

        //finish to put blocks on the current row
        if( temp->getChild(R) != NULL ) 
            blockStack.push_back( temp->getChild(R) );
        if( temp->getChild(L) != NULL )
            blockStack.push_back( temp->getChild(L) );   
    }
    
    // std::cout<<std::endl<<std::endl;
}

/* Using HPWL to estimate wire length of all the nets */
int CalcWireLength(){
    int length = 0;
    for(const auto &net:NETS){
        int min_x = 100000000, min_y = 100000000;
        int max_x = 0, max_y = 0; 
        for(int i = 0; i < net->getTerminalSize(); ++i){
            int index = net->getTerminal(i);
            TERMINAL temp(index);
            auto it = lower_bound( TERMINALS.begin(), TERMINALS.end(), &temp, compareID<TERMINAL> );
            int x = (*it)->getX();
            int y = (*it)->getY();

            min_x = std::min(min_x, x);
            min_y = std::min(min_y, y);
            max_x = std::max(max_x, x);
            max_y = std::max(max_y, y);
        }
        for(int i = 0; i < net->getBlockSize(); ++i){
            int index = net->getBlock(i);
            BLOCK temp(index);
            auto it = lower_bound( BLOCKS.begin(), BLOCKS.end(), &temp, compareID<BLOCK> );
            int x = (*it)->getX() + 0.5 * (*it)->getWidth();
            int y = (*it)->getY() + 0.5 * (*it)->getHeight();

            min_x = std::min(min_x, x);
            min_y = std::min(min_y, y);
            max_x = std::max(max_x, x);
            max_y = std::max(max_y, y);
        }
        // int parameter = max_x - min_x + max_y - min_y;
        // std::cout << "Net " << net->getID() << "\t" << parameter << std::endl;
        length += (max_x - min_x + max_y - min_y);
    }
    return length;
}

void storeResult(const bool toBLOCK, std::vector<BLOCK *>& A, const int type){

    if( !toBLOCK ){

        for(int i=0; i<BLOCK_NUM; ++i){
            A[i]->copy( *BLOCKS[i] );

            if(BLOCKS[i]->getChild(L) != NULL){
                int id = BLOCKS[i]->getChild(L)->getID();
                BLOCK temp(id);
                auto it = lower_bound( A.begin(), A.end(), &temp, compareID<BLOCK> );
                A[i]->setChild(L, *it);
            }else A[i]->setChild(L, NULL);

            if(BLOCKS[i]->getChild(R) != NULL){
                int id = BLOCKS[i]->getChild(R)->getID();
                BLOCK temp(id);
                auto it = lower_bound( A.begin(), A.end(), &temp, compareID<BLOCK> );
                A[i]->setChild(R, *it);
            }else A[i]->setChild(R, NULL);

            if(BLOCKS[i]->getParent() != NULL){
                int id = BLOCKS[i]->getParent()->getID();
                BLOCK temp(id);
                auto it = lower_bound( A.begin(), A.end(), &temp, compareID<BLOCK> );
                A[i]->setParent(*it);
            }else{
                A[i]->setParent(NULL);
                if(type == 0) accept_root = A[i];
                else if(type == 1) GLOBAL_ROOT = A[i];
                else if(type == 2) LOCAL_ROOT = A[i];
            }
        }
        
        // travel( accept_root );
    }else{

        for(int i=0; i<BLOCK_NUM; ++i){
            BLOCKS[i]->copy( *A[i] );
            if(A[i]->getChild(L) != NULL){
                int id = A[i]->getChild(L)->getID();
                BLOCK temp(id);
                auto it = lower_bound( BLOCKS.begin(), BLOCKS.end(), &temp, compareID<BLOCK> );
                BLOCKS[i]->setChild(L, *it);
            }else BLOCKS[i]->setChild(L, NULL);

            if(A[i]->getChild(R) != NULL){
                int id = A[i]->getChild(R)->getID();
                BLOCK temp(id);
                auto it = lower_bound( BLOCKS.begin(), BLOCKS.end(), &temp, compareID<BLOCK> );
                BLOCKS[i]->setChild(R, *it);
            }else BLOCKS[i]->setChild(R, NULL);

            if(A[i]->getParent() != NULL){
                int id = A[i]->getParent()->getID();
                BLOCK temp(id);
                auto it = lower_bound( BLOCKS.begin(), BLOCKS.end(), &temp, compareID<BLOCK> );
                BLOCKS[i]->setParent(*it);
            }else{
                BLOCKS[i]->setParent(NULL);
                ROOT = BLOCKS[i];
            }
        }
        // travel(ROOT);
    }
}

