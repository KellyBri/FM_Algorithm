#include "ExampleFunction.h"
#include <cmath>
#include <iostream>



ExampleFunction::ExampleFunction(Placement &placement):placement(placement)
{
    //************tune parameter************//
    this->gamma = (this->placement.boundryRight() - this->placement.boundryLeft() )/ 600;
    this->lambda = 0; 
    this->binNum = 10;  
    //*************tune parameter************//
    
    this->dim = placement.numModules();
    this->ex = new double[ this->dim ];
    this->ey = new double[ this->dim ];
    this->negex = new double[ this->dim ];
    this->negey = new double[ this->dim ];
    this->density_gx = new double[ this->dim ];
    this->density_gy = new double[ this->dim ];

    
    double boundryWidth  = this->placement.boundryRight() - this->placement.boundryLeft();
    double boundryHeight = this->placement.boundryTop()   - this->placement.boundryBottom();
    this->binWidth  = boundryWidth  / this->binNum;
    this->binHeight = boundryHeight / this->binNum;
    this->binDensity = new double[ this->binNum * this->binNum ];
    this->maxDensity = 0;
    for(unsigned i=0; i<this->dim; ++i){
        this->maxDensity += this->placement.module(i).area();
    }
    this->maxDensity /= boundryWidth * boundryHeight;


    //Pb: the preplaced block area in bin b.
    double *Pb = new double[this->binNum * this->binNum];
    memset(Pb, 0, sizeof(double) * this->binNum * this->binNum);
    for(unsigned id=0; id<this->dim; ++id){
        if( !this->placement.module(id).isFixed() ) continue;

        int i = this->placement.module(id).x() / this->binWidth; 
        int j = this->placement.module(id).y() / this->binHeight;
        if(i>=this->binNum) i = this->binNum-1;
        if(j>=this->binNum) j = this->binNum-1;
        Pb[ i * this->binNum + j] = this->placement.module(id).area();
    }

    this->Mb = new double[this->binNum * this->binNum];
    //Mb = targetDensity(Wb*Hb − Pb)
    double binArea = this->binHeight * this->binWidth;
    for(int i=0; i<this->binNum; ++i){
        for(int j=0; j<this->binNum; ++j){
            this->Mb[ i * this->binNum + j] = this->maxDensity * ( binArea - Pb[ i * this->binNum + j] );
        }
    }
    
    
}

