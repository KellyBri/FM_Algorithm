#include <iostream>
#include <fstream>  //for file read/write
#include <cstdlib>  //for atof
#include <vector>

#include "NET.h"
#include "BLOCK.h"
#include "TERMINAL.h"

#ifndef PL
#define PL 0    //PL stands for TERMINAL
#endif

#ifndef SB
#define SB 1    //SB stands for hardblock
#endif

int main(int argc, char const **argv){
    
    if(argc != 5){
        std::cout<<"Error: The number of arguments is wrong!\n";
        std::cout<<"Three input files and white space ratio must be given!\n";
        return -1;
    }
    //read .hardblocks file
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

    std::cout<<blockNum<<"\t"<<terminalNum<<std::endl;

    std::vector<BLOCK *> blocks;

    //read blocks
    for(int i=0; i<blockNum; ++i){

        file >> c >> c; //read 's' and 'b'
        
        int num;
        file >> num;    //read sb "number"
        BLOCK *temp = new BLOCK(num);

        file >> text >> num;   //read "hardrectilinear" and vertex "number"
        temp->setVertexNum(num);

        int x, y;
        for(int j=0; j<num; ++j){
            file >> c >> x >> c >> y >> c;    //read '(', x, ',', y, ')'
            temp->addVertex(x, y);
        }
        temp->calcWidthHeight();
        blocks.push_back(temp);
    }
    // for(auto it = blocks.begin(); it != blocks.end(); ++it)
    //     (*it)->print();

    // //read terminals (p)
    // std::vector<TERMINAL *> terminals;
    // for(int i=0; i<terminalNum; ++i){
    //     int num;
    //     file >> c >> num >> text;
    //     terminals.push_back( new TERMINAL(num) );
    // }
    // for(auto it = terminals.begin(); it != terminals.end(); ++it)
    //     (*it)->print();
    file.close();




    //read .nets file
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
    file >> text >> c >> num;
    std::cout << netNum << std::endl;
    std::vector<NET *> nets;
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
        nets.push_back(temp);
    }
    // for(auto it = nets.begin(); it != nets.end(); ++it)
    //     (*it)->print();
    file.close();



    //read .pl file
    file.open(argv[3], std::ifstream::in);
    if( !file.is_open() ){
        std::cout<<"Error: Failed to open .pl input file!\n";
        return -1;
    }
    //read terminals (p)
    std::vector<TERMINAL *> terminals;
    for(int i=0; i<terminalNum; ++i){
        int id, x, y;
        file >> c >> id >> x >> y;
        terminals.push_back( new TERMINAL(id, x, y) );
    }
    // for(auto it = terminals.begin(); it != terminals.end(); ++it)
    //     (*it)->print();

    file.close();


    const double WHITE_Ratio = atof(argv[4]);

    return 0;
}
