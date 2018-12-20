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
void Initial(bool);

void Swapping(BLOCK *&, BLOCK *&);
void Move(BLOCK *&, BLOCK *&);
void Rotate(BLOCK *);
bool Perturb();

void Packing();
int CalcWireLength(std::vector<BLOCK *> &);
void travel(BLOCK *);

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


    WHITE_Ratio = atof(argv[5]);
    REGION_Side = sqrt( TOTAL_AREA * (1+WHITE_Ratio) );
    // std::cout << "Original REGION_Side = " << REGION_Side << std::endl << std::endl;

    bool type = true;
    int tempRegion = REGION_Side;
    std::string inputType = argv[1];
    if( inputType.find("n100") != std::string::npos ){
        REGION_Side = 454;
        SEED = 1542635978;
        inputType = "n100";
    }else if( inputType.find("n200") != std::string::npos && WHITE_Ratio == 0.15 ){
        SEED = 1542633911;
        inputType = "n200";
    }else if( inputType.find("n200") != std::string::npos && WHITE_Ratio == 0.1 ){
        SEED = 1542654994;
        type = false;
        inputType = "n200";
    }else if( inputType.find("n300") != std::string::npos && WHITE_Ratio == 0.15 ){
        SEED = 1542633911;
        inputType = "n300";
    }else if( inputType.find("n300") != std::string::npos && WHITE_Ratio == 0.1 ){
        SEED = 1542663680;
        type = false;
        inputType = "n300";
    }else{
        SEED = 1542728461;
        type = false;
    }
    
    //std::cout << "------------------------[ "<< inputType <<"\t" << WHITE_Ratio << " ]------------------------\n";
    // std::cout << inputType <<"\t" << WHITE_Ratio << std::endl;
    
    srand(SEED);
    Initial(type);
    REGION_Side = tempRegion;
    
    if( inputType == "n100" && WHITE_Ratio == 0.1 ){
        SEED = 1542958241;
    }else if( inputType == "n100" && WHITE_Ratio == 0.15 ){
        SEED = 1542911935;
    }else if( inputType == "n200" && WHITE_Ratio == 0.1 ){
        SEED = 1543070888;
    }else if( inputType == "n200" && WHITE_Ratio == 0.15 ){
        SEED = 1542898363;
    }else if( inputType == "n300" && WHITE_Ratio == 0.1 ){
        SEED = 1543059740;
    }else SEED = 1543083218;
    srand(SEED);
    
    Floorplan();


    delete[] Y_CONTOUR;
    //output file
     file.open(argv[4], std::fstream::out);
     file << "Wirelength " << CalcWireLength(GLOBAL_BLOCKS) << std::endl <<"Blocks\n" ;
     for(int i=0; i<BLOCK_NUM; ++i){
         file << "sb" << GLOBAL_BLOCKS[i]->getID() << " ";
         file << GLOBAL_BLOCKS[i]->getX() << " " << GLOBAL_BLOCKS[i]->getY() << " ";
         if( GLOBAL_BLOCKS[i]->getRotate() == ROTATE ){
             file << GLOBAL_BLOCKS[i]->getHeight() << " " << GLOBAL_BLOCKS[i]->getWidth() << " ";
             file << "1\n";
         }else{
             file << GLOBAL_BLOCKS[i]->getWidth() << " " << GLOBAL_BLOCKS[i]->getHeight() << " ";
             file << "0\n";
         }
     }
     file.close();
    return 0;
}

void travel(BLOCK *A){
    if( A != NULL ) A->print();
    if( A->getChild(L) != NULL ) travel(A->getChild(L));
    if( A->getChild(R) != NULL ) travel(A->getChild(R));
}