//x:coordinate of each module; f:wirelength of a net; g:gradient of each module
void ExampleFunction::evaluateFG(const vector<double> &x, double &f, vector<double> &g){

    std::fill(g.begin(), g.end(), 0);

    //LSE part
    for(unsigned i=0; i<this->dim; ++i){
        this->ex[i] = exp( x[ 2*i ]     / this->gamma );
        this->ey[i] = exp( x[ 2*i + 1 ] / this->gamma );
        this->negex[i] = exp( (-1) * x[ 2*i ]     / this->gamma );
        this->negey[i] = exp( (-1) * x[ 2*i + 1 ] / this->gamma );
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
            if( !this->placement.module(moduleID).isFixed() ){
                unsigned index = moduleID * 2;
                g[ index ]     += ex[moduleID] / (gamma * sumx);
                g[ index + 1 ] += ey[moduleID] / (gamma * sumy);
                g[ index ]     -= negex[moduleID] / (gamma * sumNegx);
                g[ index + 1 ] -= negey[moduleID] / (gamma * sumNegy);
            }
        }
        f += this->gamma * ( log(sumx) + log(sumy) + log(sumNegx) + log(sumNegy) );
    }

    if(this->lambda == 0) return;

    //Density part
    memset(density_gx, 0, sizeof(double) * this->dim);
    memset(density_gy, 0, sizeof(double) * this->dim);
    memset(binDensity, 0, sizeof(double) * this->binNum*this->binNum);
    
    for(int i=0; i<this->binNum; ++i){
        for(int j=0; j<this->binNum; ++j){
            //for each module
            for(unsigned id=0; id<this->dim; ++id){

                double moduleWidth  = this->placement.module(id).width();
                double moduleHeight = this->placement.module(id).height();
                double c = moduleWidth * moduleHeight / (this->binWidth * this->binHeight); //****************tuning
                
                //deal Px with x-coordinate
                double ax = 4/( (moduleWidth + 2 * this->binWidth) * (moduleWidth + 4 * this->binWidth) );
                double bx = 2/( this->binWidth * (moduleWidth + 4 * this->binWidth) );
                double dx = this->placement.module(id).centerX() - (j + 0.5) * this->binWidth;
                double abs_dx = abs(dx);
                double Px = 0;
                double firstConstraint_x  = moduleWidth * 0.5 + this->binWidth;
                double secondConstraint_x =  firstConstraint_x  + this->binWidth;

                if( abs_dx >= 0 && abs_dx <= firstConstraint_x )
                    Px = 1 - ax * dx * dx;
                else if( abs_dx >= firstConstraint_x && abs_dx <= secondConstraint_x )
                    Px = bx * pow(abs_dx - 0.5 * moduleWidth - 2 * this->binWidth, 2 );
                
                
                //deal Py with y-coordinate
                double ay = 4/( (moduleHeight + 2 * this->binHeight) * (moduleHeight + 4 * this->binHeight) );
                double by = 2/( this->binHeight * (moduleHeight + 4 * this->binHeight) );
                double dy = this->placement.module(id).centerY() - (i + 0.5) * this->binHeight;
                double abs_dy = abs(dy);
                double Py = 0;
                double firstConstraint_y  = moduleHeight * 0.5 + this->binHeight;
                double secondConstraint_y =  firstConstraint_y  + this->binHeight;

                if( abs_dy >= 0 && abs_dy <= firstConstraint_y )
                    Py = 1 - ay * dy * dy;
                else if( abs_dy >= firstConstraint_y && abs_dy <= secondConstraint_y )
                    Py = by * pow(abs_dy - 0.5 * moduleHeight - 2 * this->binHeight, 2 );


                if( !this->placement.module(id).isFixed() ){
                    //deal g with x-coordinate
                    int sign = 1;
                    if(dx < 0) sign = -1;
                    if( abs_dx >= 0 && abs_dx <= firstConstraint_x )
                        this->density_gx[ id ] += (-2) * sign * c * ax * abs_dx * Py;
                    else if( abs_dx >= firstConstraint_x && abs_dx <= secondConstraint_x )
                        this->density_gx[ id ] += 2 * c * bx * sign * ( abs_dx - 2 * this->binWidth - 0.5 * moduleWidth ) * Py;

                    //deal g with y-coordinate
                    sign = 1;
                    if(dy < 0) sign = -1;
                    if( abs_dy >= 0 && abs_dy <= firstConstraint_y )
                        this->density_gy[ id ] += (-2) * c * sign * ay * abs_dy * Px;
                    else if( abs_dy >= firstConstraint_y && abs_dy <= secondConstraint_y )
                        this->density_gy[ id ] += 2 * c * by * sign * ( abs_dy - 2 * this->binHeight - 0.5 * moduleWidth ) * Px;
                }
                this->binDensity[ i * this->binNum + j ] += c * Px * Py;
            }
            //objective function
            f += this->lambda * pow( this->binDensity[ i * this->binNum + j ] - this->maxDensity, 2);
            // f += this->lambda * pow( this->binDensity[ i * this->binNum + j ] - this->Mb[ i * this->binNum + j ], 2);
            //calculate gradient
            for(unsigned id=0; id<this->dim; ++id){
                int D = this->lambda * 2 * (this->binDensity[ i * this->binNum + j ] - this->maxDensity);
                // int D = this->lambda * 2 * (this->binDensity[ i * this->binNum + j ] - this->Mb[ i * this->binNum + j ]);
                g[ 2*id     ] += D * this->density_gx[ id ];
                g[ 2*id + 1 ] += D * this->density_gy[ id ];
            }
        }
    }
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
    if(this->lambda == 0) return;

    //Density part
    memset(density_gx, 0, sizeof(double) * this->dim);
    memset(density_gy, 0, sizeof(double) * this->dim);
    memset(binDensity, 0, sizeof(double) * this->binNum*this->binNum);
    
    for(int i=0; i<this->binNum; ++i){
        for(int j=0; j<this->binNum; ++j){
            //for each module
            for(unsigned id=0; id<this->dim; ++id){

                double moduleWidth  = this->placement.module(id).width();
                double moduleHeight = this->placement.module(id).height();
                double c = moduleWidth * moduleHeight / (this->binWidth * this->binHeight); //****************tuning
                
                //deal Px with x-coordinate
                double ax = 4/( (moduleWidth + 2 * this->binWidth) * (moduleWidth + 4 * this->binWidth) );
                double bx = 2/( this->binWidth * (moduleWidth + 4 * this->binWidth) );
                double dx = this->placement.module(id).centerX() - (j + 0.5) * this->binWidth;
                double abs_dx = abs(dx);
                double Px = 0;
                double firstConstraint_x  = moduleWidth * 0.5 + this->binWidth;
                double secondConstraint_x =  firstConstraint_x  + this->binWidth;

                if( abs_dx >= 0 && abs_dx <= firstConstraint_x )
                    Px = 1 - ax * dx * dx;
                else if( abs_dx >= firstConstraint_x && abs_dx <= secondConstraint_x )
                    Px = bx * pow(abs_dx - 0.5 * moduleWidth - 2 * this->binWidth, 2 );
                
                
                //deal Py with y-coordinate
                double ay = 4/( (moduleHeight + 2 * this->binHeight) * (moduleHeight + 4 * this->binHeight) );
                double by = 2/( this->binHeight * (moduleHeight + 4 * this->binHeight) );
                double dy = this->placement.module(id).centerY() - (i + 0.5) * this->binHeight;
                double abs_dy = abs(dy);
                double Py = 0;
                double firstConstraint_y  = moduleHeight * 0.5 + this->binHeight;
                double secondConstraint_y =  firstConstraint_y  + this->binHeight;

                if( abs_dy >= 0 && abs_dy <= firstConstraint_y )
                    Py = 1 - ay * dy * dy;
                else if( abs_dy >= firstConstraint_y && abs_dy <= secondConstraint_y )
                    Py = by * pow(abs_dy - 0.5 * moduleHeight - 2 * this->binHeight, 2 );

                this->binDensity[ i * this->binNum + j ] += c * Px * Py;
            }
            //objective function
            f += this->lambda * pow( this->binDensity[ i * this->binNum + j ] - this->maxDensity, 2);
            // f += this->lambda * pow( this->binDensity[ i * this->binNum + j ] - this->Mb[ i * this->binNum + j ], 2);
        }
    }
}
unsigned ExampleFunction::dimension()
{
    return this->dim * 2; // num_blocks*2 
    // each two dimension represent the X and Y dimensions of each block
}

void ExampleFunction::addlambda(){
    this->lambda += 1000;
}
