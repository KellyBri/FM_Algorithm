#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"

GlobalPlacer::GlobalPlacer(Placement &placement)
	:_placement(placement)
{

}

// Randomly place modules implemented by TA
void GlobalPlacer::randomPlace(vector<double> &coordinate){
	double w = _placement.boundryRight() - _placement.boundryLeft();
	double h = _placement.boundryTop() - _placement.boundryBottom();
	for (size_t i = 0; i < _placement.numModules(); ++i ){
		double wx = _placement.module(i).width(), 
			   hx = _placement.module(i).height();
		coordinate[ 2 * i ] = (int) rand() % (int)(w - wx) + _placement.boundryLeft();
		coordinate[2 * i+1] = (int) rand() % (int)(h - hx) + _placement.boundryBottom();
		_placement.module(i).setPosition( coordinate[2*i], coordinate[2*i+1] );
	}
}


void GlobalPlacer::place()
{
	ExampleFunction ef(_placement); // require to define the object function and gradient function
    NumericalOptimizer no(ef);

    vector<double> coordinate(_placement.numModules() * 2); // solution vector, size: num_blocks*2 
                                                            // each 2 variables represent the X and Y dimensions of a block
	// Initial placement
	// An example of random placement by TA. If you want to use it, please uncomment the folllwing 2 lines.
	srand(time(NULL));
	randomPlace(coordinate);
    
	
	/* @@@ TODO 
	 * v1. Understand above example and modify ExampleFunction.cpp to implement the analytical placement
	 * v2. You can choose LSE or WA as the wirelength model, the former is easier to calculate the gradient
     *  3. For the bin density model, you could refer to the lecture notes
     *  4. You should first calculate the form of wirelength model and bin density model and the forms of their gradients ON YOUR OWN 
	 *  5. Replace the value of f in evaluateF() by the form like "f = alpha*WL() + beta*BinDensity()"
	 *  6. Replace the form of g[] in evaluateG() by the form like "g = grad(WL()) + grad(BinDensity())"
	 *  7. Set the initial vector x in main(), set step size, set #iteration, and call the solver like above example
	 * */

    no.setX(coordinate);    // set solution
    no.setNumIteration(1000); // user-specified parameter
    no.setStepSizeBound(30); // user-specified parameter
    for(int j=0; j<5; ++j){
        no.solve(); // Conjugate Gradient solver
        for (size_t i = 0; i < _placement.numModules(); ++i ){
            coordinate[ 2 * i ] = no.x(2 * i);
            coordinate[2 * i+1] = no.x(2 * i + 1);
            _placement.module(i).setPosition( coordinate[2*i], coordinate[2*i+1] );
        }
        no.setX(coordinate);
    }

    for(unsigned i=0; i<_placement.numModules(); ++i){
        _placement.module(i).setPosition( coordinate[2*i], coordinate[2*i+1] );
    }
    // cout << "Current solution:" << endl;
    // for (unsigned i = 0; i < no.dimension(); i++) {
    //     cout << "x[" << i << "] = " << no.x(i) << endl;
    // }
    // cout << "Objective: " << no.objective() << endl;
	
}


void GlobalPlacer::plotPlacementResult( const string outfilename, bool isPrompt )
{
    ofstream outfile( outfilename.c_str() , ios::out );
    outfile << " " << endl;
    outfile << "set title \"wirelength = " << _placement.computeHpwl() << "\"" << endl;
    outfile << "set size ratio 1" << endl;
    outfile << "set nokey" << endl << endl;
    outfile << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl << endl;
    outfile << "# bounding box" << endl;
    plotBoxPLT( outfile, _placement.boundryLeft(), _placement.boundryBottom(), _placement.boundryRight(), _placement.boundryTop() );
    outfile << "EOF" << endl;
    outfile << "# modules" << endl << "0.00, 0.00" << endl << endl;
    for( size_t i = 0; i < _placement.numModules(); ++i ){
        Module &module = _placement.module(i);
        plotBoxPLT( outfile, module.x(), module.y(), module.x() + module.width(), module.y() + module.height() );
    }
    outfile << "EOF" << endl;
    outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

    if( isPrompt ){
        char cmd[ 200 ];
        sprintf( cmd, "gnuplot %s", outfilename.c_str() );
        if( !system( cmd ) ) { cout << "Fail to execute: \"" << cmd << "\"." << endl; }
    }
}

void GlobalPlacer::plotBoxPLT( ofstream& stream, double x1, double y1, double x2, double y2 )
{
    stream << x1 << ", " << y1 << endl << x2 << ", " << y1 << endl
           << x2 << ", " << y2 << endl << x1 << ", " << y2 << endl
           << x1 << ", " << y1 << endl << endl;
}
