#ifndef NET_H
#define NET_H
#include <string>
#include <vector>

class NET{
public:

	NET(int, std::string);
	void addPin(int, int);


private:
	int id;
	std::string name;

	int pinNum;
	std::vector<int> x, y;


};

#endif