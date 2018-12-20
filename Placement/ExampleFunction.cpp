#include "ExampleFunction.h"
#include <cmath>
#include <iostream>



ExampleFunction::ExampleFunction(Placement &placement):placement(placement)
{
    this->gamma = 1000;
    this->dim = placement.numModules();
    this->ex = new double[ this->dim ];
    this->ey = new double[ this->dim ];
    this->negex = new double[ this->dim ];
    this->negey = new double[ this->dim ];
}

//x:coordinate of each module; f:wirelength of a net; g:gradient of each module
void ExampleFunction::evaluateFG(const vector<double> &x, double &f, vector<double> &g){

    std::fill(g.begin(), g.end(), 0);

    for(unsigned i=0; i<this->dim; ++i){
        ex[i] = exp( x[ 2*i ]     / this->gamma );
        ey[i] = exp( x[ 2*i + 1 ] / this->gamma );
        negex[i] = exp( (-1) * x[ 2*i ]     / this->gamma );
        negey[i] = exp( (-1) * x[ 2*i + 1 ] / this->gamma );
    }
    for(unsigned j=0; j<this->placement.numNets(); ++j){
        double sumx, sumNegx, sumy, sumNegy;
        sumx = sumNegx = sumy = sumNegy = 0;
        //calculate summation of e^x, e^y, e^-x, e^-y 
        for(unsigned i=0; i<this->placement.net(j).numPins(); ++i){
            unsigned moduleID = this->placement.net(j).pin(i).moduleId();
            sumx += ex[moduleID];
            sumy += ey[moduleID];
            sumNegx += negex[moduleID];
            sumNegy += negey[moduleID];
        }
        //calculate gradient
        for(unsigned i=0; i<this->placement.net(j).numPins(); ++i){
            unsigned moduleID = this->placement.net(j).pin(i).moduleId();
            unsigned index = moduleID * 2;
            g[ index ]     += ex[moduleID] / (gamma * sumx);
            g[ index + 1 ] += ey[moduleID] / (gamma * sumy);
            g[ index ]     -= negex[moduleID] / (gamma * sumNegx);
            g[ index + 1 ] -= negey[moduleID] / (gamma * sumNegy);
        }
        f += this->gamma * ( log(sumx) + log(sumy) + log(sumNegx) + log(sumNegy) );
    }
    // std::cout<<"[FG] Approximating wirelength = " << f << std::endl;
}

void ExampleFunction::evaluateF(const vector<double> &x, double &f)
{
    for(unsigned i=0; i<this->dim; ++i){
        ex[i] = exp( x[ 2*i ]     / this->gamma );
        ey[i] = exp( x[ 2*i + 1 ] / this->gamma );
        negex[i] = exp( (-1) * x[ 2*i ]     / this->gamma );
        negey[i] = exp( (-1) * x[ 2*i + 1 ] / this->gamma );
    }
    for(unsigned j=0; j<this->placement.numNets(); ++j){
        double sumx, sumNegx, sumy, sumNegy;
        sumx = sumNegx = sumy = sumNegy = 0;
        //calculate summation of e^x, e^y, e^-x, e^-y 
        for(unsigned i=0; i<this->placement.net(j).numPins(); ++i){
            unsigned moduleID = this->placement.net(j).pin(i).moduleId();
            sumx += ex[moduleID];
            sumy += ey[moduleID];
            sumNegx += negex[moduleID];
            sumNegy += negey[moduleID];
        }
        f += this->gamma * ( log(sumx) + log(sumy) + log(sumNegx) + log(sumNegy) );
    }
    // std::cout<<"[F] Approximating wirelength = " << f << std::endl;
}
unsigned ExampleFunction::dimension()
{
    return this->dim * 2; // num_blocks*2 
    // each two dimension represent the X and Y dimensions of each block
}
