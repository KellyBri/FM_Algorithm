#include <iostream>
#include <fstream>
#include <cstring>

#include "GLOBALROUTE.h"
#include "NET.h"

int main(int argc, char const **argv){

	std::string netFilename = argv[1];
	std::string outputFilename = argv[2];

	int gridxNum, gridyNum;
	int verticalCap, horizontalCap;
	int netNum;
	std::string text;

	//read file
	std::cout<<"Start to read file!\n";
	std::ifstream netFile(netFilename, std::ios::in);
	netFile >> text >> gridyNum >> gridxNum;
	netFile >> text >> text >> verticalCap;
	netFile >> text >> text >> horizontalCap;
	netFile >> text >> text >> netNum;
	// std::cout<<"grid x = "<<gridxNum<<"\ty = "<<gridyNum<<std::endl;
	// std::cout<<"vertical = "<<verticalCap<<"\thorizontal = "<<horizontalCap<<std::endl;
	// std::cout<<"num net = "<<netNum<<std::endl;
	GLOBALROUTE globalRoute(gridxNum, gridyNum, verticalCap, horizontalCap);

	for(int i=0; i<netNum; ++i){
		
		int netID, pinNum, x, y;
		std::string netName;
		netFile >> netName >> netID >> pinNum;
		NET net(netID, netName);
		// std::cout<<netName<<"\t"<<netID<<"\t"<<pinNum<<std::endl;
		for(int j=0; j<pinNum; ++j){
			netFile >> x >> y;
			net.addPin(x,y);
			// std::cout<<x<<"\t"<<y<<std::endl;
		}
		globalRoute.addNet(net);
	}
	netFile.close();
	std::cout<<"Success!" << std::endl << std::endl << std::endl;

	//routing by implementation of Lee algorithm with A*-search
	std::cout<<"Start to route!\n";
	globalRoute.route();
	std::cout<<"Success!" << std::endl << std::endl << std::endl;


	//write file
	std::cout<<"Start to write file!\n";
	std::ofstream outputFile(outputFilename, std::ios::out);
	for(int i=0; i<netNum; ++i){
		NET net = globalRoute.getNet(i); 
		outputFile << net.getName() << " " << net.getID() << std::endl;
		for(int j=0; j<net.getPathNum()-1; ++j){
			outputFile << "(" << net.getPathX(j)   << "," << net.getPathY(j)   <<",1)-";
			outputFile << "(" << net.getPathX(j+1) << "," << net.getPathY(j+1) <<",1)" << std::endl;
		}
		outputFile << "!\n";
	}
	outputFile.close();
	std::cout<<"Success!\n";
	return 0;
}