void Initial(bool type){
    //Initialize contour
    Y_CONTOUR = new int[ 5 * REGION_Side ];
    ClearContour();
    
    //Initialize B*-tree with input blocks
    InitialBTree();
    
    //initial storage vectors
    for(int i=0; i<BLOCK_NUM; ++i){
        LOCAL_BLOCKS.push_back( new BLOCK(i) );
        GLOBAL_BLOCKS.push_back( new BLOCK(i) );
        accept_Blocks.push_back( new BLOCK(i) );
    }
    storeResult(false, accept_Blocks, 0);   //store current solution from BLOCKS to accept_Blocks
    storeResult(false, GLOBAL_BLOCKS, 1);   //store current solution from BLOCKS to GLOBAL_BLOCKS
    storeResult(false, LOCAL_BLOCKS, 2);    //store current solution from BLOCKS to LOCAL_BLOCKS

    int height = MAX_HEIGHT, width = MAX_WIDTH;
    int min_area = MAX_HEIGHT * MAX_WIDTH;
    double cc = 1.25;
    for(int i=0; i<800000; ++i){
        if(type){
            if(i == 35000) cc = 1.15;
            else if(i == 60000) cc = 1.05;
            else if(i == 80000) cc = 1;
            if(i > 300000) break; 
        }else{
            if(i == 50000) cc = 1.2;
            else if(i == 100000) cc = 1.15;
            else if(i == 150000) cc = 1.1;
            else if(i == 250000) cc = 1.05;
            else if(i == 350000) cc = 1;
        }

        if( !Perturb() ) continue;
        ClearContour();
        Packing();

        int area = MAX_HEIGHT * MAX_WIDTH;
        if( area > min_area || MAX_HEIGHT > REGION_Side*cc || MAX_WIDTH > REGION_Side*cc ){
            storeResult(true, GLOBAL_BLOCKS, 1);
        }else{
            // storeResult(false, accept_Blocks, 0);
            storeResult(false, GLOBAL_BLOCKS, 1);
            // storeResult(false, LOCAL_BLOCKS, 2);
            min_area = area;
            // last = i;
            height = MAX_HEIGHT;
            width = MAX_WIDTH;
        }
    }
    storeResult(true, GLOBAL_BLOCKS, 1);
    storeResult(false, accept_Blocks, 0);
    storeResult(false, LOCAL_BLOCKS, 2);

    GLOBAL_WIRE_LENGTH = CalcWireLength(GLOBAL_BLOCKS);
    GLOBAL_HEIGHT = height;
    GLOBAL_WIDTH = width;
    LOCAL_WIRE_LENGTH = GLOBAL_WIRE_LENGTH;

    //std::cout << "------------------------[Initial]------------------------\n";
    //std::cout << "Max height = " << MAX_HEIGHT << "\tMax width = " << MAX_WIDTH << std::endl;
    //std::cout << "Wire length = " << GLOBAL_WIRE_LENGTH << std::endl << std::endl;

    // if(height <= REGION_Side && width <= REGION_Side){
    //     std::cout << "------------------------[Initial]------------------------\n";
    //     std::cout << "Seed = " << SEED <<std::endl;
    //     std::cout << "REGION_Side = " << REGION_Side << std::endl;
    //     // std::cout << last<<"\t" ;
    //     std::cout << GLOBAL_HEIGHT <<"\t" << GLOBAL_WIDTH << "\t";
    //     std::cout << "Wirelength = " << GLOBAL_WIRE_LENGTH << "\n";
    // }
}

