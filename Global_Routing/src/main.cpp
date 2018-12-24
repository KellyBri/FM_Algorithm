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

	
	std::ifstream netFile(netFilename, std::ios::in);
	netFile >> text >> gridxNum >> gridyNum;
	netFile >> text >> text >> verticalCap;
	netFile >> text >> text >> horizontalCap;
	netFile >> text >> text >> netNum;
	// std::cout<<"grid x = "<<gridxNum<<"\ty = "<<gridyNum<<std::endl;
	// std::cout<<"vertical = "<<verticalCap<<"\thorizontal = "<<horizontalCap<<std::endl;
	// std::cout<<"num net = "<<netNum<<std::endl;
	GLOBALROUTE globalRoute(netNum);
	globalRoute.setGrid(gridxNum, gridyNum);
	globalRoute.setCapacity(verticalCap, horizontalCap);
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
	}
	netFile.close();



  
	return 0;
}