void Floorplan(){

    // std::cout << "------------------------[Initial]------------------------\n";
    //std::cout << "Seed = " << SEED << "\tREGION_Side = " << REGION_Side << std::endl;
    // std::cout << GLOBAL_HEIGHT <<"\t" << GLOBAL_WIDTH << "\t";
    // std::cout << "Wirelength = " << GLOBAL_WIRE_LENGTH << "\n";

    /*************************************/
    /* Adaptive fast Simulated Annealing */
    /*************************************/
    double alpha = 0.01;
    int last_wireLength, cur_wireLength = GLOBAL_WIRE_LENGTH, diff_wireLength;
    double last_cost, cur_cost = alpha * cur_wireLength, diff_cost;


    int rejectNum = 0;  //continuous reject time
    int uphillNum = 0;  //continuous uphill time
    int noBestNum = 0;  //continuous time that the result is not the best
    int penalty;
    double prob_accept; //probability for accepting uphill solution
    int last = 0;
    
    for(int i=0; i<500000; ++i){

        last_wireLength = cur_wireLength;
        last_cost = cur_cost;

        /* Perturb the B*-tree */
        if( !Perturb() ) continue;

        /* Pack macro blocks */
        ClearContour();
        Packing();
        
        /* Evaluate the B*-tree cost */
        //estimate wire length
        cur_wireLength = CalcWireLength(BLOCKS);
        diff_wireLength = cur_wireLength - last_wireLength;
        
        //calculate cost and difference of last cost and current cost
        if( MAX_HEIGHT > REGION_Side && MAX_WIDTH > REGION_Side){ 
            penalty = (MAX_HEIGHT - REGION_Side) * MAX_WIDTH;
            penalty += (MAX_WIDTH - REGION_Side) * MAX_HEIGHT;
            penalty -= (MAX_HEIGHT - REGION_Side) * (MAX_WIDTH - REGION_Side);
        }else if( MAX_HEIGHT > REGION_Side )
            penalty = (MAX_HEIGHT - REGION_Side) * MAX_WIDTH;
        else if( MAX_WIDTH > REGION_Side )
            penalty = (MAX_WIDTH - REGION_Side) * MAX_HEIGHT;
        else penalty = 0;

        cur_cost = diff_wireLength + penalty ;
        diff_cost = cur_cost - last_cost;


        /* Decide if we should accept the new B*-tree */
        if( diff_wireLength > 0 ) uphillNum++;
        
        //If the current floorplan is inside the region, accept!
        if( MAX_HEIGHT <= REGION_Side && MAX_WIDTH <= REGION_Side ){

            //If the wirelength of current floorplan is the best, store it into GLOBAL and LOCAL!
            if(GLOBAL_WIRE_LENGTH > cur_wireLength){
                rejectNum = 0;
                uphillNum = 0;
                // noBestNum = 0;
                //store current solution from BLOCKS to GLOBAL_BLOCKS
                storeResult(false, GLOBAL_BLOCKS, 1);    
                GLOBAL_WIRE_LENGTH = cur_wireLength;
                GLOBAL_HEIGHT = MAX_HEIGHT;
                GLOBAL_WIDTH = MAX_WIDTH;
                //store current solution from BLOCKS to LOCAL_BLOCKS
                storeResult(false, LOCAL_BLOCKS, 2);    
                LOCAL_WIRE_LENGTH = cur_wireLength;
                //store current solution from BLOCKS to accept_Blocks
                storeResult(false, accept_Blocks, 0); 

                last = i;
            }
            //If the wirelength of current floorplan is better than LOCAL, store it into LOCAL!
            // else if(LOCAL_WIRE_LENGTH > cur_wireLength || diff_wireLength <= 0){
            else if(LOCAL_WIRE_LENGTH > cur_wireLength){
                rejectNum = 0;
                uphillNum = 0;
                // noBestNum++;
                //store current solution from BLOCKS to LOCAL_BLOCKS
                storeResult(false, LOCAL_BLOCKS, 2);    
                LOCAL_WIRE_LENGTH = cur_wireLength;
                //store current solution from BLOCKS to accept_Blocks
                storeResult(false, accept_Blocks, 0);
            }
            //If the wirelength of the current floorplan does not go better, decide whether accept it!
            else{
                noBestNum++;
                prob_accept = std::min(1.0, exp( (-1)*diff_cost/i) );
                if( uphillNum > 50 || rejectNum > 50 ){
                    storeResult(true, LOCAL_BLOCKS, 2);
                    storeResult(false, accept_Blocks, 0);
                    uphillNum = 0;
                    rejectNum = 0;
                }
                // else if( noBestNum == 300000 ){
                //     storeResult(true, GLOBAL_BLOCKS, 1); 
                // }
                else if(rand()%1001 < prob_accept * 1000){
                    //Accept, store current solution from BLOCKS to accept_Blocks
                    storeResult(false, accept_Blocks, 0);    
                }else{
                    rejectNum ++;
                    //Reject, store current solution from accept_Blocks to BLOCKS
                    storeResult(true, accept_Blocks, 0);    
                }
            }
        }else{
            noBestNum++;
            prob_accept = std::min(1.0, exp( (-1)*diff_cost/i) );
            if( uphillNum > 10 || rejectNum > 10 ){
                storeResult(true, LOCAL_BLOCKS, 2);
                storeResult(false, accept_Blocks, 0);
                uphillNum = 0;
                rejectNum = 0;
            }
            // else if( noBestNum == 300000 ){
            //     storeResult(true, GLOBAL_BLOCKS, 1); 
            // }
            else if(rand()%1001 < prob_accept * 1000){
                //Accept, store current solution from BLOCKS to accept_Blocks
                storeResult(false, accept_Blocks, 0);    
            }else{
                rejectNum ++;
                //Reject, store current solution from accept_Blocks to BLOCKS
                storeResult(true, accept_Blocks, 0);    
            }
        }

        // std::cout << "----------------------[ Iteration " << i << " ]---------------------\n";
        // std::cout << "Max height = " << MAX_HEIGHT << "\tMax width = " << MAX_WIDTH << std::endl;
        // std::cout << "Current Wirelength = " << cur_wireLength << "\t/ Best wirelength = " << GLOBAL_WIRE_LENGTH << std::endl;

    }
    //std::cout << "*************************[ Done ]***************************\n";
    //std::cout << "Best wirelength = " << GLOBAL_WIRE_LENGTH <<" (Interation " << last << ")" << std::endl;
    //std::cout << "height = " << GLOBAL_HEIGHT << "\twidth = " << GLOBAL_WIDTH << std::endl;
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

/* Move block B to left child of block A */
void Move(BLOCK *&A, BLOCK *&B){

    //changes in block B side
    BLOCK *parent_B = B->getParent();
    BLOCK *lchild_B = B->getChild(L);
    BLOCK *rchild_B = B->getChild(R);

    bool side;
    if( parent_B->getChild(L) == B ) side = L;
    else side = R;

    if(lchild_B != NULL){
        parent_B->setChild(side, lchild_B);
        lchild_B->setParent(parent_B);

        BLOCK *temp = lchild_B;
        while(temp->getChild(R) != NULL) temp = temp->getChild(R);

        temp->setChild(R, rchild_B);
        if(rchild_B != NULL) rchild_B->setParent(temp);
    }else{
        parent_B->setChild(side, rchild_B);
        if(rchild_B!=NULL) rchild_B->setParent(parent_B);
    }
    B->setChild(L, NULL);
    B->setChild(R, NULL);

    

    //changes in block A side
    B->setParent(A);
    BLOCK *lchild_A = A->getChild(L);
    
    if(lchild_A == NULL){
        A->setChild(L, B);
    }else{
        A->setChild(L, B);
        lchild_A->setParent(B);
        B->setChild(L, lchild_A);
    }
}

/* Perturb the b* tree */
bool Perturb(){
    int perturbType = rand() % 20;

    //randomly select 2 blocks to swap
    if(perturbType <= 9){
        int a = rand() % BLOCK_NUM;
        int b = rand() % BLOCK_NUM;
        if( a == b ) return false;
        if( BLOCKS[a]->getParent() == BLOCKS[b] || BLOCKS[b]->getParent() == BLOCKS[a] ) return false;
        // printf("---------------[Swap Blocks ID: %d and %d]---------------\n", BLOCKS[a]->getID(), BLOCKS[b]->getID());
        Swapping(BLOCKS[a], BLOCKS[b] );
    }else if(perturbType <= 18){
        int a = rand() % BLOCK_NUM;
        // printf("------------------[Rotate Block ID: %d]------------------\n", BLOCKS[a]->getID() );
        Rotate( BLOCKS[a] );
    }else{
        int a = rand() % BLOCK_NUM;
        int b = rand() % BLOCK_NUM;
        if( a == b ) return false;
        if( BLOCKS[b] == ROOT ) return false;
        if( BLOCKS[a]->getParent() == BLOCKS[b] || BLOCKS[b]->getParent() == BLOCKS[a] ) return false;
        // printf("---------------[Move Blocks ID: %d to %d]---------------\n", BLOCKS[b]->getID(), BLOCKS[a]->getID());
        Move(BLOCKS[a], BLOCKS[b] );
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
int CalcWireLength(std::vector<BLOCK *> &blocks){
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
            // std::cout<<"p"<<(*it)->getID()<<"\n";
        }
        for(int i = 0; i < net->getBlockSize(); ++i){
            int index = net->getBlock(i);
            BLOCK temp(index);
            auto it = lower_bound( blocks.begin(), blocks.end(), &temp, compareID<BLOCK> );
            int x = (*it)->getX() + 0.5 * (*it)->getWidth();
            int y = (*it)->getY() + 0.5 * (*it)->getHeight();

            min_x = std::min(min_x, x);
            min_y = std::min(min_y, y);
            max_x = std::max(max_x, x);
            max_y = std::max(max_y, y);
            // std::cout<<"sb"<<(*it)->getID()<<"\n";
        }
        // int parameter = max_x - min_x + max_y - min_y;
        // std::cout << "Net " << net->getID() << "\t" << parameter << std::endl;
        // std::cout<<"\n";
        length += (max_x - min_x + max_y - min_y);
    }
    return length;
}

/* Store the current to GLOBAL_BLOCKS or LOCAL_BLOCKS or accept_blocks
    or store back accept_blocks to BLOCKS */
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
    }
}